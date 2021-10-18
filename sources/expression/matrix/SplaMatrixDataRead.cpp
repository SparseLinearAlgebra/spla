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
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaMath.hpp>
#include <expression/matrix/SplaMatrixDataRead.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

namespace spla {
    namespace {
        struct MatrixDataReadShared {
            /** Blocks entries */
            MatrixStorage::EntryMap entries;
            /** Number of nnz in each storage blocks' row */
            std::vector<std::size_t> blockRowsNvals;
            /** Offsets of each storage blocks' rows */
            std::vector<std::size_t> blockRowsOffsets;
        };
    }// namespace
}// namespace spla

bool spla::MatrixDataRead::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::MatrixDataRead::Process(std::size_t nodeIdx, const spla::Expression &expression, tf::Taskflow &taskflow) {
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

    auto storage = matrix->GetStorage();

    CHECK_RAISE_ERROR(matrixData->GetNvals() >= storage->GetNvals(), InvalidArgument,
                      "Provided data arrays do not have enough space to store matrix data");

    auto shared = std::make_shared<MatrixDataReadShared>();
    storage->GetBlocks(shared->entries);

    for (const auto &entry : shared->entries) {
        CHECK_RAISE_ERROR(entry.second->GetFormat() == MatrixBlock::Format::COO, NotImplemented,
                          "Supported only COO matrix block format");
    }

    auto collectNnz = taskflow.emplace([=]() {
        auto &blockRowsNvals = shared->blockRowsNvals;
        auto &blockRowsOffsets = shared->blockRowsOffsets;

        blockRowsNvals.resize(storage->GetNblockRows(), std::size_t(0));
        blockRowsOffsets.resize(storage->GetNblockRows());

        // Compute nnz in each row of blocks
        for (const auto &entry : shared->entries) {
            auto &index = entry.first;
            auto row = index.first;
            auto nnz = entry.second->GetNvals();
            blockRowsNvals[row] += nnz;
        }

        // Compute offset to write rows of blocks
        std::exclusive_scan(blockRowsNvals.begin(), blockRowsNvals.end(), blockRowsOffsets.begin(), std::size_t(0));
    });

    for (std::size_t i = 0; i < storage->GetNblockRows(); i++) {
        auto copyBlocksInRow = taskflow.emplace([=]() {
            using namespace boost;

            // todo: gpu and device queue management
            compute::device gpu = library->GetDevices()[0];
            compute::context ctx = library->GetContext();
            compute::command_queue queue(ctx, gpu);

            // Where to start copy process
            std::size_t NblockCols = storage->GetNblockCols();
            std::size_t nvals = shared->blockRowsNvals[i];
            std::size_t offset = shared->blockRowsOffsets[i];

            // If no values - nothing to do
            if (nvals == 0) {
                return;
            }

            auto &entries = shared->entries;

            // Blocks to copy
            MatrixStorage::EntryList blocks;
            std::vector<unsigned int> blockColIdx;

            for (std::size_t j = 0; j < NblockCols; j++) {
                MatrixStorage::Index index{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};
                auto query = entries.find(index);

                if (query != entries.end()) {
                    blocks.push_back(*query);
                    blockColIdx.push_back(static_cast<unsigned int>(j));
                }
            }

            SPDLOG_LOGGER_TRACE(logger, "Copy matrix size=({},{}) storage row={} num of blocks={} total nvals={} offset={}",
                                storage->GetNrows(), storage->GetNcols(), i, blocks.size(), nvals, offset);

            auto rows = matrixData->GetRows();
            auto cols = matrixData->GetCols();
            auto vals = reinterpret_cast<unsigned char *>(matrixData->GetVals());
            assert(rows || cols || vals);

            auto blockSize = library->GetBlockSize();
            auto blockNrows = blocks.front().second->GetNrows();
            auto byteSize = matrix->GetType()->GetByteSize();
            auto typeHasValues = byteSize != 0;

            std::vector<std::vector<unsigned int>> blocksRows;
            std::vector<std::vector<unsigned int>> blocksCols;
            std::vector<std::vector<unsigned char>> blocksVals;

            for (auto &k : blocks) {
                auto block = k.second.Cast<MatrixCOO>();
                auto &blockRowsDevice = block->GetRows();
                std::vector<unsigned int> blockRowsHost(blockRowsDevice.size());
                compute::copy(blockRowsDevice.begin(), blockRowsDevice.end(), blockRowsHost.begin(), queue);
                blocksRows.push_back(std::move(blockRowsHost));
            }

            // Copy rows data
            if (rows) {
                std::size_t writeOffset = offset;
                std::vector<std::size_t> readPositions(blocks.size(), 0);
                auto blockFirstRow = static_cast<unsigned int>(i * blockSize);

                for (unsigned int row = 0; row < blockNrows; row++) {
                    for (std::size_t k = 0; k < blocks.size(); k++) {
                        const auto &rowsBuffer = blocksRows[k];
                        auto &readPos = readPositions[k];

                        while (readPos < rowsBuffer.size() && rowsBuffer[readPos] == row) {
                            rows[writeOffset] = row + blockFirstRow;
                            readPos += 1;
                            writeOffset += 1;
                        }
                    }
                }
            }

            // Copy cols data
            if (cols) {
                for (auto &k : blocks) {
                    auto block = k.second.Cast<MatrixCOO>();
                    auto &blockColsDevice = block->GetCols();
                    std::vector<unsigned int> blockColsHost(blockColsDevice.size());
                    compute::copy(blockColsDevice.begin(), blockColsDevice.end(), blockColsHost.begin(), queue);
                    blocksCols.push_back(std::move(blockColsHost));
                }

                std::size_t writeOffset = offset;
                std::vector<std::size_t> readPositions(blocks.size(), 0);

                for (unsigned int row = 0; row < blockNrows; row++) {
                    for (std::size_t k = 0; k < blocks.size(); k++) {
                        const auto &rowsBuffer = blocksRows[k];
                        const auto &colsBuffer = blocksCols[k];
                        auto &readPos = readPositions[k];
                        auto blockFirstCol = static_cast<unsigned int>(blockColIdx[k] * blockSize);

                        while (readPos < rowsBuffer.size() && rowsBuffer[readPos] == row) {
                            cols[writeOffset] = colsBuffer[readPos] + blockFirstCol;
                            readPos += 1;
                            writeOffset += 1;
                        }
                    }
                }
            }

            // Copy vals data
            if (vals && typeHasValues) {
                for (auto &k : blocks) {
                    auto block = k.second.Cast<MatrixCOO>();
                    auto &blockValsDevice = block->GetVals();
                    std::vector<unsigned char> blockValsHost(blockValsDevice.size());
                    compute::copy(blockValsDevice.begin(), blockValsDevice.end(), blockValsHost.begin(), queue);
                    blocksVals.push_back(std::move(blockValsHost));
                }

                std::size_t writeOffset = offset;
                std::vector<std::size_t> readPositions(blocks.size(), 0);

                for (unsigned int row = 0; row < blockNrows; row++) {
                    for (std::size_t k = 0; k < blocks.size(); k++) {
                        const auto &rowsBuffer = blocksRows[k];
                        const auto &valsBuffer = blocksVals[k];
                        auto &readPos = readPositions[k];

                        while (readPos < rowsBuffer.size() && rowsBuffer[readPos] == row) {
                            std::memcpy(&vals[writeOffset * byteSize], &valsBuffer[readPos * byteSize], byteSize);
                            readPos += 1;
                            writeOffset += 1;
                        }
                    }
                }
            }
        });

        // Start copy as soon as nnz evaluated
        collectNnz.precede(copyBlocksInRow);
    }
}

spla::ExpressionNode::Operation spla::MatrixDataRead::GetOperationType() const {
    return ExpressionNode::Operation::MatrixDataRead;
}
