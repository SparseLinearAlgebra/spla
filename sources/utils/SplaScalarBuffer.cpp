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

#include <algo/SplaAlgorithmParams.hpp>
#include <compute/SplaReduce2.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <utils/SplaScalarBuffer.hpp>

void spla::detail::ScalarBuffer::Add(RefPtr<ScalarValue> scalar) {
    assert(scalar.IsNotNull());
    std::scoped_lock guard(mMutex);
    mScalars.push_back(std::move(scalar));
}

std::size_t spla::detail::ScalarBuffer::GetNScalars() const noexcept {
    return mScalars.size();
}

spla::RefPtr<spla::ScalarValue> spla::detail::ScalarBuffer::FirstScalar() const {
    return mScalars.front();
}

std::size_t spla::detail::ScalarBuffer::BuildSharedBuffer(std::size_t byteSize, boost::compute::vector<unsigned char> &buffer, boost::compute::command_queue &queue) const {
    if (mScalars.empty()) {
        buffer.resize(0, queue);
        return 0;
    }
    buffer.resize(mScalars.size() * byteSize, queue);
    std::size_t offset = 0;
    for (const auto &scalarValue : mScalars) {
        assert(scalarValue.IsNotNull());
        const auto &curValue = scalarValue->GetVal();
        boost::compute::copy(curValue.begin(), curValue.end(),
                             buffer.begin() + static_cast<std::ptrdiff_t>(offset),
                             queue);
        offset += byteSize;
    }
    return mScalars.size();
}

boost::compute::vector<unsigned char> spla::detail::ScalarBuffer::Reduce(const spla::RefPtr<spla::FunctionBinary> &reduce,
                                                                         boost::compute::command_queue &queue) {

    auto ctx = queue.get_context();

    const std::size_t valueByteSize = reduce->GetA()->GetByteSize();

    if (GetNScalars() == 1) {
        auto &value = FirstScalar()->GetVal();
        return {value, queue};
    }

    boost::compute::vector<unsigned char> buffer(ctx);
    const std::size_t nnzInBuffer = BuildSharedBuffer(valueByteSize, buffer, queue);

    return ::spla::Reduce2(buffer, valueByteSize, reduce->GetSource(), queue);
}
