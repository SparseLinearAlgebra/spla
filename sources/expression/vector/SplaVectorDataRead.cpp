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

#include <algo/SplaAlgorithmManager.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <expression/vector/SplaVectorDataRead.hpp>
#include <storage/SplaVectorStorage.hpp>

#include <algorithm>

namespace spla {
    namespace {
        struct VectorDataReadShared {
            /** Blocks entries */
            VectorStorage::EntryList entries;
            /** Offsets of each storage blocks' rows */
            std::vector<std::size_t> blockRowsOffsets;
        };
    }// namespace
}// namespace spla

bool spla::VectorDataRead::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::VectorDataRead::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();

    auto vector = node->GetArg(0).Cast<Vector>();
    auto vectorData = node->GetArg(1).Cast<DataVector>();
    auto desc = node->GetDescriptor();

    assert(vector.IsNotNull());
    assert(vectorData.IsNotNull());
    assert(desc.IsNotNull());

    auto shared = std::make_shared<VectorDataReadShared>();
    auto storage = vector->GetStorage();

    CHECK_RAISE_ERROR(vectorData->GetNvals() >= storage->GetNvals(), InvalidArgument,
                      "Provided data arrays do not have enough space to store vector data");

    // Read-only storage copy
    storage->GetBlocks(shared->entries);
    std::size_t entries = shared->entries.size();

    // Sort blocks by id
    std::sort(shared->entries.begin(), shared->entries.end(),
              [](const VectorStorage::Entry &a, const VectorStorage::Entry &b) { return a.first < b.first; });

    // Compute nnz in each row of blocks
    std::vector<std::size_t> blockRowsNvals(entries);
    std::transform(shared->entries.begin(), shared->entries.end(), blockRowsNvals.begin(),
                   [](const VectorStorage::Entry &e) { return e.second->GetNvals(); });

    // Compute offset to write rows of blocks
    shared->blockRowsOffsets.resize(entries);
    std::exclusive_scan(blockRowsNvals.begin(), blockRowsNvals.end(),
                        shared->blockRowsOffsets.begin(), std::size_t{0});

    // Fetch device
    auto deviceId = library->GetDeviceManager().FetchDevice(node);

    // Copy data of each block to the vectorData
    for (std::size_t i = 0; i < shared->entries.size(); i++) {
        builder.Emplace("vec-read", [=]() {
            auto entry = shared->entries[i];
            ParamsVectorRead params;
            params.deviceId = deviceId;
            params.desc = desc;
            params.byteSize = vector->GetType()->GetByteSize();
            params.offset = shared->blockRowsOffsets[i];
            params.baseI = entry.first * storage->GetBlockSize();
            params.v = entry.second;
            params.d = vectorData;
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorRead, params);
        });
    }
}

spla::ExpressionNode::Operation spla::VectorDataRead::GetOperationType() const {
    return ExpressionNode::Operation::VectorDataRead;
}
