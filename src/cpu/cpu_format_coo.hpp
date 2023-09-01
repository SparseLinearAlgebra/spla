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

#ifndef SPLA_CPU_FORMAT_COO_HPP
#define SPLA_CPU_FORMAT_COO_HPP

#include <cpu/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_coo_resize(const uint n_values,
                        CpuCoo<T>& storage) {
        storage.Ai.resize(n_values);
        storage.Aj.resize(n_values);
        storage.Ax.resize(n_values);
        storage.values = n_values;
    }

    template<typename T>
    void cpu_coo_clear(CpuCoo<T>& in) {
        in.Ai.clear();
        in.Aj.clear();
        in.Ax.clear();
        in.values = 0;
    }

    template<typename T>
    void cpu_coo_to_lil(uint             n_rows,
                        const CpuCoo<T>& in,
                        CpuLil<T>&       out) {
        auto& Rr = out.Ar;

        auto& Ai = in.Ai;
        auto& Aj = in.Aj;
        auto& Ax = in.Ax;

        assert(Rr.size() == n_rows);

        for (uint k = 0; k < in.values; k++) {
            const uint i = Ai[k];
            const uint j = Aj[k];
            const T    x = Ax[k];

            Rr[i].emplace_back(j, x);
        }
    }

    template<typename T>
    void cpu_coo_to_dok(const CpuCoo<T>& in,
                        CpuDok<T>&       out) {
        auto& Rx = out.Ax;

        auto& Ai = in.Ai;
        auto& Aj = in.Aj;
        auto& Ax = in.Ax;

        assert(Rx.empty());

        for (uint i = 0; i < in.values; i++) {
            typename CpuDok<T>::Key key{Ai[i], Aj[i]};
            Rx[key] = Ax[i];
        }
    }

    template<typename T>
    void cpu_coo_to_csr(uint             n_rows,
                        const CpuCoo<T>& in,
                        CpuCsr<T>&       out) {
        auto& Rp = out.Ap;
        auto& Rj = out.Aj;
        auto& Rx = out.Ax;
        auto& Ai = in.Ai;
        auto& Aj = in.Aj;
        auto& Ax = in.Ax;

        assert(Rp.size() == n_rows + 1);
        assert(Rj.size() == in.values);
        assert(Rx.size() == in.values);

        std::fill(Rp.begin(), Rp.end(), 0u);

        for (uint k = 0; k < in.values; ++k) {
            Rp[Ai[k]] += 1;
        }

        std::exclusive_scan(Rp.begin(), Rp.end(), Rp.begin(), 0, std::plus<>());
        assert(Rp[n_rows] == in.values);

        for (uint k = 0; k < in.values; ++k) {
            Rj[k] = Aj[k];
            Rx[k] = Ax[k];
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMAT_COO_HPP
