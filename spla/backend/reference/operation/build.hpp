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

#include <spla/binary_op.hpp>
#include <spla/descriptor.hpp>
#include <spla/io/log.hpp>
#include <spla/types.hpp>

#include <spla/detail/pair_utils.hpp>

#include <spla/backend/shared/params.hpp>
#include <spla/backend/shared/reduce.hpp>
#include <spla/backend/shared/rows_ptr.hpp>
#include <spla/backend/shared/sort.hpp>

#include <spla/backend/reference/storage/matrix_csr.hpp>
#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

namespace spla::backend {

    /**
     * @addtogroup reference
     * @{
     */

    template<typename T, typename ReduceOp>
    inline void build(detail::Ref<VectorCoo<T>> &w,
                      const ReduceOp &reduceOp,
                      const std::vector<Index> &rows,
                      const std::vector<T> &values,
                      const Descriptor &descriptor,
                      const BuildParams &buildParams,
                      const DispatchParams &dispatchParams) {
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
            SPLA_LOG_INFO("sort values id=" << dispatchParams.id);
            sort(vectorRows, vectorValues);
        }

        if (!descriptor.no_duplicates() && nvals > 1) {
            SPLA_LOG_INFO("reduce duplicates id=" << dispatchParams.id);
            std::vector<Index> reducedRows;
            std::vector<T> reducedValues;

            if constexpr (null_op<ReduceOp>())
                reduce(vectorRows, vectorValues, binary_op::first<T>(), reducedRows, reducedValues);
            else
                reduce(vectorRows, vectorValues, reduceOp, reducedRows, reducedValues);

            nvals = reducedRows.size();
            std::swap(reducedRows, vectorRows);
            std::swap(reducedValues, vectorValues);
        }

        w.acquire(new VectorCoo<T>(buildParams.size, nvals, std::move(vectorRows), std::move(vectorValues)));
    }

    template<typename T, typename ReduceOp>
    inline void build(detail::Ref<MatrixCsr<T>> &w,
                      const ReduceOp &reduceOp,
                      const std::vector<Index> &rows,
                      const std::vector<Index> &cols,
                      const std::vector<T> &values,
                      const Descriptor &descriptor,
                      const BuildParamsMat &buildParams,
                      const DispatchParams &dispatchParams) {
        std::size_t nvals = 0;

        for (std::size_t k = 0; k < rows.size(); k++) {
            assert(rows[k] < buildParams.boundsRow);
            assert(cols[k] < buildParams.boundsCol);
            if (buildParams.firstIndexRow <= rows[k] && rows[k] <= buildParams.firstIndexRow + buildParams.sizeRow &&
                buildParams.firstIndexCol <= cols[k] && cols[k] <= buildParams.firstIndexCol + buildParams.sizeCol)
                nvals += 1;
        }

        if (nvals == 0) {
            w.reset();
            return;
        }

        std::vector<Index> vectorRows;
        std::vector<Index> vectorCols;
        std::vector<T> vectorValues;
        vectorRows.reserve(nvals);
        vectorCols.reserve(nvals);
        vectorValues.reserve(type_has_values<T>() ? nvals : 0);

        for (std::size_t k = 0; k < rows.size(); k++) {
            auto i = rows[k];
            auto j = cols[k];

            if (buildParams.firstIndexRow <= i && i <= buildParams.firstIndexRow + buildParams.sizeRow &&
                buildParams.firstIndexCol <= j && j <= buildParams.firstIndexCol + buildParams.sizeCol) {
                vectorRows.push_back(i - buildParams.firstIndexRow);
                vectorCols.push_back(j - buildParams.firstIndexCol);
                if constexpr (type_has_values<T>()) vectorValues.push_back(values[k]);
            }
        }

        assert(vectorRows.size() == nvals);
        assert(vectorCols.size() == nvals);
        assert(vectorValues.empty() || vectorValues.size() == nvals);

        if (!descriptor.sorted() && nvals > 1) {
            SPLA_LOG_INFO("sort values id=" << dispatchParams.id);
            sort(vectorRows, vectorCols, vectorValues);
        }

        if (!descriptor.no_duplicates() && nvals > 1) {
            SPLA_LOG_INFO("reduce duplicates id=" << dispatchParams.id);
            std::vector<Index> reducedRows;
            std::vector<Index> reducedCols;
            std::vector<T> reducedValues;

            if constexpr (null_op<ReduceOp>())
                reduce(vectorRows, vectorCols, vectorValues, binary_op::first<T>(),
                       reducedRows, reducedCols, reducedValues);
            else
                reduce(vectorRows, vectorCols, vectorValues, reduceOp,
                       reducedRows, reducedCols, reducedValues);

            nvals = reducedRows.size();
            std::swap(reducedRows, vectorRows);
            std::swap(reducedCols, vectorCols);
            std::swap(reducedValues, vectorValues);
        }

        std::vector<Index> rows_ptr;
        rows_offsets(buildParams.sizeRow, vectorRows, rows_ptr);

        assert(rows_ptr.back() == nvals);

        w.acquire(new MatrixCsr<T>(buildParams.sizeRow, buildParams.sizeCol, nvals,
                                   std::move(rows_ptr), std::move(vectorCols), std::move(vectorValues)));
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_BUILD_HPP
