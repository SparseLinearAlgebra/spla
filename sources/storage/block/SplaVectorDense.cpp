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

#include <cassert>

#include <storage/block/SplaVectorDense.hpp>

spla::RefPtr<spla::VectorDense> spla::VectorDense::Make(std::size_t nrows, std::size_t nvals, Mask mask, Values vals) {
    return {new VectorDense(nrows, nvals, std::move(mask), std::move(vals))};
}

spla::VectorDense::VectorDense(std::size_t nrows, std::size_t nvals, Mask mask, Values vals)
    : VectorBlock(nrows, nvals, Format::Dense), mMask(std::move(mask)), mVals(std::move(vals)) {
}

const spla::VectorDense::Mask &spla::VectorDense::GetMask() const noexcept {
    return mMask;
}

const spla::VectorDense::Values &spla::VectorDense::GetVals() const noexcept {
    return mVals;
}

std::size_t spla::VectorDense::GetMemoryUsage() const {
    return mMask.size() * sizeof(Index) + mVals.size();
}

void spla::VectorDense::Dump(std::ostream &stream, unsigned int baseI) const {
    using namespace boost;
    compute::context context = mVals.get_buffer().get_context();
    compute::command_queue queue(context, context.get_device());

    std::vector<unsigned int> mask(mMask.size());
    std::vector<unsigned char> vals;

    compute::copy(mMask.begin(), mMask.end(), mask.begin(), queue);

    auto hasValues = !mVals.empty();
    auto byteSize = mVals.size() / GetNvals();

    if (hasValues) {
        vals.resize(mVals.size());
        compute::copy(mVals.begin(), mVals.end(), vals.begin(), queue);
    }

    stream << "Vector " << GetNrows()
           << " nvals=" << GetNvals()
           << " bsize=" << byteSize
           << " format=dense" << std::endl;

    std::size_t k = 0;

    for (std::size_t i = 0; i < GetNrows(); i++) {
        if (mask[i]) {
            stream << "[" << k++ << "] " << baseI + i << " ";
            stream << std::hex;

            auto offset = i * byteSize;
            for (std::size_t byte = 0; byte < byteSize; byte++) {
                stream << static_cast<unsigned int>(vals[offset + byte]);
            }

            stream << std::endl
                   << std::dec;
        }
    }
}