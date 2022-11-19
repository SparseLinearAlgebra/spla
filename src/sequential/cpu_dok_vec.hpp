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

#ifndef SPLA_CPU_DOK_VEC_HPP
#define SPLA_CPU_DOK_VEC_HPP

#include <sequential/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_dok_vec_to_coo(const CpuDokVec<T>& in,
                            CpuCooVec<T>&       out) {
        assert(out.Ai.size() == in.values);
        assert(out.Ax.size() == in.values);

        uint k = 0;

        for (const auto& entry : in.Ax) {
            const uint i = entry.first;
            const T    x = entry.second;
            out.Ai[k]    = i;
            out.Ax[k]    = x;
            k += 1;
        }
    }

    template<typename T>
    void cpu_dok_vec_to_dense(const uint          n_rows,
                              const CpuDokVec<T>& in,
                              CpuDenseVec<T>&     out) {
        assert(out.Ax.size() == n_rows);

        for (const auto& entry : in.Ax) {
            const uint i = entry.first;
            const T    x = entry.second;
            out.Ax[i]    = x;
        }
    }

    template<typename T>
    void cpu_dok_vec_add_element(uint          row_id,
                                 T             element,
                                 CpuDokVec<T>& vec) {
        auto entry = vec.Ax.find(row_id);
        if (entry != vec.Ax.end()) {
            entry->second = vec.reduce(entry->second, element);
            return;
        }

        vec.Ax[row_id] = element;
        vec.values += 1;
    }

    template<typename T>
    void cpu_dok_vec_clear(CpuDokVec<T>& vec) {
        vec.values = 0;
        vec.Ax.clear();
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_DOK_VEC_HPP
