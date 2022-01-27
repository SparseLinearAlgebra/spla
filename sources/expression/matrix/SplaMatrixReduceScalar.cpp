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
#include <compute/SplaAdd.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/matrix/SplaMatrixReduceScalar.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <utils/SplaScalarBuffer.hpp>


bool spla::MatrixReduceScalar::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::MatrixReduceScalar::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto desc = node->GetDescriptor();

    auto argScalar = node->GetArg(0).Cast<Scalar>();
    auto argMask = node->GetArg(1).Cast<Matrix>();
    auto argAccum = node->GetArg(2).Cast<FunctionBinary>();
    auto argReduce = node->GetArg(3).Cast<FunctionBinary>();
    auto argMatrix = node->GetArg(4).Cast<Matrix>();
    auto argType = argScalar->GetType();

    const bool hasAccum = argAccum.IsNotNull();

    assert(argScalar.IsNotNull());
    assert(argReduce.IsNotNull());
    assert(argMatrix.IsNotNull());

    if (hasAccum) {
        assert(argScalar.IsNotNull() && argScalar->HasValue());
    }

    if (argMatrix->GetNvals() == 0) {
        argScalar->GetStorage()->RemoveValue();
        SPDLOG_LOGGER_TRACE(logger, "ReduceScalar an empty matrix");
        return;
    }

    auto rowBlocks = argMatrix->GetStorage()->GetNblockRows();
    auto colBlocks = argMatrix->GetStorage()->GetNblockCols();

    auto intermediateBuffer = std::make_shared<detail::ScalarBuffer>();
    auto deviceIds = library->GetDeviceManager().FetchDevices(rowBlocks * colBlocks + 1, node);

    std::vector<tf::Task> reduceBlocksTasks;
    reduceBlocksTasks.reserve(rowBlocks * colBlocks);

    for (std::size_t i = 0; i < rowBlocks; ++i) {
        for (std::size_t j = 0; j < colBlocks; ++j) {
            auto deviceId = deviceIds[i * colBlocks + j];
            auto blockIdx = MatrixStorage::Index(static_cast<unsigned int>(i), static_cast<unsigned int>(j));
            auto block = argMatrix->GetStorage()->GetBlock(blockIdx);

            if (block.IsNotNull()) {
                tf::Task reduceAnotherBlock = builder.Emplace([=]() {
                    ParamsMatrixReduceScalar params;
                    params.desc = desc;
                    params.deviceId = deviceId;
                    params.matrix = block;
                    params.hasMask = argMask.IsNotNull();
                    params.mask = argMask.IsNotNull()
                                          ? argMask->GetStorage()->GetBlock(blockIdx)
                                          : nullptr;
                    params.type = argType;
                    params.reduce = argReduce;
                    library->GetAlgoManager()->Dispatch(Algorithm::Type::MatrixReduceScalar, params);
                    if (params.scalar.IsNotNull()) {
                        intermediateBuffer->Add(params.scalar);
                    }

                    SPDLOG_LOGGER_TRACE(logger, "Reduce matrix block ({}, {}) nnz={}", i, j, params.matrix->GetNvals());
                });
                reduceBlocksTasks.push_back(std::move(reduceAnotherBlock));
            }
        }
    }

    auto lastReduceDeviceId = deviceIds.back();
    tf::Task reduceIntermediateBuffer = builder.Emplace([=]() {
        if (intermediateBuffer->GetNScalars() == 0) {
            argScalar->GetStorage()->RemoveValue();
            return;
        }

        auto &ctx = library->GetContext();
        auto queue = boost::compute::command_queue(ctx, library->GetDeviceManager().GetDevice(lastReduceDeviceId));
        QueueFinisher finisher(queue);

        const std::size_t valueByteSize = argType->GetByteSize();
        boost::compute::vector<unsigned char> reduced = intermediateBuffer->Reduce(argReduce, queue);

#ifdef SPLA_DEBUG
        const std::size_t reducedBufferSize = reduced.size();
#endif

        if (hasAccum) {
            argScalar->GetStorage()->SetValue(ScalarValue::Make(
                    Add(argScalar->GetStorage()->GetValue()->GetVal(),
                        reduced,
                        valueByteSize,
                        argAccum->GetSource(),
                        queue)));
        } else {
            argScalar->GetStorage()->SetValue(ScalarValue::Make(std::move(reduced)));
        }

        SPDLOG_LOGGER_TRACE(logger, "Reduce final reduce of intermediate buffer, nnz={}, accum: {}",
                            reducedBufferSize / valueByteSize, hasAccum ? "true" : "false");
    });

    for (auto &blockReduce : reduceBlocksTasks) {
        blockReduce.precede(reduceIntermediateBuffer);
    }
}

spla::ExpressionNode::Operation spla::MatrixReduceScalar::GetOperationType() const {
    return spla::ExpressionNode::Operation::MatrixReduceScalar;
}
