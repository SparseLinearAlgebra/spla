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

#ifndef SPLA_REFERENCE_ASSIGN_HPP
#define SPLA_REFERENCE_ASSIGN_HPP

#include <algorithm>
#include <vector>

#include <spla/binary_op.hpp>
#include <spla/descriptor.hpp>
#include <spla/io/log.hpp>
#include <spla/types.hpp>

#include <spla/backend/shared/ewiseadd.hpp>
#include <spla/backend/shared/params.hpp>

#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

namespace spla::backend {

    /**
     * @addtogroup reference
     * @{
     */

    template<typename T, typename M, typename AccumOp>
    inline void assign(detail::Ref<VectorCoo<T>> &w,
                       const detail::Ref<VectorCoo<M>> &m,
                       const AccumOp &accumOp,
                       const T &value,
                       const Descriptor &,
                       const AssignParams &,
                       const DispatchParams &) {
        auto nvals = m->nvals();
        std::vector<Index> indices(m->rows().begin(), m->rows().end());
        std::vector<T> values(nvals, value);

        // If has source w, accum result
        if (w.is_not_null()) {
            std::vector<Index> result_indices;
            std::vector<T> result_values;

            if constexpr (null_op<AccumOp>())
                ewiseadd(w->rows(), w->values(), indices, values, binary_op::second<T>(), result_indices, result_values);
            else
                ewiseadd(w->rows(), w->values(), result_indices, result_values, accumOp, result_indices, result_values);

            nvals = result_indices.size();
            std::swap(result_indices, indices);
            std::swap(result_values, values);
        }

        w.acquire(new VectorCoo<T>(m->nrows(), nvals, std::move(indices), std::move(values)));
    }

    template<typename T, typename M, typename AccumOp>
    inline void assign(detail::Ref<VectorDense<T>> &w,
                       const detail::Ref<VectorCoo<M>> &m,
                       const AccumOp &accumOp,
                       const T &value,
                       const Descriptor &,
                       const AssignParams &,
                       const DispatchParams &) {
        const auto nrows = m->nrows();
        const auto &m_rows = m->rows();
        const auto &m_values = m->values();

        // If no block, allocate empty dense vector
        if (w.is_null())
            w.acquire(new VectorDense<T>(nrows, 0,
                                         std::vector<Index>(nrows, 0),
                                         std::vector<T>(type_has_values<T>() ? nrows : 0)));

        auto &w_mask = w->mask();
        auto &w_values = w->values();

        // Fill values inside dense block using
        for (std::size_t k = 0; k < m->nvals(); k++) {
            auto i = m_rows[k];
            if (!w_mask[i]) {
                w_mask[i] = 1;
                w->nvals() += 1;
                if constexpr (type_has_values<T>()) w_values[i] = m_values[k];
            } else if constexpr (type_has_values<T>()) {
                if constexpr (null_op<AccumOp>())
                    w_values[i] = m_values[k];
                else
                    w_values[i] = accumOp.invoke_host(w_values[i], m_values[k]);
            }
        }
    }

    template<typename T, typename M, typename AccumOp>
    inline void assign(detail::Ref<VectorDense<T>> &w,
                       const detail::Ref<VectorDense<M>> &m,
                       const AccumOp &accumOp,
                       const T &value,
                       const Descriptor &,
                       const AssignParams &,
                       const DispatchParams &) {
        const auto nrows = m->nrows();
        const auto &m_mask = m->mask();
        const auto &m_values = m->values();

        // If no block, allocate empty dense vector
        if (w.is_null())
            w.acquire(new VectorDense<T>(nrows, 0,
                                         std::vector<Index>(nrows, 0),
                                         std::vector<T>(type_has_values<T>() ? nrows : 0)));

        auto &w_mask = w->mask();
        auto &w_values = w->values();

        for (std::size_t i = 0; i < nrows; i++) {
            if (m_mask[i]) {
                if (!w_mask[i]) {
                    w_mask[i] = 1;
                    w->nvals() += 1;
                    if constexpr (type_has_values<T>()) w_values[i] = m_values[i];
                } else if constexpr (type_has_values<T>()) {
                    if constexpr (null_op<AccumOp>()) {
                        w_values[i] = m_values[i];
                    } else {
                        w_values[i] = accumOp.invoke_host(w_values[i], m_values[i]);
                    }
                }
            }
        }
    }


    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_ASSIGN_HPP
