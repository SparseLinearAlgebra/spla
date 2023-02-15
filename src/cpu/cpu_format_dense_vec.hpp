/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_CPU_FORMAT_DENSE_VEC_HPP
#define SPLA_CPU_FORMAT_DENSE_VEC_HPP

#include <cpu/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_dense_vec_resize(const uint      n_rows,
                              CpuDenseVec<T>& vec) {
        vec.Ax.resize(n_rows);
        vec.values = n_rows;
    }

    template<typename T>
    void cpu_dense_vec_fill(const T         fill_value,
                            CpuDenseVec<T>& vec) {
        std::fill(vec.Ax.begin(), vec.Ax.end(), fill_value);
    }

    template<typename T>
    void cpu_dense_vec_to_dok(const uint            n_rows,
                              const T               fill_value,
                              const CpuDenseVec<T>& in,
                              CpuDokVec<T>&         out) {
        assert(out.values == 0);
        assert(out.Ax.empty());

        for (uint i = 0; i < n_rows; ++i) {
            if (in.Ax[i] != fill_value) {
                out.Ax[i] = in.Ax[i];
                out.values += 1;
            }
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMAT_DENSE_VEC_HPP
