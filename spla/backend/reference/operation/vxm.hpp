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

#ifndef SPLA_REFERENCE_VXM_HPP
#define SPLA_REFERENCE_VXM_HPP

#include <algorithm>
#include <vector>

#include <spla/binary_op.hpp>
#include <spla/descriptor.hpp>
#include <spla/io/log.hpp>
#include <spla/types.hpp>

#include <spla/backend/shared/ewiseadd.hpp>
#include <spla/backend/shared/params.hpp>

#include <spla/backend/reference/storage/matrix_csr.hpp>
#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

namespace spla::backend {

    /**
     * @addtogroup spla
     * @{
     */

    template<typename T,
             typename M,
             typename U,
             typename V,
             typename AccumOp,
             typename MultiplyOp,
             typename ReduceOp>
    inline void vxm(detail::Ref<VectorCoo<T>> &w,
                    const detail::Ref<VectorDense<T>> &mask,
                    const AccumOp &accumOp,
                    const MultiplyOp &multiplyOp,
                    const ReduceOp &reduceOp,
                    const detail::Ref<VectorCoo<T>> &v,
                    const detail::Ref<MatrixCsr<T>> &m,
                    const Descriptor &descriptor,
                    const DispatchParams &dispatchParams) {
        assert(null_op<AccumOp>());

        auto nrows = m->nrows();
        auto ncols = m->ncols();

        assert(nrows == v->nrows());

        auto use_scmp = descriptor.scmp();

        std::vector<Index> out_mask(ncols, Index{0});
        std::vector<T> out_values(type_has_values<T>() ? ncols : 0);

        const auto &mask_mask = mask->mask();

        const auto &v_rows = v->rows();
        const auto &v_values = v->values();

        const auto &m_rows_ptr = m->rows_ptr();
        const auto &m_cols_index = m->cols_index();
        const auto &m_values = m->values();

        std::size_t count = 0;

        for (std::size_t l = 0; l < v_rows.size(); l++) {
            auto i = v_rows[l];

            for (Index k = m_rows_ptr[i]; k < m_rows_ptr[i + 1]; k++) {
                auto j = m_cols_index[k];

                if (mask_mask[j] != use_scmp) {
                    if (!out_mask[j]) {
                        out_mask[j] = 1;
                        count += 1;

                        if constexpr (type_has_values<T>())
                            out_values[j] = multiplyOp.invoke_host(v_values[l], m_values[k]);

                        continue;
                    }

                    if constexpr (type_has_values<T>())
                        out_values[j] = reduceOp.invoke_host(out_values[j],
                                                             multiplyOp.invoke_host(v_values[l], m_values[k]));
                }
            }
        }

        std::vector<Index> compact_rows;
        std::vector<T> compact_values;

        compact_rows.reserve(count);
        compact_rows.reserve(type_has_values<T>() ? count : 0);

        for (std::size_t j = 0; j < ncols; j++) {
            if (out_mask[j]) {
                compact_rows.push_back(j);
                if constexpr (type_has_values<T>())
                    compact_values.push_back(out_values[j]);
            }
        }

        assert(compact_rows.size() == count);
        w.acquire(new VectorCoo<T>(ncols, count, std::move(compact_rows), std::move(compact_values)));
    }

    template<typename T,
             typename M,
             typename U,
             typename V,
             typename AccumOp,
             typename MultiplyOp,
             typename ReduceOp>
    inline void vxm(detail::Ref<VectorDense<T>> &w,
                    const detail::Ref<VectorDense<T>> &mask,
                    const AccumOp &accumOp,
                    const MultiplyOp &multiplyOp,
                    const ReduceOp &reduceOp,
                    const detail::Ref<VectorDense<T>> &v,
                    const detail::Ref<MatrixCsr<T>> &m,
                    const Descriptor &descriptor,
                    const DispatchParams &dispatchParams) {
        assert(null_op<AccumOp>());

        auto nrows = m->nrows();
        auto ncols = m->ncols();

        assert(nrows == v->nrows());

        auto use_scmp = descriptor.scmp();

        std::vector<Index> out_mask(ncols, Index{0});
        std::vector<T> out_values(type_has_values<T>() ? ncols : 0);

        const auto &mask_mask = mask->mask();

        const auto &v_mask = v->mask();
        const auto &v_values = v->values();

        const auto &m_rows_ptr = m->rows_ptr();
        const auto &m_cols_index = m->cols_index();
        const auto &m_values = m->values();

        std::size_t count = 0;

        for (std::size_t i = 0; i < nrows; i++) {
            if (!v_mask[i])
                continue;

            for (Index k = m_rows_ptr[i]; k < m_rows_ptr[i + 1]; k++) {
                auto j = m_cols_index[k];

                if (mask_mask[j] != use_scmp) {
                    if (!out_mask[j]) {
                        out_mask[j] = 1;
                        count += 1;

                        if constexpr (type_has_values<T>())
                            out_values[j] = multiplyOp.invoke_host(v_values[i], m_values[k]);

                        continue;
                    }

                    if constexpr (type_has_values<T>())
                        out_values[j] = reduceOp.invoke_host(out_values[j],
                                                             multiplyOp.invoke_host(v_values[i], m_values[k]));
                }
            }
        }

        w.acquire(new VectorDense<T>(ncols, count, std::move(out_mask), std::move(out_values)));
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_VXM_HPP
