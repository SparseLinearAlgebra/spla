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
#include <expression/vector/SplaVectorEWiseAdd.hpp>
#include <storage/SplaVectorStorage.hpp>

bool spla::VectorEWiseAdd::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::VectorEWiseAdd::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto w = node->GetArg(0).Cast<Vector>();
    auto mask = node->GetArg(1).Cast<Vector>();
    auto op = node->GetArg(2).Cast<FunctionBinary>();
    auto a = node->GetArg(3).Cast<Vector>();
    auto b = node->GetArg(4).Cast<Vector>();
    auto desc = node->GetDescriptor();

    // NOTE: mask and op allowed to be null
    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(b.IsNotNull());
    assert(desc.IsNotNull());

    std::size_t requiredDeviceCount = w->GetStorage()->GetNblockRows();
    auto deviceIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < w->GetStorage()->GetNblockRows(); i++) {
        auto deviceId = deviceIds[i];
        builder.Emplace([=]() {
            ParamsVectorEWiseAdd params;
            params.desc = desc;
            params.deviceId = deviceId;
            params.hasMask = mask.IsNotNull();
            params.mask = mask.IsNotNull() ? mask->GetStorage()->GetBlock(i) : RefPtr<VectorBlock>{};
            params.op = op;
            params.a = a->GetStorage()->GetBlock(i);
            params.b = b->GetStorage()->GetBlock(i);
            params.type = w->GetType();
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorEWiseAdd, params);

            if (params.w.IsNotNull()) {
                w->GetStorage()->SetBlock(i, params.w);
                SPDLOG_LOGGER_TRACE(logger, "Merge block i={} nnz={}", i, params.w->GetNvals());
            } else
                w->GetStorage()->RemoveBlock(i);
        });
    }
}

spla::ExpressionNode::Operation spla::VectorEWiseAdd::GetOperationType() const {
    return ExpressionNode::Operation::VectorEWiseAdd;
}
