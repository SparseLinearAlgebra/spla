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

#ifndef SPLA_REFERENCE_VECTOR_STORAGE_HPP
#define SPLA_REFERENCE_VECTOR_STORAGE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <numeric>
#include <shared_mutex>
#include <unordered_map>

#include <spla/library.hpp>
#include <spla/types.hpp>

#include <spla/backend/reference/backend.hpp>
#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>

#include <spla/detail/storage_utils.hpp>
#include <spla/storage/storage_schema.hpp>

namespace spla::backend {

    /**
     * @addtogroup reference
     * @{
     */

    /**
     * @class VectorStorage
     * @brief Reference host backend blocked vector storage
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class VectorStorage final : public detail::RefCnt {
    public:
        typedef std::vector<detail::Ref<detail::VectorBlock<T>>> Blocks;
        typedef std::vector<detail::Ref<VectorCoo<T>>> BlocksSparse;
        typedef std::vector<detail::Ref<VectorDense<T>>> BlocksDense;

        explicit VectorStorage(std::size_t nrows)
            : m_nrows(nrows) {
            m_block_size = detail::block_size(nrows, library().block_factor(), device_count());
            m_block_count_rows = detail::block_count(nrows, m_block_size);
            m_sparse.resize(m_block_count_rows);
            m_dense.resize(m_block_count_rows);
        }

        ~VectorStorage() override = default;

        [[nodiscard]] std::size_t nrows() const { return m_nrows; }
        [[nodiscard]] std::size_t block_size() const { return m_block_size; }
        [[nodiscard]] std::size_t block_count_rows() const { return m_block_count_rows; }

        void build(BlocksSparse sparse) {
            assert(sparse.size() == block_count_rows());
            m_schema = StorageSchema::Sparse;
            m_sparse = std::move(sparse);
            m_nvals = 0;

            for (const auto &b : m_sparse)
                if (b.is_not_null()) m_nvals += b->nvals();
        }

        void build(BlocksDense sparse) {
            assert(sparse.size() == block_count_rows());
            m_schema = StorageSchema::Dense;
            m_dense = std::move(sparse);
            m_nvals = 0;

            for (const auto &b : m_dense)
                if (b.is_not_null()) m_nvals += b->nvals();
        }

        [[nodiscard]] std::size_t nvals() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_nvals;
        }

        [[nodiscard]] StorageSchema storage_schema() {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_schema;
        }

        [[nodiscard]] detail::Ref<VectorCoo<T>> block_sparse(std::size_t i) {
            assert(i < block_count_rows());

            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);

            if (m_schema != StorageSchema::Sparse)
                throw std::runtime_error("invalid storage schema");

            return m_sparse[i];
        }

        [[nodiscard]] detail::Ref<VectorDense<T>> block_dense(std::size_t i) {
            assert(i < block_count_rows());

            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);

            if (m_schema != StorageSchema::Dense)
                throw std::runtime_error("invalid storage schema");

            return m_dense[i];
        }

        [[nodiscard]] BlocksSparse blocks_sparse() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);

            if (m_schema != StorageSchema::Sparse)
                throw std::runtime_error("invalid storage schema");

            return m_sparse;
        }

        [[nodiscard]] BlocksDense blocks_dense() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);

            if (m_schema != StorageSchema::Dense)
                throw std::runtime_error("invalid storage schema");

            return m_dense;
        }

        [[nodiscard]] Blocks blocks() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            Blocks blocks_list(block_count_rows());

            if (m_schema == StorageSchema::Sparse)
                std::copy(m_sparse.begin(), m_sparse.end(), blocks_list.begin());
            if (m_schema == StorageSchema::Dense)
                std::copy(m_sparse.begin(), m_sparse.end(), blocks_list.begin());

            return blocks_list;
        }

        void lock_write() {}
        void unlock_write() {}

        void lock_read() {}
        void unlock_read() {}

    private:
        std::size_t m_nrows;
        std::size_t m_nvals = 0;
        std::size_t m_block_size = 0;
        std::size_t m_block_count_rows = 0;

        StorageSchema m_schema = StorageSchema::Sparse;

        BlocksSparse m_sparse;
        BlocksDense m_dense;

        mutable std::shared_mutex m_mutex;
    };

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_VECTOR_STORAGE_HPP
