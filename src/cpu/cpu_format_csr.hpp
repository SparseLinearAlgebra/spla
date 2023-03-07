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

#ifndef SPLA_CPU_FORMAT_CSR_HPP
#define SPLA_CPU_FORMAT_CSR_HPP

#include <cpu/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_csr_resize(const uint n_rows,
                        const uint n_values,
                        CpuCsr<T>& storage) {
        storage.Ap.resize(n_rows + 1);
        storage.Aj.resize(n_values);
        storage.Ax.resize(n_values);
        storage.values = n_values;
    }

    template<typename T>
    void cpu_csr_to_dok(uint             n_rows,
                        const CpuCsr<T>& in,
                        CpuDok<T>&       out) {
        auto& Ap = in.Ap;
        auto& Aj = in.Aj;
        auto& Ax = in.Ax;

        assert(out.Ax.empty());

        for (uint i = 0; i < n_rows; i++) {
            for (uint j = Ap[i]; j < Ap[i + 1]; j++) {
                out.Ax.insert(robin_hood::pair<std::pair<uint, uint>, T>(std::pair<uint, uint>(i, Aj[j]), Ax[j]));
            }
        }
    }

    template<typename T>
    void cpu_csr_to_coo(uint             n_rows,
                        const CpuCsr<T>& in,
                        CpuCoo<T>&       out) {
        auto& Ap = in.Ap;
        auto& Aj = in.Aj;
        auto& Ax = in.Ax;

        auto& Ri = out.Ai;
        auto& Rj = out.Aj;
        auto& Rx = out.Ax;

        assert(Ri.size() == in.values);
        assert(Rj.size() == in.values);
        assert(Rx.size() == in.values);

        for (uint i = 0; i < n_rows; i++) {
            for (uint j = Ap[i]; j < Ap[i + 1]; j++) {
                Ri[j] = i;
                Rj[j] = Aj[j];
                Rx[j] = Ax[j];
            }
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMAT_CSR_HPP
