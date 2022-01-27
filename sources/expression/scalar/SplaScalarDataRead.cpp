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
#include <expression/scalar/SplaScalarDataRead.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaScalarValue.hpp>

bool spla::ScalarDataRead::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::ScalarDataRead::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto &library = node->GetLibrary().GetPrivate();

    auto scalar = node->GetArg(0).Cast<Scalar>();
    auto data = node->GetArg(1).Cast<DataScalar>();
    auto desc = node->GetDescriptor();
    auto hostValue = reinterpret_cast<unsigned char *>(data->GetValue());

    assert(scalar);
    assert(data);
    assert(desc);
    assert(hostValue);

    auto deviceValue = scalar->GetStorage()->GetValue();

    if (deviceValue.IsNotNull()) {
        using namespace boost;

        auto &deviceMan = library.GetDeviceManager();
        auto deviceId = deviceMan.FetchDevice(node);

        compute::device device = deviceMan.GetDevice(deviceId);
        compute::context ctx = library.GetContext();
        compute::command_queue queue(ctx, device);
        QueueFinisher finisher(queue, library.GetLogger());

        compute::copy(deviceValue->GetVal().begin(), deviceValue->GetVal().end(), hostValue, queue);
    }
}

spla::ExpressionNode::Operation spla::ScalarDataRead::GetOperationType() const {
    return ExpressionNode::Operation::ScalarDataRead;
}
