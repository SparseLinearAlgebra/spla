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

#ifndef SPLA_CPU_FORMAT_COO_VEC_HPP
#define SPLA_CPU_FORMAT_COO_VEC_HPP

#include <cpu/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_coo_vec_resize(const uint    n_values,
                            CpuCooVec<T>& vec) {
        vec.Ai.resize(n_values);
        vec.Ax.resize(n_values);
        vec.values = n_values;
    }

    template<typename T>
    void cpu_coo_vec_clear(CpuCooVec<T>& vec) {
        vec.Ai.clear();
        vec.Ax.clear();
        vec.values = 0;
    }

    template<typename T>
    void cpu_coo_vec_to_dok(const CpuCooVec<T>& in,
                            CpuDokVec<T>&       out) {
        assert(out.values == 0);
        assert(out.Ax.empty());

        for (std::size_t k = 0; k < in.Ai.size(); ++k) {
            const uint i = in.Ai[k];
            const T    x = in.Ax[k];
            out.Ax[i]    = x;
            out.values += 1;
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMAT_COO_VEC_HPP
