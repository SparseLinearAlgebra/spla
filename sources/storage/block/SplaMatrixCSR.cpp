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

#include <compute/SplaIndicesToRowOffsets.hpp>
#include <storage/block/SplaMatrixCSR.hpp>

const spla::MatrixCSR::Indices &spla::MatrixCSR::GetRowsOffsets() const noexcept {
    return mRowOffsets;
}

const spla::MatrixCSR::Indices &spla::MatrixCSR::GetRowLengths() const noexcept {
    return mRowLengths;
}

spla::RefPtr<spla::MatrixCSR> spla::MatrixCSR::Make(std::size_t nrows, std::size_t ncols, std::size_t nvals, spla::MatrixCSR::Indices rows, spla::MatrixCSR::Indices cols, spla::MatrixCSR::Values vals, boost::compute::command_queue &queue) {
    return spla::RefPtr<spla::MatrixCSR>(new MatrixCSR(nrows, ncols, nvals, std::move(rows), std::move(cols), std::move(vals), queue));
}

spla::RefPtr<spla::MatrixCSR> spla::MatrixCSR::Make(const spla::RefPtr<spla::MatrixCOO> &block, boost::compute::command_queue &queue) {
    // Copy data from block (blocks are immutable)
    Indices rows(block->GetRows().begin(), block->GetRows().end(), queue);
    Indices cols(block->GetCols().begin(), block->GetCols().end(), queue);
    Values vals(block->GetVals().begin(), block->GetVals().end(), queue);
    return Make(block->GetNrows(), block->GetNcols(), block->GetNvals(), std::move(rows), std::move(cols), std::move(vals), queue);
}

spla::MatrixCSR::MatrixCSR(std::size_t nrows, std::size_t ncols, std::size_t nvals, spla::MatrixCSR::Indices rows, spla::MatrixCSR::Indices cols, spla::MatrixCSR::Values vals, boost::compute::command_queue &queue)
    : MatrixCOO(nrows, ncols, nvals, std::move(rows), std::move(cols), std::move(vals)), mRowOffsets(queue.get_context()), mRowLengths(queue.get_context()) {
    mFormat = Format::CSR;
    // Compute and cache offsets and rows lengths (frequently used in vxm mxm operations)
    IndicesToRowOffsets(GetRows(), mRowOffsets, mRowLengths, GetNrows(), queue);
}
