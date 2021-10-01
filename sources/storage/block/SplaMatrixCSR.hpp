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

#ifndef SPLA_SPLAMATRIXCSR_HPP
#define SPLA_SPLAMATRIXCSR_HPP

#include <detail/SplaSvm.hpp>
#include <storage/SplaMatrixBlock.hpp>

namespace spla {

    class MatrixCSR final : public MatrixBlock {
    public:
        ~MatrixCSR() override = default;

        [[nodiscard]] const RefPtr<Svm<unsigned int>> &GetRows() const noexcept {
            return mRowsCompressed;
        }

        [[nodiscard]] const RefPtr<Svm<unsigned int>> &GetCols() const noexcept {
            return mCols;
        }

        [[nodiscard]] const RefPtr<Svm<unsigned char>> &GetVals() const noexcept {
            return mVals;
        }

        static RefPtr<MatrixCSR> Make(size_t nrows, size_t ncols, size_t nvals,
                                      RefPtr<Svm<unsigned int>> rowsCompressed,
                                      RefPtr<Svm<unsigned int>> cols,
                                      RefPtr<Svm<unsigned char>> vals);

    private:
        MatrixCSR(size_t nrows, size_t ncols, size_t nvals,
                  RefPtr<Svm<unsigned int>> rowsCompressed,
                  RefPtr<Svm<unsigned int>> cols,
                  RefPtr<Svm<unsigned char>> vals);

        RefPtr<Svm<unsigned int>> mRowsCompressed;
        RefPtr<Svm<unsigned int>> mCols;
        RefPtr<Svm<unsigned char>> mVals;
    };

}// namespace spla

#endif//SPLA_SPLAMATRIXCSR_HPP
