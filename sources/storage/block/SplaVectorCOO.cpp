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

#include <storage/block/SplaVectorCOO.hpp>

spla::RefPtr<spla::VectorCOO> spla::VectorCOO::Make(std::size_t nrows, std::size_t nvals, Indices rows, Values vals) {
    return spla::RefPtr<spla::VectorCOO>(new VectorCOO(nrows, nvals, std::move(rows), std::move(vals)));
}

spla::VectorCOO::VectorCOO(std::size_t nrows, std::size_t nvals, Indices rows, Values vals)
    : VectorBlock(nrows, nvals, Format::COO),
      mRows(std::move(rows)),
      mVals(std::move(vals)) {
}

const spla::VectorCOO::Indices &spla::VectorCOO::GetRows() const noexcept {
    return mRows;
}

const spla::VectorCOO::Values &spla::VectorCOO::GetVals() const noexcept {
    return mVals;
}

void spla::VectorCOO::Dump(std::ostream &stream, unsigned int baseI) const {
    using namespace boost;
    compute::context context = mRows.get_buffer().get_context();
    compute::command_queue queue(context, context.get_device());

    std::vector<unsigned int> rows(GetNvals());
    std::vector<unsigned char> vals;

    compute::copy(mRows.begin(), mRows.end(), rows.begin(), queue);

    auto hasValues = !mVals.empty();

    if (hasValues) {
        vals.resize(mVals.size());
        compute::copy(mVals.begin(), mVals.end(), vals.begin(), queue);
    }

    auto byteSize = mVals.size() / GetNvals();

    stream << "Vector " << GetNrows()
           << " nvals=" << GetNvals()
           << " bsize=" << byteSize
           << " format=coo" << std::endl;

    for (std::size_t k = 0; k < GetNvals(); k++) {
        auto i = rows[k] + baseI;

        stream << "[" << k << "] " << i << " ";
        stream << std::hex;

        auto offset = k * byteSize;
        for (std::size_t byte = 0; byte < byteSize; byte++) {
            stream << static_cast<unsigned int>(vals[offset + byte]);
        }

        stream << std::endl
               << std::dec;
    }
}
