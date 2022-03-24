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

#ifndef SPLA_MATRIX_STORAGE_HPP
#define SPLA_MATRIX_STORAGE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <numeric>
#include <shared_mutex>
#include <unordered_map>

#include <spla/backend.hpp>
#include <spla/library.hpp>
#include <spla/types.hpp>

#include <spla/detail/grid.hpp>
#include <spla/detail/matrix_block.hpp>
#include <spla/detail/resource.hpp>
#include <spla/detail/storage_utils.hpp>
#include <spla/storage/storage_schema.hpp>

namespace spla::detail {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class MatrixStorage
     * @brief Backend blocked matrix storage
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class MatrixStorage final : public RefCnt, Resource {
    public:
        typedef Grid<Ref<MatrixBlock<T>>> Blocks;
        typedef typename Grid<Ref<MatrixBlock<T>>>::Index Index;

        MatrixStorage(std::size_t nrows, std::size_t ncols)
            : m_dim(nrows, ncols) {
            assert(nrows > 0);
            assert(ncols > 0);
            m_block_size = detail::block_size(nrows, ncols, library().block_factor(), backend::device_count());
            m_block_count = detail::block_count(nrows, ncols, m_block_size);
        }

        void build(MatrixSchema schema, Blocks blocks) {
            assert(blocks.dim().first == m_dim.first);
            assert(blocks.dim().second == m_dim.second);

            std::lock_guard<std::shared_mutex> lockGuard(m_mutex);

            m_schema = schema;
            m_nvals = 0;
            m_blocks[schema] = std::move(blocks);

            for (const auto &b : m_blocks[schema].elements())
                if (b.second.is_not_null()) m_nvals += b.second->nvals();
        }

        [[nodiscard]] std::size_t nrows() const { return m_dim.first; }
        [[nodiscard]] std::size_t ncols() const { return m_dim.second; }
        [[nodiscard]] std::size_t block_size() const { return m_block_size; }
        [[nodiscard]] std::pair<std::size_t, std::size_t> dim() const { return m_dim; }
        [[nodiscard]] std::pair<std::size_t, std::size_t> block_count() const { return m_block_count; }

        [[nodiscard]] std::size_t nvals() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_nvals;
        }

        [[nodiscard]] MatrixSchema schema() {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_schema;
        }

        [[nodiscard]] Ref<MatrixBlock<T>> block(const Index &idx) {
            assert(idx.first < block_count().first);
            assert(idx.second < block_count().second);
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_blocks[m_schema][idx];
        }

        [[nodiscard]] Blocks blocks() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_blocks[m_schema];
        }

    private:
        /** Matrix dim */
        std::pair<std::size_t, std::size_t> m_dim;
        /** Total number of non-zero values */
        std::size_t m_nvals = 0;
        /** Size of block (except the edge case) */
        std::size_t m_block_size = 0;
        /** Number of blocks after decomposition */
        std::pair<std::size_t, std::size_t> m_block_count{0, 0};

        /** Active storage schema of the matrix (used to query blocks) */
        MatrixSchema m_schema = MatrixSchema::Csr;
        /** Blocks of the matrix per schema (some may be cached) */
        mutable std::unordered_map<MatrixSchema, Blocks> m_blocks;

        mutable std::shared_mutex m_mutex;
    };

    /**
     * @}
     */

}// namespace spla::detail

#endif//SPLA_MATRIX_STORAGE_HPP
