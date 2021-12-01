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

#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaMath.hpp>
#include <expression/vector/SplaVectorAssign.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaVectorStorage.hpp>
#include <utils/SplaAlgo.hpp>

bool spla::VectorAssign::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::VectorAssign::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();
    auto &deviceMan = library->GetDeviceManager();

    auto w = node->GetArg(0).Cast<Vector>();
    auto mask = node->GetArg(1).Cast<Vector>();
    auto accum = node->GetArg(2).Cast<FunctionBinary>();
    auto s = node->GetArg(3).Cast<Scalar>();
    auto desc = node->GetDescriptor();

    assert(w.IsNotNull());
    assert(desc.IsNotNull());

    // Create temporary vector for assignment result
    auto tmp = Vector::Make(w->GetNrows(), w->GetType(), w->GetLibrary());

    // If assign is null, make default to keep new entries
    accum = accum.IsNull() ? utils::MakeFunctionChooseSecond(w->GetType()) : accum;

    std::size_t requiredDeviceCount = w->GetStorage()->GetNblockRows();
    auto deviceIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < w->GetStorage()->GetNblockRows(); i++) {
        auto deviceId = deviceIds[i];

        auto assignmentTask = builder.Emplace([=]() {
            auto blockIdx = i;
            auto blockSize = w->GetStorage()->GetBlockSize();
            auto dim = w->GetStorage()->GetNrows();

            ParamsVectorAssign params;
            params.desc = desc;
            params.deviceId = deviceId;
            params.size = math::GetBlockActualSize(blockIdx, dim, blockSize);
            params.hasMask = mask.IsNotNull();
            params.mask = mask.IsNotNull() ? mask->GetStorage()->GetBlock(i) : RefPtr<VectorBlock>{};
            params.s = s.IsNotNull() ? s->GetStorage()->GetValue() : RefPtr<ScalarValue>{};
            params.type = w->GetType();
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorAssign, params);

            if (params.w.IsNotNull()) {
                tmp->GetStorage()->SetBlock(i, params.w);
                SPDLOG_LOGGER_TRACE(logger, "Assign block i={} nnz={}", i, params.w->GetNvals());
            }
        });

        auto accumTask = builder.Emplace([=]() {
            auto tmpBlock = w->GetStorage()->GetBlock(i);

            if (tmpBlock.IsNotNull()) {
                ParamsVectorEWiseAdd params;
                params.desc = desc;
                params.deviceId = deviceId;
                params.a = w->GetStorage()->GetBlock(i);
                params.b = tmp->GetStorage()->GetBlock(i);
                params.op = accum;
                params.type = w->GetType();
                library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorEWiseAdd, params);

                if (params.w.IsNotNull()) {
                    w->GetStorage()->SetBlock(i, params.w);
                    SPDLOG_LOGGER_TRACE(logger, "Accum block i={} nnz={}", i, params.w->GetNvals());
                }
            }
        });

        // Assign and the accum result
        assignmentTask.precede(accumTask);
    }
}

spla::ExpressionNode::Operation spla::VectorAssign::GetOperationType() const {
    return spla::ExpressionNode::Operation::VectorAssign;
}
