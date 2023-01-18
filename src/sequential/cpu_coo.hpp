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

#ifndef SPLA_CPU_COO_HPP
#define SPLA_CPU_COO_HPP

#include <sequential/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

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

#endif//SPLA_CPU_COO_HPP
