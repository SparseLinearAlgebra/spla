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

#ifndef SPLA_CPU_FORMAT_LIL_HPP
#define SPLA_CPU_FORMAT_LIL_HPP

#include <cpu/cpu_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cpu_lil_resize(uint       n_rows,
                        CpuLil<T>& lil) {
        lil.Ar.resize(n_rows);
    }

    template<typename T>
    void cpu_lil_clear(CpuLil<T>& lil) {
        for (auto& row : lil.Ar) {
            row.clear();
        }
        lil.values = 0;
    }

    template<typename T>
    void cpu_lil_add_element(uint       row_id,
                             uint       col_id,
                             T          element,
                             CpuLil<T>& lil) {
        using Entry = typename CpuLil<T>::Entry;
        auto& row   = lil.Ar[row_id];

        auto where = std::upper_bound(row.begin(), row.end(), Entry{col_id, element}, [](const Entry& val, const Entry& point) {
            return val.first <= point.first;
        });

        if (where != row.end() && (*where).first == col_id) {
            (*where).second = lil.reduce((*where).second, element);
            return;
        }

        row.insert(where, Entry{col_id, element});
        lil.values += 1;
    }

    template<typename T>
    void cpu_lil_to_dok(uint             n_rows,
                        const CpuLil<T>& in,
                        CpuDok<T>&       out) {
        auto& Rx = out.Ax;
        auto& Ar = in.Ar;

        assert(Rx.empty());

        for (uint i = 0; i < n_rows; i++) {
            const auto& row = Ar[i];
            for (uint j = 0; j < row.size(); j++) {
                typename CpuDok<T>::Key key{i, row[j].first};
                Rx[key] = row[j].second;
            }
        }
    }

    template<typename T>
    void cpu_lil_to_coo(uint             n_rows,
                        const CpuLil<T>& in,
                        CpuCoo<T>&       out) {
        auto& Ri = out.Ai;
        auto& Rj = out.Aj;
        auto& Rx = out.Ax;
        auto& Ar = in.Ar;

        assert(Ri.size() == in.values);
        assert(Rj.size() == in.values);
        assert(Rx.size() == in.values);

        uint k = 0;
        for (uint i = 0; i < n_rows; i++) {
            const auto& row = Ar[i];
            for (uint j = 0; j < row.size(); j++) {
                Ri[k] = i;
                Rj[k] = row[j].first;
                Rx[k] = row[j].second;
                k += 1;
            }
        }
    }

    template<typename T>
    void cpu_lil_to_csr(uint             n_rows,
                        const CpuLil<T>& in,
                        CpuCsr<T>&       out) {
        auto& Rp = out.Ap;
        auto& Rj = out.Aj;
        auto& Rx = out.Ax;
        auto& Ar = in.Ar;

        assert(Rp.size() == n_rows + 1);
        assert(Rj.size() == in.values);
        assert(Rx.size() == in.values);

        for (uint i = 0; i < n_rows; i++) {
            Rp[i] = Ar[i].size();
        }

        std::exclusive_scan(Rp.begin(), Rp.end(), Rp.begin(), 0, std::plus<>());
        assert(Rp[n_rows] == in.values);

        uint k = 0;
        for (uint i = 0; i < n_rows; i++) {
            const auto& row = Ar[i];
            for (uint j = 0; j < row.size(); j++) {
                Rj[k] = row[j].first;
                Rx[k] = row[j].second;
                k += 1;
            }
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMAT_LIL_HPP
