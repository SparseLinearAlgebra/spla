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

#ifndef SPLA_SHARED_REDUCE_HPP
#define SPLA_SHARED_REDUCE_HPP

#include <vector>

#include <spla/types.hpp>

namespace spla::backend {

    /**
     * @addtogroup shared
     * @{
     */

    template<typename T, typename ReduceOp>
    inline void reduce(const std::vector<Index> &indices,
                       const std::vector<T> &values,
                       const ReduceOp &reduceOp,
                       std::vector<Index> &out_indices,
                       std::vector<T> &out_values) {
        std::size_t dst_pos = 0;
        std::size_t nvals = indices.size();
        std::vector<Index> reduced_indices = indices;
        std::vector<T> reduced_values = values;

        for (std::size_t src_pos = 1; src_pos < nvals; src_pos += 1) {
            if (reduced_indices[dst_pos] != reduced_indices[src_pos]) {
                dst_pos += 1;
                reduced_indices[dst_pos] = reduced_indices[src_pos];
                if constexpr (type_has_values<T>()) reduced_values[dst_pos] = reduced_values[src_pos];
            } else {
                reduced_values[dst_pos] = reduceOp.invoke_host(reduced_values[dst_pos], reduced_values[src_pos]);
            }
        }

        if (!indices.empty())
            dst_pos += 1;

        reduced_indices.resize(dst_pos);
        reduced_values.resize(type_has_values<T>() ? dst_pos : 0);

        out_indices = std::move(reduced_indices);
        out_values = std::move(reduced_values);
    }

    template<typename T, typename ReduceOp>
    inline void reduce(const std::vector<Index> &rows,
                       const std::vector<Index> &cols,
                       const std::vector<T> &values,
                       const ReduceOp &reduceOp,
                       std::vector<Index> &out_rows,
                       std::vector<Index> &out_cols,
                       std::vector<T> &out_values) {
        std::size_t dst_pos = 0;
        std::size_t nvals = rows.size();
        std::vector<Index> reduced_rows = rows;
        std::vector<Index> reduced_cols = cols;
        std::vector<T> reduced_values = values;

        auto eq = [](Index a1, Index a2, Index b1, Index b2) { return a1 == b1 && a2 == b2; };

        for (std::size_t src_pos = 1; src_pos < nvals; src_pos += 1) {
            if (!eq(reduced_rows[dst_pos], reduced_cols[dst_pos], reduced_rows[src_pos], reduced_cols[src_pos])) {
                dst_pos += 1;
                reduced_rows[dst_pos] = reduced_rows[src_pos];
                reduced_cols[dst_pos] = reduced_cols[src_pos];
                if constexpr (type_has_values<T>()) reduced_values[dst_pos] = reduced_values[src_pos];
            } else {
                reduced_values[dst_pos] = reduceOp.invoke_host(reduced_values[dst_pos], reduced_values[src_pos]);
            }
        }

        if (!rows.empty())
            dst_pos += 1;

        reduced_rows.resize(dst_pos);
        reduced_cols.resize(dst_pos);
        reduced_values.resize(type_has_values<T>() ? dst_pos : 0);

        out_rows = std::move(reduced_rows);
        out_cols = std::move(reduced_cols);
        out_values = std::move(reduced_values);
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_SHARED_REDUCE_HPP
