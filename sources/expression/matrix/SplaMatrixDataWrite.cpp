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

#include <boost/compute.hpp>

#include <compute/SplaSortByRowColumn.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaMath.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/matrix/SplaMatrixDataWrite.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCSR.hpp>

#include <vector>

bool spla::MatrixDataWrite::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::MatrixDataWrite::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto matrix = node->GetArg(0).Cast<Matrix>();
    auto matrixData = node->GetArg(1).Cast<DataMatrix>();
    auto desc = node->GetDescriptor();

    assert(matrix.IsNotNull());
    assert(matrixData.IsNotNull());
    assert(desc.IsNotNull());

    auto nrows = matrix->GetNrows();
    auto ncols = matrix->GetNcols();
    auto blockSize = library->GetBlockSize();

    auto blocksCountInRow = math::GetBlocksCount(nrows, blockSize);
    auto blocksCountInCol = math::GetBlocksCount(ncols, blockSize);
    auto requiredDeviceCount = blocksCountInRow * blocksCountInCol;
    auto devicesIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < blocksCountInRow; i++) {
        for (std::size_t j = 0; j < blocksCountInCol; j++) {
            auto deviceId = devicesIds[i * blocksCountInCol + j];
            builder.Emplace([=]() {
                using namespace boost;

                compute::context ctx = library->GetContext();
                compute::device device = library->GetDeviceManager().GetDevice(deviceId);
                compute::command_queue queue(ctx, device);
                QueueFinisher finisher(queue);

                auto blockIndex = MatrixStorage::Index{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};
                auto blockNrows = math::GetBlockActualSize(i, nrows, blockSize);
                auto blockNcols = math::GetBlockActualSize(j, ncols, blockSize);

                auto firstRow = static_cast<unsigned int>(i * blockSize);
                auto firstCol = static_cast<unsigned int>(j * blockSize);
                auto lastRow = firstRow + static_cast<unsigned int>(blockNrows);
                auto lastCol = firstCol + static_cast<unsigned int>(blockNcols);

                auto rowsHost = matrixData->GetRows();
                auto colsHost = matrixData->GetCols();
                auto valsHost = reinterpret_cast<unsigned char *>(matrixData->GetVals());
                auto nvalsHost = matrixData->GetNvals();

                assert(rowsHost);
                assert(colsHost);

                // Count number of nnz values to store in this block
                std::size_t blockNvals = 0;
                {
                    for (std::size_t k = 0; k < nvalsHost; k++) {
                        auto rowIdx = rowsHost[k];
                        auto colIdx = colsHost[k];

                        if (firstRow <= rowIdx && rowIdx < lastRow &&
                            firstCol <= colIdx && colIdx < lastCol)
                            blockNvals += 1;
                    }
                }

                SPDLOG_LOGGER_TRACE(logger, "Process matrix block ({},{}) size=({},{}) ranges=([{}..{}),[{}..{})) nvals={}",
                                    i, j, blockNrows, blockNcols, firstRow, lastRow, firstCol, lastCol, blockNvals);

                auto storage = matrix->GetStorage();

                // If no values, leave block empty and exit
                if (!blockNvals) {
                    storage->RemoveBlock(blockIndex);
                    return;
                }

                // Allocate storage for rows, cols, values
                auto type = matrix->GetType();
                auto byteSize = type->GetByteSize();
                auto typeHasValues = byteSize != 0;

                compute::vector<unsigned int> blockRows(blockNvals, ctx);
                compute::vector<unsigned int> blockCols(blockNvals, ctx);
                compute::vector<unsigned char> blockVals(ctx);

                std::vector<unsigned int> blockRowsHost(blockNvals);
                std::vector<unsigned int> blockColsHost(blockNvals);
                std::vector<unsigned char> blockValsHost;

                // If type has non-zero elements size, resize values storage
                if (typeHasValues) {
                    assert(valsHost);
                    blockVals.resize(blockNvals * byteSize, queue);
                    blockValsHost.resize(blockNvals * byteSize);
                }

                // Copy data related to this block
                std::size_t writeOffset = 0;
                {
                    using namespace boost;

                    for (std::size_t k = 0; k < nvalsHost; k++) {
                        auto rowIdx = rowsHost[k];
                        auto colIdx = colsHost[k];

                        if (firstRow <= rowIdx && rowIdx < lastRow &&
                            firstCol <= colIdx && colIdx < lastCol) {

                            // Offset indices, so they are in range [0..blockSize)
                            blockRowsHost[writeOffset] = rowIdx - firstRow;
                            blockColsHost[writeOffset] = colIdx - firstCol;

                            // If has values, copy values
                            if (typeHasValues) {
                                auto src = k * byteSize;
                                auto dst = writeOffset * byteSize;
                                std::memcpy(&blockValsHost[dst], &valsHost[src], byteSize);
                            }

                            writeOffset += 1;
                        }
                    }
                }

                // Copy block data from host to device buffers
                {
                    compute::copy(blockRowsHost.begin(), blockRowsHost.end(), blockRows.begin(), queue);
                    compute::copy(blockColsHost.begin(), blockColsHost.end(), blockCols.begin(), queue);

                    if (typeHasValues)
                        compute::copy(blockValsHost.begin(), blockValsHost.end(), blockVals.begin(), queue);
                }

                // If entries are not sorted, we must sort it here in row-cols order
                if (!desc->IsParamSet(Descriptor::Param::ValuesSorted) && blockNvals > 1) {
                    SPDLOG_LOGGER_TRACE(logger, "Sort block ({},{}) entries", i, j);
                    SortByRowColumn(blockRows, blockCols, blockVals, byteSize, queue);
                }

                if (!desc->IsParamSet(Descriptor::Param::NoDuplicates) && blockNvals > 1) {
                    // Use this mask to find unique elements
                    // NOTE: unique has 1, otherwise 0
                    compute::vector<unsigned int> mask(blockNvals + 1, ctx);
                    mask.begin().write(1u, queue);

                    BOOST_COMPUTE_CLOSURE(
                            unsigned int, findUnique, (unsigned int i), (blockRows, blockCols), {
                                const uint row = blockRows[i];
                                const uint col = blockCols[i];
                                const uint rowPrev = blockRows[i - 1];
                                const uint colPrev = blockCols[i - 1];

                                return rowPrev == row && colPrev == col ? 0 : 1;
                            });

                    // For each entry starting from 1 check if is unique, first is always unique
                    compute::transform(compute::counting_iterator<unsigned int>(1),
                                       compute::counting_iterator<unsigned int>(blockNvals),
                                       mask.begin() + 1,
                                       findUnique,
                                       queue);

                    // Define write offsets (where to write value in result buffer) for each unique value
                    compute::vector<unsigned int> offsets(mask.size(), ctx);
                    compute::exclusive_scan(mask.begin(), mask.end(), offsets.begin(), 0, queue);

                    // Count number of unique values to allocate storage
                    std::size_t resultNvals = (offsets.end() - 1).read(queue);

                    // Allocate new buffers
                    compute::vector<unsigned int> newRows(resultNvals, ctx);
                    compute::vector<unsigned int> newCols(resultNvals, ctx);
                    compute::vector<unsigned char> newVals(ctx);

                    // Copy indices
                    BOOST_COMPUTE_CLOSURE(
                            void, copyIndices, (unsigned int i), (mask, offsets, newRows, newCols, blockRows, blockCols), {
                                if (mask[i]) {
                                    const uint offset = offsets[i];
                                    newRows[offset] = blockRows[i];
                                    newCols[offset] = blockCols[i];
                                }
                            });
                    compute::for_each_n(compute::counting_iterator<unsigned int>(0),
                                        blockNvals,
                                        copyIndices,
                                        queue);

                    // Copy values
                    if (typeHasValues) {
                        newVals.resize(resultNvals * byteSize, queue);

                        BOOST_COMPUTE_CLOSURE(void, copyValues, (unsigned int i), (mask, offsets, newVals, blockVals, byteSize), {
                            if (mask[i]) {
                                const size_t offset = offsets[i];
                                const size_t dst = byteSize * offset;
                                const size_t src = byteSize * i;
                                for (size_t k = 0; k < byteSize; k++) {
                                    newVals[dst + k] = blockVals[src + k];
                                }
                            }
                        });
                        compute::for_each_n(compute::counting_iterator<unsigned int>(0),
                                            blockNvals,
                                            copyValues,
                                            queue);
                    }

                    SPDLOG_LOGGER_TRACE(logger, "Reduce duplicates block ({},{}) entries old={} new={}",
                                        i, j, blockNvals, resultNvals);

                    // Update block data
                    blockNvals = resultNvals;
                    std::swap(blockRows, newRows);
                    std::swap(blockCols, newCols);
                    std::swap(blockVals, newVals);
                }

                // Allocate result block and set in storage
                // Currently, we prefer CSR blocks over COO
                // todo: make auto choice of the best storage scheme
                auto block = MatrixCSR::Make(blockNrows, blockNcols, blockNvals, std::move(blockRows), std::move(blockCols), std::move(blockVals), queue);
                storage->SetBlock(blockIndex, block.As<MatrixBlock>());
            });
        }
    }
}

spla::ExpressionNode::Operation spla::MatrixDataWrite::GetOperationType() const {
    return ExpressionNode::Operation::MatrixDataWrite;
}
