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

#include <mutex>

#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/vector/SplaVectorReduce.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaVectorStorage.hpp>
#include <storage/block/SplaVectorCOO.hpp>
#include <utils/SplaScalarBuffer.hpp>

bool spla::VectorReduce::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::VectorReduce::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();
    auto &deviceMan = library->GetDeviceManager();

    auto argS = node->GetArg(0).Cast<Scalar>();
    auto argOp = node->GetArg(1).Cast<FunctionBinary>();
    auto argV = node->GetArg(2).Cast<Vector>();
    auto desc = node->GetDescriptor();

    assert(argS.IsNotNull());
    assert(argOp.IsNotNull());
    assert(argV.IsNotNull());

    if (argV->GetNvals() == 0) {
        argS->GetStorage()->RemoveValue();
        SPDLOG_LOGGER_TRACE(logger, "Reduce an empty vector");
        return;
    }

    auto blocksInVector = argV->GetStorage()->GetNblockRows();
    auto intermediateBuffer = std::make_shared<detail::ScalarBuffer>();
    auto deviceIds = library->GetDeviceManager().FetchDevices(blocksInVector + 1, node);

    std::vector<tf::Task> reduceBlocksTasks;
    reduceBlocksTasks.reserve(blocksInVector);

    for (std::size_t i = 0; i < blocksInVector; ++i) {
        auto deviceId = deviceIds[i];
        auto blockVec = argV->GetStorage()->GetBlock(i);

        if (blockVec.IsNotNull()) {
            tf::Task reduceIthBlock = builder.Emplace([=]() {
                ParamsVectorReduce params;
                params.desc = desc;
                params.deviceId = deviceId;
                params.vec = blockVec;
                params.type = argV->GetType();
                params.reduce = argOp;
                library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorReduce, params);
                if (params.scalar.IsNotNull()) {
                    intermediateBuffer->Add(params.scalar);
                }
                SPDLOG_LOGGER_TRACE(logger, "VectorReduce: block i={} nnz={}", i, params.vec->GetNvals());
            });
            reduceBlocksTasks.push_back(std::move(reduceIthBlock));
        }
    }

    auto lastReduceDeviceId = deviceIds[blocksInVector];
    tf::Task reduceIntermediateBuffer = builder.Emplace([=]() {
        auto &ctx = library->GetContext();
        auto queue = boost::compute::command_queue(ctx, library->GetDeviceManager().GetDevice(lastReduceDeviceId));
        QueueFinisher finisher(queue);
        argS->GetStorage()->SetValue(ScalarValue::Make(intermediateBuffer->Reduce(argOp, queue)));
        SPDLOG_LOGGER_TRACE(logger, "VectorReduce: final reduce of intermediate buffer, nnz={}", intermediateBuffer->GetNScalars());
    });

    for (auto &blockReduce : reduceBlocksTasks) {
        blockReduce.precede(reduceIntermediateBuffer);
    }
}

spla::ExpressionNode::Operation spla::VectorReduce::GetOperationType() const {
    return spla::ExpressionNode::Operation::VectorReduce;
}
