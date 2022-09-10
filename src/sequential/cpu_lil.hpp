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

#ifndef SPLA_CPU_LIL_HPP
#define SPLA_CPU_LIL_HPP

#include <spla/config.hpp>

#include <core/tdecoration.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CpuLil
     * @brief CPU list-of-list matrix format for fast incremental build
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuLil : public TDecoration<T> {
    public:
        ~CpuLil() override = default;

        using Entry  = std::pair<uint, T>;
        using Row    = std::vector<Entry>;
        using List   = std::vector<Row>;
        using Reduce = std::function<T(T accum, T added)>;

        void clear();
        void resize(uint new_n_rows, uint new_n_cols);
        void add_element(uint row_id, uint col_id, T element);
        void to_coo(std::vector<uint>& Ri, std::vector<uint>& Rj, std::vector<T>& Rx);
        void to_csr(std::vector<uint>& Rp, std::vector<uint>& Rj, std::vector<T>& Rx);

        Reduce reduce = [](T, T a) { return a; };
        List   data{};
        uint   n_rows   = 0;
        uint   n_cols   = 0;
        uint   n_values = 0;
    };

    template<typename T>
    void CpuLil<T>::clear() {
        for (auto& row : data) {
            row.clear();
        }

        n_values = 0;
    }

    template<typename T>
    void CpuLil<T>::resize(uint new_n_rows, uint new_n_cols) {
        data.resize(n_rows);
        n_rows = new_n_rows;
        n_cols = new_n_cols;
    }

    template<typename T>
    void CpuLil<T>::add_element(uint row_id, uint col_id, T element) {
        assert(row_id < n_rows);
        assert(col_id < n_cols);

        Row& row = data[row_id];
        uint i   = 0;

        auto where = std::upper_bound(row.begin(), row.end(), Entry{col_id, element}, [](const Entry& val, const Entry& point) {
            return val.first < point.first;
        });

        if (where != row.end() && *where.first == col_id) {
            *where.second = reduce(*where.second, element);
            return;
        }

        row.insert(where, Entry{col_id, element});
        n_values += 1;
    }

    template<typename T>
    void CpuLil<T>::to_coo(std::vector<uint>& Ri, std::vector<uint>& Rj, std::vector<T>& Rx) {
        assert(Ri.size() == n_values);
        assert(Rj.size() == n_values);
        assert(Rx.size() == n_values);

        uint k = 0;
        for (uint i = 0; i < n_rows; i++) {
            const Row& row = data[i];
            for (uint j = 0; j < row.size(); j++) {
                Ri[k] = i;
                Rj[k] = row[j].first;
                Rx[k] = row[j].second;
                k += 1;
            }
        }
    }

    template<typename T>
    void CpuLil<T>::to_csr(std::vector<uint>& Rp, std::vector<uint>& Rj, std::vector<T>& Rx) {
        assert(Rp.size() == n_rows + 1);
        assert(Rj.size() == n_values);
        assert(Rx.size() == n_values);

        for (uint i = 0; i < n_rows; i++) {
            Rp[i] = data[i].size();
        }

        std::exclusive_scan(Rp.begin(), Rp.end(), Rp.begin(), 0, std::plus<>());

        uint k = 0;
        for (uint i = 0; i < n_rows; i++) {
            const Row& row = data[i];
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

#endif//SPLA_CPU_LIL_HPP
