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

#include <storage/SplaScalarValue.hpp>

const spla::ScalarValue::Value &spla::ScalarValue::GetVal() const noexcept {
    return mValue;
}

spla::ScalarValue::Value &spla::ScalarValue::GetVal() noexcept {
    return mValue;
}

void spla::ScalarValue::Dump(std::ostream &stream) const {
    using namespace boost;
    compute::context context = mValue.get_buffer().get_context();
    compute::command_queue queue(context, context.get_device());

    std::vector<unsigned char> value;
    compute::copy(mValue.begin(), mValue.end(), value.begin(), queue);

    auto byteSize = value.size();

    stream << "Scalar bsize=" << byteSize << " val=";
    stream << std::hex;

    for (std::size_t byte = 0; byte < byteSize; byte++) {
        stream << static_cast<unsigned int>(value[byte]);
    }

    stream << std::endl
           << std::dec;
}

spla::RefPtr<spla::ScalarValue> spla::ScalarValue::Make(spla::ScalarValue::Value val) {
    return spla::RefPtr<spla::ScalarValue>(new ScalarValue(std::move(val)));
}

spla::ScalarValue::ScalarValue(spla::ScalarValue::Value val) : mValue(std::move(val)) {
}
