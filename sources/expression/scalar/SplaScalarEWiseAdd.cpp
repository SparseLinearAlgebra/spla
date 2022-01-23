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
#include <expression/scalar/SplaScalarEWiseAdd.hpp>

bool spla::ScalarEWiseAdd::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::ScalarEWiseAdd::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto node = nodes[nodeIdx];
    auto library = expression.GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto argResult = node->GetArg(0).Cast<Scalar>();
    auto argAdd = node->GetArg(1).Cast<FunctionBinary>();
    auto argA = node->GetArg(2).Cast<Scalar>();
    auto argB = node->GetArg(3).Cast<Scalar>();
    auto desc = node->GetDescriptor();

    assert(argResult.IsNotNull());
    assert(argAdd.IsNotNull());
    assert(argA.IsNotNull());
    assert(argB.IsNotNull());
    assert(desc.IsNotNull());

    auto deviceId = library->GetDeviceManager().FetchDevice(node);

    builder.Emplace([=]() {
        auto device = library->GetDeviceManager().GetDevice(deviceId);
        boost::compute::context ctx = library->GetContext();
        boost::compute::command_queue queue(ctx, device);

        QueueFinisher finisher(queue);
        argResult->GetStorage()->SetValue(ScalarValue::Make(
                Add(argA->GetStorage()->GetValue()->GetVal(),
                    argB->GetStorage()->GetValue()->GetVal(),
                    argResult->GetType()->GetByteSize(),
                    argAdd->GetSource(),
                    queue)));

        SPDLOG_LOGGER_TRACE(logger, "Scalar addition: [{}] x [{}] -> [{}]",
                            argAdd->GetA()->GetByteSize(),
                            argAdd->GetB()->GetByteSize(),
                            argResult->GetType()->GetByteSize());
    });
}

spla::ExpressionNode::Operation spla::ScalarEWiseAdd::GetOperationType() const {
    return ExpressionNode::Operation::ScalarEWiseAdd;
}
