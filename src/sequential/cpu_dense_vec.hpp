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

#ifndef SPLA_CPU_DENSE_VEC_HPP
#define SPLA_CPU_DENSE_VEC_HPP

#include <sequential/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_dense_vec_resize(uint            n_rows,
                              CpuDenseVec<T>& vec) {
        vec.Ax.clear();
        vec.Ax.resize(n_rows);
        vec.values = n_rows;
    }

    template<typename T>
    void cpu_dense_vec_fill(T               value,
                            CpuDenseVec<T>& vec) {
        std::fill(vec.Ax.begin(), vec.Ax.end(), value);
    }

    template<typename T>
    void cpu_dense_vec_add_element(uint            row_id,
                                   T               element,
                                   CpuDenseVec<T>& vec) {
        vec.Ax[row_id] = vec.reduce(vec.Ax[row_id], element);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_DENSE_VEC_HPP