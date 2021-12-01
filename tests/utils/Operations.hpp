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

#ifndef SPLA_OPERATIONS_HPP
#define SPLA_OPERATIONS_HPP

#include <utils/Matrix.hpp>
#include <utils/Vector.hpp>

namespace utils {

    template<typename T, typename Mult, typename Add>
    inline Vector<T> VxM(const Vector<T> &a, const Matrix<T> &b, Mult mult, Add add) {
        assert(a.GetNrows() == b.GetNrows());

        using Index = typename Vector<T>::Index;

        std::vector<Index> rows;
        std::vector<T> vals;

        std::size_t n = b.GetNcols();
        std::vector<T> tmp(n);
        std::vector<bool> mask(n, false);
        std::vector<Index> nnz;

        auto bOffsets = ToOffsets(b.GetNrows(), b.GetRowsVec());

        for (std::size_t ak = 0; ak < a.GetNvals(); ak++) {
            auto aRowId = a.GetRowsVec()[ak];
            auto aVal = a.GetValsVec()[ak];

            for (std::size_t bk = bOffsets[aRowId]; bk < bOffsets[aRowId + 1]; bk++) {
                auto bColId = b.GetColsVec()[bk];
                auto bVal = b.GetValsVec()[bk];

                if (mask[bColId])
                    tmp[bColId] = add(tmp[bColId], mult(aVal, bVal));
                else {
                    mask[bColId] = true;
                    nnz.push_back(bColId);
                    tmp[bColId] = mult(aVal, bVal);
                }
            }
        }

        if (!nnz.empty()) {
            std::sort(nnz.begin(), nnz.end());
            for (auto k : nnz) {
                rows.push_back(k);
                vals.push_back(tmp[k]);
            }
        }

        return Vector<T>(n, std::move(rows), std::move(vals));
    }

    template<typename T, typename M, typename Mult, typename Add>
    inline Vector<T> VxM(const Vector<M> &mask, bool complement, const Vector<T> &a, const Matrix<T> &b, Mult mult, Add add) {
        return VxM(a, b, mult, add).Mask(mask, complement);
    }

}// namespace utils

#endif//SPLA_OPERATIONS_HPP
