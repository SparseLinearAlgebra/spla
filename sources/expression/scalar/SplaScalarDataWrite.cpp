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
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <expression/scalar/SplaScalarDataWrite.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaScalarValue.hpp>

bool spla::ScalarDataWrite::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::ScalarDataWrite::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto &library = node->GetLibrary().GetPrivate();
    auto &logger = library.GetLogger();

    auto scalar = node->GetArg(0).Cast<Scalar>();
    auto data = node->GetArg(1).Cast<DataScalar>();
    auto desc = node->GetDescriptor();
    auto &type = scalar->GetType();
    auto byteSize = type->GetByteSize();
    auto hostValue = reinterpret_cast<const unsigned char *>(data->GetValue());

    assert(scalar);
    assert(data);
    assert(desc);
    assert(type->HasValues());
    assert(hostValue);

    using namespace boost;

    auto &deviceMan = library.GetDeviceManager();
    auto deviceId = deviceMan.FetchDevice(node);

    compute::device device = deviceMan.GetDevice(deviceId);
    compute::context ctx = library.GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    compute::vector<unsigned char> deviceValue(byteSize, ctx);
    compute::copy(hostValue, hostValue + byteSize, deviceValue.begin(), queue);

    auto scalarValue = ScalarValue::Make(std::move(deviceValue));
    scalar->GetStorage()->SetValue(scalarValue);

    SPDLOG_LOGGER_TRACE(logger, "Write value byteSize={}", byteSize);
}

spla::ExpressionNode::Operation spla::ScalarDataWrite::GetOperationType() const {
    return ExpressionNode::Operation::ScalarDataWrite;
}
