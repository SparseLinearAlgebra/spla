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

#ifndef SPLA_REFERENCE_EWISEADD_HPP
#define SPLA_REFERENCE_EWISEADD_HPP

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

    template<typename T, typename ReduceOp>
    inline void ewiseadd(detail::Ref<VectorCoo<T>> &w,
                         const ReduceOp &reduceOp,
                         const detail::Ref<VectorCoo<T>> &a,
                         const detail::Ref<VectorCoo<T>> &b,
                         const Descriptor &desc,
                         const DispatchParams &dispatchParams) {
        std::vector<Index> rows;
        std::vector<T> values;

        const auto &a_rows = a->rows();
        const auto &a_values = a->values();

        const auto &b_rows = b->rows();
        const auto &b_values = b->values();

        ewiseadd_coo(a_rows, a_values, b_rows, b_values, reduceOp, rows, values);
        w.acquire(new VectorCoo<T>(a->nrows(), rows.size(), std::move(rows), std::move(values)));
    }

    template<typename T, typename ReduceOp>
    inline void ewiseadd(detail::Ref<VectorDense<T>> &w,
                         const ReduceOp &reduceOp,
                         const detail::Ref<VectorDense<T>> &a,
                         const detail::Ref<VectorDense<T>> &b,
                         const Descriptor &desc,
                         const DispatchParams &dispatchParams) {
        std::vector<Index> mask;
        std::vector<T> values;

        const auto &a_mask = a->mask();
        const auto &a_values = a->values();

        const auto &b_mask = b->mask();
        const auto &b_values = b->values();

        std::size_t nvals;

        ewiseadd_dense(a_mask, a_values, b_mask, b_values, reduceOp, mask, values, nvals);
        w.acquire(new VectorDense<T>(a->nrows(), nvals, std::move(mask), std::move(values)));
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_EWISEADD_HPP
