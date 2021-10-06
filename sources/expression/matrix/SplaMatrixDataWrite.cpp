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
#include <compute/SplaGather.hpp>
#include <detail/SplaLibraryPrivate.hpp>
#include <detail/SplaMath.hpp>
#include <expression/matrix/SplaMatrixDataWrite.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <vector>

bool spla::MatrixDataWrite::Select(size_t nodeIdx, spla::ExpressionContext &context) {
    return true;
}

void spla::MatrixDataWrite::Process(size_t nodeIdx, spla::ExpressionContext &context) {
    auto &taskflow = context.nodesTaskflow[nodeIdx];
    auto &nodes = context.expression->GetNodes();
    auto node = nodes[nodeIdx];
    auto library = context.expression->GetLibrary().GetPrivatePtr();
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

    for (size_t i = 0; i < math::GetBlocksCount(nrows, blockSize); i++) {
        for (size_t j = 0; j < math::GetBlocksCount(ncols, blockSize); j++) {
            taskflow.emplace([=]() {
                using namespace boost;

                // todo: gpu and device queue management
                boost::compute::device gpu = library->GetDevices()[0];
                boost::compute::context ctx = library->GetContext();
                boost::compute::command_queue queue(ctx, gpu);

                auto blockIndex = MatrixStorage::Index{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};
                auto blockNrows = math::GetBlockActualSize(i, nrows, blockSize);
                auto blockNcols = math::GetBlockActualSize(i, ncols, blockSize);

                auto firstRow = static_cast<unsigned int>(i * blockSize);
                auto firstCol = static_cast<unsigned int>(j * blockSize);
                auto lastRow = firstRow + static_cast<unsigned int>(blockNrows);
                auto lastCol = firstCol + static_cast<unsigned int>(blockNcols);

                SPDLOG_LOGGER_TRACE(logger, "Process block ({},{}) size=({},{}) ranges=([{}..{}),[{}..{}))",
                                    i, j, blockNrows, blockNcols, firstRow, lastRow, firstCol, lastCol);

                auto rowsHost = matrixData->GetRows();
                auto colsHost = matrixData->GetCols();
                auto valsHost = reinterpret_cast<unsigned char *>(matrixData->GetVals());
                auto nvalsHost = matrixData->GetNvals();

                assert(rowsHost);
                assert(colsHost);

                // Count number of nnz values to store in this block
                size_t blockNvals = 0;
                {
                    for (size_t k = 0; k < nvalsHost; k++) {
                        auto rowIdx = rowsHost[k];
                        auto colIdx = colsHost[k];

                        if (firstRow <= rowIdx && rowIdx < lastRow &&
                            firstCol <= colIdx && colIdx < lastCol)
                            blockNvals += 1;
                    }
                }
                SPDLOG_LOGGER_TRACE(logger, "Block ({},{}) nvals={}", i, j, blockNvals);
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
                    blockVals.resize(blockNvals * byteSize);
                    blockValsHost.resize(blockNvals * byteSize);
                }

                // Copy data related to this block
                size_t writeOffset = 0;
                {
                    using namespace boost;

                    for (size_t k = 0; k < nvalsHost; k++) {
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
                if (!desc->IsParamSet(Descriptor::Param::ValuesSorted)) {
                    SPDLOG_LOGGER_TRACE(logger, "Sort block ({},{}) entries", i, j);

                    compute::vector<unsigned int> permutation(blockNvals, ctx);
                    compute::copy(compute::counting_iterator<cl_uint>(0),
                                  compute::counting_iterator<cl_uint>(blockNvals),
                                  permutation.begin(),
                                  queue);

                    compute::vector<unsigned int> tmp(blockNvals, ctx);
                    compute::copy(blockCols.begin(), blockCols.end(), tmp.begin(), queue);

                    // Sort in column order and then, using permutation, shuffle row indices
                    compute::sort_by_key(tmp.begin(), tmp.end(), permutation.begin(), queue);
                    compute::gather(permutation.begin(), permutation.end(), blockRows.begin(), tmp.begin(), queue);

                    // Sort in row order and then, using permutation, shuffle column indices
                    compute::sort_by_key(tmp.begin(), tmp.end(), permutation.begin(), queue);
                    compute::copy(tmp.begin(), tmp.end(), blockRows.begin(), queue);
                    compute::copy(blockCols.begin(), blockCols.end(), tmp.begin(), queue);
                    compute::gather(permutation.begin(), permutation.end(), tmp.begin(), blockCols.begin(), queue);

                    // Copy values, using permutation
                    if (typeHasValues) {
                        compute::vector<unsigned char> valsTmp(blockNvals * byteSize, ctx);
                        Gather(permutation.begin(), permutation.end(), blockVals.begin(), valsTmp.begin(), byteSize, queue);
                        std::swap(blockVals, valsTmp);
                    }
                }

                if (!desc->IsParamSet(Descriptor::Param::NoDuplicates)) {
                    SPDLOG_LOGGER_TRACE(logger, "Reduce duplicates block ({},{}) entries", i, j);
                    // todo: reduce duplicates if present
                }

                // Allocate result block and set in storage
                auto block = MatrixCOO::Make(blockNrows, blockNcols, blockNvals, std::move(blockRows), std::move(blockCols), std::move(blockVals));
                storage->SetBlock(blockIndex, block.As<MatrixBlock>());
            });
        }
    }
}

spla::ExpressionNode::Operation spla::MatrixDataWrite::GetOperationType() const {
    return ExpressionNode::Operation::MatrixDataWrite;
}
