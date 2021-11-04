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
#include <boost/compute/container/vector.hpp>
#include <boost/compute/iterator.hpp>
#include <compute/SplaGather.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaMath.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/vector/SplaVectorDataWrite.hpp>
#include <storage/SplaVectorStorage.hpp>
#include <storage/block/SplaVectorCOO.hpp>

bool spla::VectorDataWrite::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::VectorDataWrite::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto vector = node->GetArg(0).Cast<Vector>();
    auto vectorData = node->GetArg(1).Cast<DataVector>();
    auto desc = node->GetDescriptor();

    assert(vector.IsNotNull());
    assert(vectorData.IsNotNull());
    assert(desc.IsNotNull());

    auto nrows = vector->GetNrows();
    auto blockSize = library->GetBlockSize();

    std::size_t blockCountInRow = math::GetBlocksCount(nrows, blockSize);

    auto requiredDeviceCount = blockCountInRow;
    auto devicesIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < blockCountInRow; i++) {
        auto deviceId = devicesIds[i];
        builder.Emplace([=]() {
            using namespace boost;

            compute::device device = library->GetDeviceManager().GetDevice(deviceId);
            compute::context ctx = library->GetContext();
            compute::command_queue queue(ctx, device);
            QueueFinisher finisher(queue);

            auto blockIndex = VectorStorage::Index{static_cast<unsigned int>(i)};
            auto blockNrows = math::GetBlockActualSize(i, nrows, blockSize);

            auto firstRow = static_cast<unsigned int>(i * blockSize);
            auto lastRow = firstRow + static_cast<unsigned int>(blockNrows);

            auto rowsHost = vectorData->GetRows();
            auto valsHost = reinterpret_cast<unsigned char *>(vectorData->GetVals());
            auto nvalsHost = vectorData->GetNvals();

            assert(rowsHost);

            // Count number of nnz values to store in this block
            std::size_t blockNvals = 0;
            {
                for (std::size_t k = 0; k < nvalsHost; k++) {
                    auto rowIdx = rowsHost[k];

                    if (firstRow <= rowIdx && rowIdx < lastRow)
                        blockNvals += 1;
                }
            }

            SPDLOG_LOGGER_TRACE(logger, "Process vector block {} size={} range={}..{} nvals={}",
                                i, blockNrows, firstRow, lastRow, blockNvals);

            auto storage = vector->GetStorage();

            // If no values, leave block empty and exit
            if (!blockNvals) {
                storage->RemoveBlock(blockIndex);
                return;
            }

            // Allocate storage for rows, cols, values
            auto type = vector->GetType();
            auto byteSize = type->GetByteSize();
            auto typeHasValues = byteSize != 0;

            compute::vector<unsigned int> blockRows(blockNvals, ctx);
            compute::vector<unsigned char> blockVals(ctx);

            std::vector<unsigned int> blockRowsHost(blockNvals);
            std::vector<unsigned char> blockValsHost;

            // If type has non-zero elements size, resize values storage
            if (typeHasValues) {
                blockVals.resize(blockNvals * byteSize, queue);
                blockValsHost.resize(blockNvals * byteSize);
            }

            // Copy data related to this block
            std::size_t writeOffset = 0;
            {
                using namespace boost;

                for (std::size_t k = 0; k < nvalsHost; k++) {
                    auto rowIdx = rowsHost[k];

                    if (firstRow <= rowIdx && rowIdx < lastRow) {
                        // Offset indices, so they are in range [0..blockSize)
                        blockRowsHost[writeOffset] = rowIdx - firstRow;

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
                if (typeHasValues) {
                    compute::copy(blockValsHost.begin(), blockValsHost.end(), blockVals.begin(), queue);
                }
            }

            // If entries are not sorted, we must sort it here in row-cols order
            if (!desc->IsParamSet(Descriptor::Param::ValuesSorted) && blockNvals > 1) {
                SPDLOG_LOGGER_TRACE(logger, "Sort vector block {} entries", i);

                compute::vector<unsigned int> permutation(blockNvals, ctx);
                compute::copy(compute::counting_iterator<cl_uint>(0),
                              compute::counting_iterator<cl_uint>(blockNvals),
                              permutation.begin(),
                              queue);

                compute::sort_by_key(blockRows.begin(), blockRows.end(), permutation.begin(), queue);

                // Copy values, using permutation
                if (typeHasValues) {
                    compute::vector<unsigned char> valsTmp(blockNvals * byteSize, ctx);
                    Gather(permutation.begin(), permutation.end(), blockVals.begin(), valsTmp.begin(), byteSize, queue);
                    std::swap(blockVals, valsTmp);
                }
            }

            if (!desc->IsParamSet(Descriptor::Param::NoDuplicates) && blockNvals > 1) {
                compute::vector<unsigned int> permutation(blockNvals, ctx);
                compute::copy(compute::counting_iterator<cl_uint>(0),
                              compute::counting_iterator<cl_uint>(blockNvals),
                              permutation.begin(),
                              queue);

                compute::vector<unsigned int> reducedPermutation(blockNvals, ctx);
                compute::vector<unsigned int> reducedRows(blockNvals, ctx);

                auto [rowsResEnd, permResEnd] = compute::reduce_by_key(
                        blockRows.begin(), blockRows.end(),
                        permutation.begin(),
                        reducedRows.begin(), reducedPermutation.begin(),
                        boost::compute::min<unsigned int>(),
                        queue);

                blockNvals = std::distance(reducedPermutation.begin(), permResEnd);
                reducedRows.resize(std::distance(reducedRows.begin(), rowsResEnd), queue);
                std::swap(blockRows, reducedRows);

                // Copy values, using permutation
                if (typeHasValues) {
                    compute::vector<unsigned char> valsTmp(blockNvals * byteSize, ctx);
                    Gather(reducedPermutation.begin(), permResEnd, blockVals.begin(), valsTmp.begin(), byteSize, queue);
                    std::swap(blockVals, valsTmp);
                }
            }

            // Allocate result block and set in storage
            auto block = VectorCOO::Make(blockNrows, blockNvals, std::move(blockRows), std::move(blockVals));
            storage->SetBlock(blockIndex, block.As<VectorBlock>());
        });
    }
}

spla::ExpressionNode::Operation spla::VectorDataWrite::GetOperationType() const {
    return ExpressionNode::Operation::VectorDataWrite;
}
