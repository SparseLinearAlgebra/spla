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

#ifndef SPLA_SHARED_EWISEADD_HPP
#define SPLA_SHARED_EWISEADD_HPP

#include <vector>

#include <spla/types.hpp>

namespace spla::backend {

    /**
     * @addtogroup shared
     * @{
     */

    template<typename T, typename ReduceOp>
    inline void ewiseadd_coo(const std::vector<Index> &indices1,
                             const std::vector<T> &values1,
                             const std::vector<Index> &indices2,
                             const std::vector<T> &values2,
                             const ReduceOp &reduceOp,
                             std::vector<Index> &out_indices,
                             std::vector<T> &out_values) {
        auto count1 = indices1.size();
        auto count2 = indices2.size();

        std::vector<Index> indices;
        std::vector<T> values;

        indices.reserve(count1 + count2);
        values.reserve(type_has_values<T>() ? count1 + count2 : 0);

        std::size_t i = 0;
        std::size_t j = 0;

        while (i < count1 && j < count2) {
            if (indices1[i] < indices2[j]) {
                indices.push_back(indices1[i]);
                if constexpr (type_has_values<T>()) values.push_back(values1[i]);
                i += 1;
            } else if (indices2[j] < indices1[i]) {
                indices.push_back(indices2[j]);
                if constexpr (type_has_values<T>()) values.push_back(values2[j]);
                j += 1;
            } else {
                indices.push_back(indices1[i]);
                if constexpr (type_has_values<T>()) values.push_back(reduceOp.invoke_host(values1[i], values2[j]));
                i += 1;
                j += 1;
            }
        }

        while (i < count1) {
            indices.push_back(indices1[i]);
            if constexpr (type_has_values<T>()) values.push_back(values1[i]);
            i += 1;
        }

        while (j < count2) {
            indices.push_back(indices2[j]);
            if constexpr (type_has_values<T>()) values.push_back(values2[j]);
            j += 1;
        }

        out_indices = std::move(indices);
        out_values = std::move(values);
    }

    template<typename T, typename ReduceOp>
    inline void ewiseadd_dense(const std::vector<Index> &mask1,
                               const std::vector<T> &values1,
                               const std::vector<Index> &mask2,
                               const std::vector<T> &values2,
                               const ReduceOp &reduceOp,
                               std::vector<Index> &out_mask,
                               std::vector<T> &out_values,
                               std::size_t &count) {
        auto size = mask1.size();

        std::vector<Index> mask;
        std::vector<T> values;

        mask.resize(size, Index{0});
        values.resize(type_has_values<T>() ? size : 0);

        count = 0;

        for (std::size_t i = 0; i < size; i++) {
            auto m1 = mask1[i];
            auto m2 = mask2[i];

            if (m1 || m2) {
                mask[i] = 1;
                count += 1;

                if constexpr (type_has_values<T>()) {
                    if (m1 && m2) {
                        values[i] = reduceOp.invoke_host(values1[i], values2[i]);
                    } else if (m1) {
                        values[i] = values1[i];
                    } else {
                        values[i] = values2[i];
                    }
                }
            }
        }

        std::swap(out_mask, mask);
        std::swap(out_values, values);
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_SHARED_EWISEADD_HPP
