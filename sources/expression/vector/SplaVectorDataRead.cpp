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
#include <expression/vector/SplaVectorDataRead.hpp>
#include <storage/SplaVectorStorage.hpp>
#include <storage/block/SplaVectorCOO.hpp>

namespace spla {
    namespace {
        struct VectorDataReadShared {
            /** Blocks entries */
            VectorStorage::EntryList entries;
            /** Number of nnz in each storage blocks' row */
            std::vector<std::size_t> blockRowsNvals;
            /** Offsets of each storage blocks' rows */
            std::vector<std::size_t> blockRowsOffsets;
        };
    }// namespace
}// namespace spla

bool spla::VectorDataRead::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::VectorDataRead::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
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

    auto storage = vector->GetStorage();

    CHECK_RAISE_ERROR(vectorData->GetNvals() >= storage->GetNvals(), InvalidArgument,
                      "Provided data arrays do not have enough space to store vector data");

    auto shared = std::make_shared<VectorDataReadShared>();
    storage->GetBlocks(shared->entries);

    for (const auto &entry : shared->entries) {
        CHECK_RAISE_ERROR(entry.second->GetFormat() == VectorBlock::Format::COO, NotImplemented,
                          "Supported only COO vector block format");
    }

    auto collectNnz = builder.Emplace([=]() {
        auto &blockRowsNvals = shared->blockRowsNvals;
        auto &blockRowsOffsets = shared->blockRowsOffsets;

        blockRowsNvals.resize(storage->GetNblockRows());
        blockRowsOffsets.resize(storage->GetNblockRows());

        // Compute nnz in each row of blocks
        for (const auto &entry : shared->entries) {
            auto &index = entry.first;
            auto row = index;
            auto nnz = entry.second->GetNvals();
            blockRowsNvals[row] = nnz;
        }

        // Compute offset to write rows of blocks
        std::exclusive_scan(blockRowsNvals.begin(), blockRowsNvals.end(), blockRowsOffsets.begin(), std::size_t{0});
    });

    std::size_t requiredDeviceCount = storage->GetNblockRows();
    auto devicesIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < storage->GetNblockRows(); i++) {
        tf::Task copyBlocksInRow;
        auto deviceId = devicesIds[i];
        copyBlocksInRow = builder.Emplace([=]() {
            using namespace boost;

            auto device = library->GetDeviceManager().GetDevice(deviceId);
            compute::context ctx = library->GetContext();
            compute::command_queue queue(ctx, device);

            // Where to start copy process
            std::size_t nvals = shared->blockRowsNvals[i];
            std::size_t offset = shared->blockRowsOffsets[i];
            std::size_t blockFirstRow = i * library->GetBlockSize();

            // If no values - nothing to do
            if (nvals == 0) {
                return;
            }

            auto &entries = shared->entries;

            VectorStorage::Index index = i;
            auto query = std::find_if(entries.begin(), entries.end(),
                                      [index](const auto &p) { return p.first == index; });

            assert(query != entries.end());

            auto block = query->second.Cast<VectorCOO>();

            SPDLOG_LOGGER_TRACE(logger, "Copy vector size={} storage row={} total nvals={} offset={}",
                                storage->GetNrows(), i, nvals, offset);

            auto rows = vectorData->GetRows();
            auto vals = reinterpret_cast<unsigned char *>(vectorData->GetVals());
            assert(rows || vals);

            auto byteSize = vector->GetType()->GetByteSize();
            auto typeHasValues = byteSize != 0;

            if (rows) {
                auto &blockRowsDevice = block->GetRows();
                compute::copy(blockRowsDevice.begin(), blockRowsDevice.end(), &rows[offset], queue);
                for (std::size_t rowRelInd = 0; rowRelInd < nvals; ++rowRelInd) {
                    rows[rowRelInd + offset] += blockFirstRow;
                }
            }

            if (vals && typeHasValues) {
                auto &blockValsDevice = block->GetVals();
                compute::copy(blockValsDevice.begin(), blockValsDevice.end(), &vals[offset * byteSize], queue);
            }
        });

        // Start copy as soon as nnz evaluated
        collectNnz.precede(copyBlocksInRow);
    }
}

spla::ExpressionNode::Operation spla::VectorDataRead::GetOperationType() const {
    return ExpressionNode::Operation::VectorDataRead;
}
