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

#ifndef SPLA_REFERENCE_READ_HPP
#define SPLA_REFERENCE_READ_HPP

#include <algorithm>
#include <vector>

#include <spla/descriptor.hpp>
#include <spla/io/log.hpp>
#include <spla/types.hpp>

#include <spla/backend/shared/params.hpp>

#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

namespace spla::backend {

    /**
     * @addtogroup reference
     * @{
     */

    template<typename T>
    inline void read(const detail::Ref<VectorCoo<T>> &w,
                     std::vector<Index> &rows,
                     std::vector<T> &values,
                     const Descriptor &,
                     const ReadParams &readParams,
                     const DispatchParams &dispatchParams) {
        auto nvals = w->nvals();
        const auto &w_rows = w->rows();
        const auto &w_values = w->values();

        for (std::size_t i = 0; i < nvals; i++) {
            rows[readParams.offset + i] = w_rows[i] + readParams.firstIndex;

            if (type_has_values<T>()) {
                values[readParams.offset + i] = w_values[i];
            }
        }

        SPLA_LOG_INFO("read block id=" << dispatchParams.id);
    }

    template<typename T>
    inline void read(const detail::Ref<VectorDense<T>> &w,
                     std::vector<Index> &rows,
                     std::vector<T> &values,
                     const Descriptor &,
                     const ReadParams &readParams,
                     const DispatchParams &dispatchParams) {
        auto nrows = w->nrows();
        const auto &w_rows = w->mask();
        const auto &w_values = w->values();

        std::size_t idx = 0;

        for (std::size_t i = 0; i < nrows; i++) {
            if (w_rows[i]) {
                rows[readParams.offset + idx] = w_rows[i] + readParams.firstIndex;

                if (type_has_values<T>()) {
                    values[readParams.offset + idx] = w_values[i];
                }

                idx += 1;
            }
        }

        assert(idx == w->nvals());

        SPLA_LOG_INFO("read block id=" << dispatchParams.id);
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_READ_HPP
