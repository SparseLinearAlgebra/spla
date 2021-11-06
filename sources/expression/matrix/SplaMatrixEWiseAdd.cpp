/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#include <boost/compute/algorithm.hpp>
#include <compute/SplaGather.hpp>
#include <compute/SplaMaskByKey.hpp>
#include <compute/SplaMergeByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/matrix/SplaMatrixEWiseAdd.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

bool spla::MatrixEWiseAdd::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::MatrixEWiseAdd::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto w = node->GetArg(0).Cast<Matrix>();
    auto mask = node->GetArg(1).Cast<Matrix>();
    auto op = node->GetArg(2).Cast<FunctionBinary>();
    auto a = node->GetArg(3).Cast<Matrix>();
    auto b = node->GetArg(4).Cast<Matrix>();
    auto desc = node->GetDescriptor();

    // NOTE: mask and op allowed to be null
    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(b.IsNotNull());
    assert(desc.IsNotNull());

    std::size_t requiredDeviceCount = w->GetStorage()->GetNblockRows() * w->GetStorage()->GetNblockCols();
    auto deviceIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < w->GetStorage()->GetNblockRows(); i++) {
        for (std::size_t j = 0; j < w->GetStorage()->GetNblockCols(); j++) {
            auto deviceId = deviceIds[i * w->GetStorage()->GetNblockCols() + j];
            builder.Emplace([=]() {
                using namespace boost;

                auto blockIndex = MatrixStorage::Index{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};
                auto device = library->GetDeviceManager().GetDevice(deviceId);
                compute::context ctx = library->GetContext();
                compute::command_queue queue(ctx, device);
                QueueFinisher finisher(queue);

                SPDLOG_LOGGER_TRACE(logger, "Process block=({},{})", i, j);

                auto type = w->GetType();
                auto byteSize = type->GetByteSize();
                auto typeHasValues = type->HasValues();
                assert(typeHasValues);

                auto fillValuesPermutationIndices = [&](RefPtr<MatrixCOO> &block, compute::vector<unsigned int> &perm) {
                    if (block.IsNotNull() /* todo: && typeHasValues */) {
                        auto nnz = block->GetNvals();
                        perm.resize(nnz, queue);
                        compute::copy_n(compute::make_counting_iterator(0), nnz, perm.begin(), queue);
                    }
                };

                auto blockA = a->GetStorage()->GetBlock(blockIndex).Cast<MatrixCOO>();
                const compute::vector<unsigned int> *rowsA = nullptr;
                const compute::vector<unsigned int> *colsA = nullptr;
                compute::vector<unsigned int> permA(ctx);

                fillValuesPermutationIndices(blockA, permA);

                auto blockB = b->GetStorage()->GetBlock(blockIndex).Cast<MatrixCOO>();
                const compute::vector<unsigned int> *rowsB = nullptr;
                const compute::vector<unsigned int> *colsB = nullptr;
                compute::vector<unsigned int> permB(ctx);

                fillValuesPermutationIndices(blockB, permB);

                // If mask is present, apply it to a and b blocks
                compute::vector<unsigned int> tmpRowsA(ctx);
                compute::vector<unsigned int> tmpRowsB(ctx);
                compute::vector<unsigned int> tmpColsA(ctx);
                compute::vector<unsigned int> tmpColsB(ctx);

                auto maskBlock = mask.IsNotNull() ? mask->GetStorage()->GetBlock(blockIndex).Cast<MatrixCOO>() : RefPtr<MatrixCOO>();
                auto applyMask = [&](RefPtr<MatrixCOO> &block,
                                     compute::vector<unsigned int> &tmpRows,
                                     compute::vector<unsigned int> &tmpCols,
                                     compute::vector<unsigned int> &perm,
                                     const compute::vector<unsigned int> *&outRows,
                                     const compute::vector<unsigned int> *&outCols) {
                    if (block.IsNull())
                        return;
                    if (mask.IsNull()) {
                        outRows = &block->GetRows();
                        outCols = &block->GetCols();
                        return;
                    }
                    if (maskBlock.IsNull())
                        return;

                    auto maxResultCount = std::min(maskBlock->GetNvals(), block->GetNvals());
                    tmpRows.resize(maxResultCount, queue);
                    tmpCols.resize(maxResultCount, queue);

                    auto count = MaskByPairKey(maskBlock->GetRows().begin(), maskBlock->GetRows().end(), maskBlock->GetCols().begin(),
                                               block->GetRows().begin(), block->GetRows().end(), block->GetCols().begin(),
                                               perm.begin(),
                                               tmpRows.begin(),
                                               tmpCols.begin(),
                                               perm.begin(),
                                               queue);

                    // NOTE: remember to shrink size of each buffer after masking to match actual count size
                    tmpRows.resize(count, queue);
                    tmpCols.resize(count, queue);
                    perm.resize(count, queue);
                    outRows = &tmpRows;
                    outCols = &tmpCols;
                };

                applyMask(blockA, tmpRowsA, tmpColsA, permA, rowsA, colsA);
                applyMask(blockB, tmpRowsB, tmpColsB, permB, rowsB, colsB);

                // If some block is empty (or both, save result as is and finish without merge)
                auto aEmpty = !rowsA || rowsA->empty();
                auto bEmpty = !rowsB || rowsB->empty();

                if (aEmpty || bEmpty) {
                    auto &storage = w->GetStorage();

                    // Remove old block in case if both a and b are null
                    storage->RemoveBlock(blockIndex);

                    auto setResult = [&](RefPtr<MatrixCOO> &block,
                                         const compute::vector<unsigned int> *rows,
                                         const compute::vector<unsigned int> *cols,
                                         compute::vector<unsigned int> &perm) {
                        auto nnz = rows->size();
                        assert(nnz == cols->size());
                        compute::vector<unsigned int> newRows(*rows, queue);
                        compute::vector<unsigned int> newCols(*cols, queue);
                        compute::vector<unsigned char> newVals(ctx);

                        // Copy masked values if presented
                        if (typeHasValues) {
                            auto &vals = block->GetVals();
                            newVals.resize(nnz * byteSize, queue);
                            Gather(perm.begin(), perm.end(), vals.begin(), newVals.begin(), byteSize, queue);
                        }

                        auto result = MatrixCOO::Make(block->GetNrows(), block->GetNcols(), nnz, std::move(newRows), std::move(newCols), std::move(newVals));
                        storage->SetBlock(blockIndex, result.As<MatrixBlock>());
                    };

                    // Copy result of masking a block
                    if (!aEmpty)
                        setResult(blockA, rowsA, colsA, permA);

                    // Copy result of masking b block
                    if (!bEmpty)
                        setResult(blockB, rowsB, colsB, permB);

                    return;
                }

                // NOTE: offset b perm indices to preserve uniqueness
                {
                    auto offset = static_cast<unsigned int>(blockA->GetNvals());
                    BOOST_COMPUTE_CLOSURE(void, offsetB, (unsigned int i), (permB, offset), {
                        permB[i] = permB[i] + offset;
                    });

                    compute::for_each_n(compute::counting_iterator<unsigned int>(0), permB.size(), offsetB, queue);
                }

                // Merge a and b values
                auto mergeCount = rowsA->size() + rowsB->size();
                compute::vector<unsigned int> mergedRows(mergeCount, ctx);
                compute::vector<unsigned int> mergedCols(mergeCount, ctx);
                compute::vector<unsigned int> mergedPerm(mergeCount, ctx);

                MergeByPairKey(rowsA->begin(), colsA->begin(), rowsA->end(),
                               permA.begin(),
                               rowsB->begin(), colsB->begin(), rowsB->end(),
                               permB.begin(),
                               mergedRows.begin(), mergedCols.begin(), mergedPerm.begin(),
                               queue);

                // Copy values to single buffer
                compute::vector<unsigned char> mergedValues(mergeCount * byteSize, ctx);
                {
                    auto &aVals = blockA->GetVals();
                    auto &bVals = blockB->GetVals();
                    auto offset = blockA->GetNvals();
                    BOOST_COMPUTE_CLOSURE(void, copyValues, (unsigned int i), (mergedPerm, mergedValues, aVals, bVals, offset, byteSize), {
                        const uint idx = mergedPerm[i];

                        if (idx < offset) {
                            const uint dst = i * byteSize;
                            const uint src = idx * byteSize;
                            for (uint k = 0; k < byteSize; k++)
                                mergedValues[dst + k] = aVals[src + k];
                        } else {
                            const uint dst = i * byteSize;
                            const uint src = (idx - offset) * byteSize;
                            for (uint k = 0; k < byteSize; k++)
                                mergedValues[dst + k] = bVals[src + k];
                        }
                    });
                    compute::for_each_n(compute::counting_iterator<unsigned int>(0), mergedPerm.size(), copyValues, queue);
                }

                // Reduce duplicates
                // NOTE: max 2 duplicated entries for each index
                compute::vector<unsigned int> resultRows(ctx);
                compute::vector<unsigned int> resultCols(ctx);
                compute::vector<unsigned char> resultVals(ctx);
                auto resultNvals = ReduceDuplicates(mergedRows, mergedCols, mergedValues,
                                                    resultRows, resultCols, resultVals,
                                                    byteSize,
                                                    op->GetSource(),
                                                    queue);

                SPDLOG_LOGGER_TRACE(logger, "Merge block (i={}; j={}) nnz={}", i, j, resultNvals);

                auto result = MatrixCOO::Make(blockA->GetNrows(), blockA->GetNcols(), resultNvals, std::move(resultRows), std::move(resultCols), std::move(resultVals));
                w->GetStorage()->SetBlock(blockIndex, result.As<MatrixBlock>());
            });
        }
    }
}

spla::ExpressionNode::Operation spla::MatrixEWiseAdd::GetOperationType() const {
    return ExpressionNode::Operation::MatrixEWiseAdd;
}
