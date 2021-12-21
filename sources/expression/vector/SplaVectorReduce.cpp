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

namespace spla::detail {

    class ReduceIntermediateBuffer {
    public:
        explicit ReduceIntermediateBuffer(std::size_t size, const RefPtr<Type> &type, Library &library)
            : mBuffer(size) {
            assert(size > 0);
            for (std::size_t i = 0; i < size; ++i) {
                mBuffer[i] = Scalar::Make(type, library);
            }
        }

        RefPtr<Scalar> &operator[](std::size_t i) {
            assert(i < mBuffer.size());
            std::scoped_lock guard(mMutex);
            return mBuffer[i];
        }

        const RefPtr<Scalar> &operator[](std::size_t i) const {
            assert(i < mBuffer.size());
            std::scoped_lock guard(mMutex);
            return mBuffer[i];
        }

        [[nodiscard]] auto BuildSharedBuffer(boost::compute::command_queue &queue) const {
            const std::size_t byteSize = mBuffer[0]->GetType()->GetByteSize();
            boost::compute::vector<unsigned char> sharedBuffer(mBuffer.size() * byteSize, queue.get_context());
            std::size_t offset = 0;
            std::size_t nnz = 0;
            for (const auto &scalar : mBuffer) {
                if (!scalar->HasValue()) {
                    continue;
                }
                nnz += 1;
                const auto &curValue = scalar->GetStorage()->GetValue()->GetVal();
                boost::compute::copy(curValue.begin(), curValue.end(),
                                     sharedBuffer.begin() + static_cast<std::ptrdiff_t>(offset),
                                     queue);
                offset += byteSize;
            }
            return std::pair{std::move(sharedBuffer), nnz};
        }

    private:
        mutable std::mutex mMutex;
        std::vector<RefPtr<Scalar>> mBuffer;
    };

}// namespace spla::detail

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
    auto blocksInVector = argV->GetStorage()->GetNblockRows();

    assert(argS.IsNotNull());
    assert(argOp.IsNotNull());
    assert(argV.IsNotNull());

    auto intermediateBuffer = std::make_shared<detail::ReduceIntermediateBuffer>(blocksInVector, argS->GetType(), node->GetLibrary());
    auto deviceIds = library->GetDeviceManager().FetchDevices(blocksInVector + 1, node);

    std::vector<tf::Task> reduceBlocksTasks;
    reduceBlocksTasks.reserve(blocksInVector);

    for (std::size_t i = 0; i < blocksInVector; ++i) {
        auto deviceId = deviceIds[i];
        auto intermediateBufferElem = (*intermediateBuffer)[i];

        tf::Task reduceIthBlock = builder.Emplace([=]() {
            ParamsVectorReduce params;
            params.desc = desc;
            params.deviceId = deviceId;
            params.v = argV->GetStorage()->GetBlock(i);
            params.tv = argV->GetType();
            params.s = intermediateBufferElem;
            params.ts = argS->GetType();
            params.reduce = argOp;
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorReduce, params);

            SPDLOG_LOGGER_TRACE(logger, "Reduce block i={} nnz={}", i, params.v->GetNvals());
        });
        reduceBlocksTasks.push_back(std::move(reduceIthBlock));
    }

    auto lastReduceDeviceId = deviceIds[blocksInVector];
    tf::Task reduceIntermediateBuffer = builder.Emplace([=]() {
        if (blocksInVector == 1) {
            auto &value = (*intermediateBuffer)[0]->GetStorage()->GetValue()->GetVal();
            argS->GetStorage()->SetValue(ScalarValue::Make(std::move(value)));
            SPDLOG_LOGGER_TRACE(logger, "Primitive final reduce of intermediate buffer, nnz=1");
            return;
        }

        auto &ctx = library->GetContext();
        boost::compute::vector<unsigned char> builtIntermediateBuffer(ctx);
        std::size_t nnzIntermediate;
        {
            auto queue = boost::compute::command_queue(ctx, library->GetDeviceManager().GetDevice(lastReduceDeviceId));
            QueueFinisher finisher(queue);
            auto [buf, nnz] = intermediateBuffer->BuildSharedBuffer(queue);
            builtIntermediateBuffer = std::move(buf);
            nnzIntermediate = nnz;
        }

        auto intermedicateVector = VectorCOO::Make(0,
                                                   nnzIntermediate,
                                                   boost::compute::vector<unsigned int>(ctx),
                                                   std::move(builtIntermediateBuffer))
                                           .Cast<VectorBlock>();
        ParamsVectorReduce params;
        params.desc = desc;
        params.deviceId = lastReduceDeviceId;
        params.v = intermedicateVector;
        params.tv = argV->GetType();
        params.s = argS;
        params.ts = argS->GetType();
        params.reduce = argOp;
        library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorReduce, params);
        SPDLOG_LOGGER_TRACE(logger, "Final reduce of intermediate buffer, nnz={}", nnzIntermediate);
    });

    for (auto &blockReduce : reduceBlocksTasks) {
        blockReduce.precede(reduceIntermediateBuffer);
    }
}

spla::ExpressionNode::Operation spla::VectorReduce::GetOperationType() const {
    return spla::ExpressionNode::Operation::VectorReduce;
}
