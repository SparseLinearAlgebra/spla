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
        void Add(RefPtr<ScalarValue> scalar) {
            assert(scalar.IsNotNull());
            std::scoped_lock guard(mMutex);
            mScalars.push_back(std::move(scalar));
        }

        [[nodiscard]] std::size_t GetNScalars() const noexcept {
            return mScalars.size();
        }

        [[nodiscard]] RefPtr<ScalarValue> FirstScalar() const {
            return mScalars.front();
        }

        [[nodiscard]] auto BuildSharedBuffer(std::size_t byteSize, boost::compute::command_queue &queue) const {
            if (mScalars.empty()) {
                return std::pair{boost::compute::vector<unsigned char>(queue.get_context()), 0UL};
            }
            boost::compute::vector<unsigned char> sharedBuffer(mScalars.size() * byteSize, queue.get_context());
            std::size_t offset = 0;
            for (const auto &scalarValue : mScalars) {
                assert(scalarValue.IsNotNull());
                const auto &curValue = scalarValue->GetVal();
                boost::compute::copy(curValue.begin(), curValue.end(),
                                     sharedBuffer.begin() + static_cast<std::ptrdiff_t>(offset),
                                     queue);
                offset += byteSize;
            }
            return std::pair{std::move(sharedBuffer), mScalars.size()};
        }

    private:
        mutable std::mutex mMutex;
        std::deque<RefPtr<ScalarValue>> mScalars;
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

    auto intermediateBuffer = std::make_shared<detail::ReduceIntermediateBuffer>();
    auto deviceIds = library->GetDeviceManager().FetchDevices(blocksInVector + 1, node);

    std::vector<tf::Task> reduceBlocksTasks;
    reduceBlocksTasks.reserve(blocksInVector);

    for (std::size_t i = 0; i < blocksInVector; ++i) {
        auto deviceId = deviceIds[i];

        tf::Task reduceIthBlock = builder.Emplace([=]() {
            ParamsVectorReduce params;
            params.desc = desc;
            params.deviceId = deviceId;
            params.vec = argV->GetStorage()->GetBlock(i);
            params.type = argV->GetType();
            params.reduce = argOp;
            library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorReduce, params);
            if (params.scalar.IsNotNull()) {
                intermediateBuffer->Add(params.scalar);
            }
            SPDLOG_LOGGER_TRACE(logger, "Reduce block i={} nnz={}", i, params.vec->GetNvals());
        });
        reduceBlocksTasks.push_back(std::move(reduceIthBlock));
    }

    auto lastReduceDeviceId = deviceIds[blocksInVector];
    tf::Task reduceIntermediateBuffer = builder.Emplace([=]() {
        if (intermediateBuffer->GetNScalars() == 0) {
            argS->GetStorage()->RemoveValue();
            SPDLOG_LOGGER_TRACE(logger, "Final reduce of empty intermediate buffer, nnz=0");
            return;
        }

        auto &ctx = library->GetContext();
        auto queue = boost::compute::command_queue(ctx, library->GetDeviceManager().GetDevice(lastReduceDeviceId));
        QueueFinisher finisher(queue);

        if (intermediateBuffer->GetNScalars() == 1) {
            auto &value = intermediateBuffer->FirstScalar()->GetVal();
            boost::compute::vector<unsigned char> bufferCopy(value, queue);
            queue.finish();
            argS->GetStorage()->SetValue(ScalarValue::Make(std::move(bufferCopy)));
            SPDLOG_LOGGER_TRACE(logger, "Primitive final reduce of intermediate buffer, nnz=1");
            return;
        }

        auto [builtIntermediateBuffer, nnzIntermediate] = intermediateBuffer->BuildSharedBuffer(argV->GetType()->GetByteSize(), queue);
        queue.finish();

        auto intermedicateVector = VectorCOO::Make(0,
                                                   nnzIntermediate,
                                                   boost::compute::vector<unsigned int>(ctx),
                                                   std::move(builtIntermediateBuffer));
        ParamsVectorReduce params;
        params.desc = desc;
        params.deviceId = lastReduceDeviceId;
        params.vec = intermedicateVector.Cast<VectorBlock>();
        params.type = argV->GetType();
        params.reduce = argOp;
        library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorReduce, params);

        boost::compute::vector<unsigned char> reducedVal(params.scalar->GetVal(), queue);
        queue.finish();

        argS->GetStorage()->SetValue(ScalarValue::Make(std::move(reducedVal)));
        SPDLOG_LOGGER_TRACE(logger, "Final reduce of intermediate buffer, nnz={}", nnzIntermediate);
    });

    for (auto &blockReduce : reduceBlocksTasks) {
        blockReduce.precede(reduceIntermediateBuffer);
    }
}

spla::ExpressionNode::Operation spla::VectorReduce::GetOperationType() const {
    return spla::ExpressionNode::Operation::VectorReduce;
}
