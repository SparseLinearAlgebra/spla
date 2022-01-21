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

#include <algo/matrix/SplaMatrixEWiseAddCOO.hpp>
#include <compute/SplaCopyUtils.hpp>
#include <compute/SplaGather.hpp>
#include <compute/SplaMaskByKey.hpp>
#include <compute/SplaMergeByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

#include <boost/compute/algorithm.hpp>

bool spla::MatrixEWiseAddCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMatrixEWiseAdd *>(&params);

    return p &&
           p->mask.Is<MatrixCOO>() &&
           p->a.Is<MatrixCOO>() &&
           p->b.Is<MatrixCOO>();
}

void spla::MatrixEWiseAddCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    auto p = dynamic_cast<ParamsMatrixEWiseAdd *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto &type = p->type;
    auto byteSize = type->GetByteSize();
    auto typeHasValues = type->HasValues();

    auto fillValuesPermutationIndices = [&](RefPtr<MatrixCOO> &block, compute::vector<unsigned int> &perm) {
        if (block.IsNotNull() && typeHasValues) {
            auto nnz = block->GetNvals();
            perm.resize(nnz, queue);
            compute::copy_n(compute::make_counting_iterator(0), nnz, perm.begin(), queue);
        }
    };

    auto blockA = p->a.Cast<MatrixCOO>();
    const compute::vector<unsigned int> *rowsA = nullptr;
    const compute::vector<unsigned int> *colsA = nullptr;
    compute::vector<unsigned int> permA(ctx);

    fillValuesPermutationIndices(blockA, permA);

    auto blockB = p->b.Cast<MatrixCOO>();
    const compute::vector<unsigned int> *rowsB = nullptr;
    const compute::vector<unsigned int> *colsB = nullptr;
    compute::vector<unsigned int> permB(ctx);

    fillValuesPermutationIndices(blockB, permB);

    // If mask is present, apply it to a and b blocks
    compute::vector<unsigned int> tmpRowsA(ctx);
    compute::vector<unsigned int> tmpRowsB(ctx);
    compute::vector<unsigned int> tmpColsA(ctx);
    compute::vector<unsigned int> tmpColsB(ctx);

    auto maskBlock = p->mask.Cast<MatrixCOO>();
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);

    auto applyMask = [&](RefPtr<MatrixCOO> &block,
                         compute::vector<unsigned int> &tmpRows,
                         compute::vector<unsigned int> &tmpCols,
                         compute::vector<unsigned int> &perm,
                         const compute::vector<unsigned int> *&outRows,
                         const compute::vector<unsigned int> *&outCols) {
        // Nothing to do
        if (block.IsNull())
            return;

        // No mask or must apply inverse mask (but !null = all)
        if (!p->hasMask || (complementMask && maskBlock.IsNull())) {
            outRows = &block->GetRows();
            outCols = &block->GetCols();
            return;
        }

        // No inverse and no block - null result
        if (!complementMask && maskBlock.IsNull())
            return;

        if (typeHasValues) {
            compute::vector<unsigned int> tmpPerm(ctx);
            MaskByPairKeys(maskBlock->GetRows(), maskBlock->GetCols(),
                           block->GetRows(), block->GetCols(), perm,
                           tmpRows, tmpCols, tmpPerm,
                           complementMask,
                           queue);
            std::swap(perm, tmpPerm);
        } else {
            MaskPairKeys(maskBlock->GetRows(), maskBlock->GetCols(),
                         block->GetRows(), block->GetCols(),
                         tmpRows, tmpCols,
                         complementMask,
                         queue);
        }

        outRows = &tmpRows;
        outCols = &tmpCols;
    };

    applyMask(blockA, tmpRowsA, tmpColsA, permA, rowsA, colsA);
    applyMask(blockB, tmpRowsB, tmpColsB, permB, rowsB, colsB);

    // If some block is empty (or both, save result as is and finish without merge)
    auto aEmpty = !rowsA || rowsA->empty();
    auto bEmpty = !rowsB || rowsB->empty();

    if (aEmpty || bEmpty) {
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

            p->w = MatrixCOO::Make(block->GetNrows(), block->GetNcols(), nnz, std::move(newRows), std::move(newCols), std::move(newVals)).As<MatrixBlock>();
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
    if (typeHasValues)
        OffsetIndices(permB, blockA->GetNvals(), queue);

    // Merge a and b values
    auto mergeCount = rowsA->size() + rowsB->size();
    compute::vector<unsigned int> mergedRows(mergeCount, ctx);
    compute::vector<unsigned int> mergedCols(mergeCount, ctx);
    compute::vector<unsigned int> mergedPerm(ctx);

    if (typeHasValues) {
        mergedPerm.resize(mergeCount, queue);
        MergeByPairKeys(rowsA->begin(), rowsA->end(), colsA->begin(),
                        permA.begin(),
                        rowsB->begin(), rowsB->end(), colsB->begin(),
                        permB.begin(),
                        mergedRows.begin(), mergedCols.begin(), mergedPerm.begin(),
                        queue);
    } else
        MergePairKeys(rowsA->begin(), rowsA->end(), colsA->begin(),
                      rowsB->begin(), rowsB->end(), colsB->begin(),
                      mergedRows.begin(), mergedCols.begin(),
                      queue);

    // Copy values to single buffer
    compute::vector<unsigned char> mergedValues(ctx);
    if (typeHasValues) {
        mergedValues.resize(mergeCount * byteSize, queue);
        CopyMergedValues(mergedPerm, blockA->GetVals(), blockB->GetVals(), mergedValues, blockA->GetNvals(), byteSize, queue);
    }

    // Reduce duplicates
    // NOTE: max 2 duplicated entries for each index
    compute::vector<unsigned int> resultRows(ctx);
    compute::vector<unsigned int> resultCols(ctx);
    compute::vector<unsigned char> resultVals(ctx);
    std::size_t resultNvals;

    if (typeHasValues)
        resultNvals = ReduceDuplicates(mergedRows, mergedCols, mergedValues,
                                       resultRows, resultCols, resultVals,
                                       byteSize,
                                       p->op->GetSource(),
                                       queue);
    else
        resultNvals = ReduceDuplicates(mergedRows, mergedCols,
                                       resultRows, resultCols,
                                       queue);

    p->w = MatrixCOO::Make(blockA->GetNrows(), blockA->GetNcols(), resultNvals, std::move(resultRows), std::move(resultCols), std::move(resultVals)).As<MatrixBlock>();
}

spla::Algorithm::Type spla::MatrixEWiseAddCOO::GetType() const {
    return Type::MatrixEWiseAdd;
}

std::string spla::MatrixEWiseAddCOO::GetName() const {
    return "MatrixEWiseAddCOO";
}
