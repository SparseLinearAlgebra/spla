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

#include <spla-algo/SplaAlgoCommon.hpp>

spla::HostVector::HostVector(spla::Size nrows, std::vector<Index> rows, std::vector<unsigned char> vals)
    : mNrows(nrows), mNnvals(rows.size()), mRowIndices(std::move(rows)), mValues(std::move(vals)) {
    mElementSize = !mRowIndices.empty() ? mValues.size() / mRowIndices.size() : 0;
}

spla::RefPtr<spla::DataVector> spla::HostVector::GetData(Library &library) {
    return DataVector::Make(mRowIndices.data(), HasValues() ? mValues.data() : nullptr, GetNnvals(), library);
}

spla::HostMatrix::HostMatrix(spla::Size nrows, spla::Size ncols, std::vector<Index> rows, std::vector<Index> cols, std::vector<unsigned char> vals)
    : mNrows(nrows), mNcols(ncols), mNnvals(rows.size()), mRowIndices(std::move(rows)), mColIndices(std::move(cols)), mValues(std::move(vals)) {
    mElementSize = !mRowIndices.empty() ? mValues.size() / mRowIndices.size() : 0;
}

spla::RefPtr<spla::DataMatrix> spla::HostMatrix::GetData(Library &library) {
    return DataMatrix::Make(mRowIndices.data(), mColIndices.data(), HasValues() ? mValues.data() : nullptr, GetNnvals(), library);
}