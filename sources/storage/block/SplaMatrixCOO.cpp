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

#include <sstream>
#include <storage/block/SplaMatrixCOO.hpp>
#include <vector>

spla::RefPtr<spla::MatrixCOO> spla::MatrixCOO::Make(std::size_t nrows, std::size_t ncols, std::size_t nvals, Indices rows, Indices cols, Values vals) {
    return spla::RefPtr<spla::MatrixCOO>(new MatrixCOO(nrows, ncols, nvals, std::move(rows), std::move(cols), std::move(vals)));
}

spla::MatrixCOO::MatrixCOO(std::size_t nrows, std::size_t ncols, std::size_t nvals, Indices rows, Indices cols, Values vals)
    : MatrixBlock(nrows, ncols, nvals, Format::COO),
      mRows(std::move(rows)),
      mCols(std::move(cols)),
      mVals(std::move(vals)) {
}

const spla::MatrixCOO::Indices &spla::MatrixCOO::GetRows() const noexcept {
    return mRows;
}

const spla::MatrixCOO::Indices &spla::MatrixCOO::GetCols() const noexcept {
    return mCols;
}

const spla::MatrixCOO::Values &spla::MatrixCOO::GetVals() const noexcept {
    return mVals;
}

void spla::MatrixCOO::Dump(std::ostream &stream, unsigned int baseI, unsigned int baseJ) const {
    using namespace boost;
    compute::context context = mRows.get_buffer().get_context();
    compute::command_queue queue(context, context.get_device());

    std::vector<unsigned int> rows(GetNvals());
    std::vector<unsigned int> cols(GetNvals());
    std::vector<unsigned char> vals;

    compute::copy(mRows.begin(), mRows.end(), rows.begin(), queue);
    compute::copy(mCols.begin(), mCols.end(), cols.begin(), queue);

    auto hasValues = !mVals.empty();

    if (hasValues) {
        vals.resize(mVals.size());
        compute::copy(mVals.begin(), mVals.end(), vals.begin(), queue);
    }

    auto byteSize = mVals.size() / GetNvals();

    stream << "Matrix " << GetNrows() << "x" << GetNcols()
           << " nvals=" << GetNvals()
           << " bsize=" << byteSize
           << " format=coo" << std::endl;

    for (std::size_t k = 0; k < GetNvals(); k++) {
        auto i = rows[k] + baseI;
        auto j = cols[k] + baseJ;

        stream << "[" << k << "] " << i << " " << j << " ";
        stream << std::hex;

        auto offset = k * byteSize;
        for (std::size_t byte = 0; byte < byteSize; byte++) {
            stream << static_cast<unsigned int>(vals[offset + byte]);
        }

        stream << std::endl
               << std::dec;
    }
}
