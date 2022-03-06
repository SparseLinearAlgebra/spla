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

#ifndef SPLA_REFERENCE_BUILD_HPP
#define SPLA_REFERENCE_BUILD_HPP

#include <algorithm>
#include <vector>

#include <spla/descriptor.hpp>
#include <spla/io/log.hpp>
#include <spla/types.hpp>

#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

namespace spla::backend {

    struct BuildParams {
        std::size_t firstIndex;
        std::size_t size;
        std::size_t bounds;
    };

    template<typename T, typename ReduceOp>
    inline void build(detail::Ref<VectorCoo<T>> &w,
                      ReduceOp reduceOp,
                      const std::vector<Index> &rows,
                      const std::vector<T> &values,
                      const Descriptor &descriptor,
                      const BuildParams &buildParams,
                      std::size_t id) {
        std::size_t nvals = 0;

        for (auto i : rows) {
            assert(i < buildParams.bounds);
            if (buildParams.firstIndex <= i && i < buildParams.firstIndex + buildParams.size)
                nvals += 1;
        }

        if (nvals == 0) {
            w.reset();
            return;
        }

        std::vector<Index> vectorRows;
        std::vector<T> vectorValues;
        vectorRows.reserve(nvals);
        vectorValues.reserve(type_has_values<T>() ? nvals : 0);

        for (std::size_t i = 0; i < rows.size(); i++) {
            auto rowId = rows[i];
            if (buildParams.firstIndex <= rowId && rowId < buildParams.firstIndex + buildParams.size) {
                vectorRows.push_back(rowId - buildParams.firstIndex);
                if (type_has_values<T>()) {
                    vectorValues.push_back(values[i]);
                }
            }
        }

        assert(vectorRows.size() == nvals);
        assert(vectorValues.empty() || vectorValues.size() == nvals);

        if (!descriptor.sorted() && nvals > 1) {
            SPLA_LOG_INFO("sort values id=" << id);

            if (type_has_values<T>()) {
                // Pack indices and values, sort and unpack
                using tuple = std::pair<Index, T>;
                std::vector<tuple> to_sort(nvals);

                for (std::size_t i = 0; i < nvals; i++)
                    to_sort[i] = {vectorRows[i], vectorValues[i]};

                std::sort(to_sort.begin(), to_sort.end(),
                          [](const auto &a, const auto &b) { return a.first < b.first; });

                for (std::size_t i = 0; i < nvals; i++) {
                    vectorRows[i] = to_sort[i].first;
                    vectorValues[i] = to_sort[i].second;
                }
            } else {
                // Sort only indices
                std::sort(vectorRows.begin(), vectorRows.end(), std::less<>());
            }
        }

        if (!descriptor.no_duplicates() && nvals > 1) {
            SPLA_LOG_INFO("reduce duplicates id=" << id);
            std::size_t pos = 0;
            for (std::size_t i = 1; i < nvals; i++) {
                if (vectorRows[pos] != vectorRows[i]) {
                    // Offset pos and copy index
                    pos += 1;
                    vectorRows[pos] = vectorRows[i];
                    if (type_has_values<T>()) {
                        vectorValues[pos] = vectorValues[i];
                    }
                } else if (!null_op<ReduceOp>()) {
                    // If has reduce op, can reduce duplicated values (store at the same prev pos)
                    vectorValues[pos] = reduceOp.invoke_host(vectorValues[pos], vectorValues[i]);
                }
            }
            nvals = pos + 1;
            vectorRows.resize(nvals);
            vectorValues.resize(type_has_values<T>() ? nvals : 0);
        }

        w.acquire(new VectorCoo<T>(buildParams.size, nvals, std::move(vectorRows), std::move(vectorValues)));
    }

}// namespace spla::backend

#endif//SPLA_REFERENCE_BUILD_HPP
