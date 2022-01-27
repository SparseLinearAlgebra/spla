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

#include <algo/SplaAlgorithmManager.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <expression/vector/SplaVectorToDense.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaVectorStorage.hpp>

bool spla::VectorToDense::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::VectorToDense::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();

    auto argW = node->GetArg(0).Cast<Vector>();
    auto argV = node->GetArg(1).Cast<Vector>();
    auto desc = node->GetDescriptor();

    assert(argW.IsNotNull());
    assert(argV.IsNotNull());

    VectorStorage::EntryList entriesInMatrix;
    VectorStorage::EntryList entries;

    argV->GetStorage()->GetBlocks(entriesInMatrix);
    argW->GetStorage()->Clear();

    // Determine which entries not in dense format to convert
    for (auto& entry: entriesInMatrix) {
        if (entry.second->GetFormat() != VectorBlock::Format::Dense)
            entries.push_back(entry);
        else
            argW->GetStorage()->SetBlock(entry.first, entry.second);
    }

    // No entries => nothing to do
    if (entries.empty())
        return;

    auto devicesCount = entries.size();
    auto devicesIds = library->GetDeviceManager().FetchDevices(devicesCount, node);

    for (std::size_t i = 0; i < entries.size(); i++) {
        auto deviceId = devicesIds[i];
        auto entry = entries[i];
        builder.Emplace("vec-sp2dn", [=]() {
            ParamsVectorToDense params;
            params.deviceId = deviceId;
            params.desc = desc;
            params.v = entry.second;
            params.byteSize = argV->GetType()->GetByteSize();
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorToDense, params);

            if (params.w.IsNotNull()) {
                argW->GetStorage()->SetBlock(entry.first, params.w);
                SPDLOG_LOGGER_TRACE(library->GetLogger(), "Convert block={} to dense", entry.first);
            }
        });
    }
}

spla::ExpressionNode::Operation spla::VectorToDense::GetOperationType() const {
    return spla::ExpressionNode::Operation::VectorToDense;
}
