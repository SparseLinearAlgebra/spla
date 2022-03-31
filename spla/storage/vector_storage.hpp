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

#include <spla/backend.hpp>
#include <spla/library.hpp>
#include <spla/types.hpp>

#include <spla/detail/resource.hpp>
#include <spla/detail/storage_utils.hpp>
#include <spla/detail/vector_block.hpp>
#include <spla/storage/storage_schema.hpp>

namespace spla::detail {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class VectorStorage
     * @brief Backend blocked vector storage
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class VectorStorage final : public RefCnt, public Resource {
    public:
        typedef Ref<VectorBlock<T>> Block;
        typedef std::vector<Block> Blocks;

        explicit VectorStorage(std::size_t nrows)
            : m_nrows(nrows) {
            assert(nrows > 0);
            m_block_size = detail::block_size(nrows, library().block_factor(), backend::device_count());
            m_block_count_rows = detail::block_count(nrows, m_block_size);

            // Allocate empty blocks for default schema
            build(m_schema, Blocks(m_block_count_rows));
        }

        ~VectorStorage() override = default;

        void build(VectorSchema schema, Blocks blocks) {
            assert(blocks.size() == block_count_rows());

            std::lock_guard<std::shared_mutex> lockGuard(m_mutex);

            m_schema = schema;
            m_nvals = 0;
            m_blocks[schema] = std::move(blocks);

            for (const auto &b : m_blocks[schema])
                if (b.is_not_null()) m_nvals += b->nvals();
        }

        void set_block(VectorSchema schema, std::size_t i, Block block) {
            std::lock_guard<std::shared_mutex> lockGuard(m_mutex);

            assert(schema == m_schema);
            assert(i < block_count_rows());

            if (schema != m_schema)
                throw std::runtime_error("set block only allowed for active storage schema");

            auto &prev = m_blocks[m_schema][i];

            m_nvals -= prev.is_not_null() ? prev->nvals() : 0;
            m_nvals += block.is_not_null() ? block->nvals() : 0;
            prev = std::move(block);
        }

        void clear() {
            build(VectorSchema::Sparse, Blocks(m_block_count_rows));
        }

        [[nodiscard]] std::size_t nrows() const { return m_nrows; }
        [[nodiscard]] std::size_t block_size() const { return m_block_size; }
        [[nodiscard]] std::size_t block_count_rows() const { return m_block_count_rows; }

        [[nodiscard]] std::size_t nvals() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_nvals;
        }

        [[nodiscard]] VectorSchema schema() {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_schema;
        }

        [[nodiscard]] Ref<VectorBlock<T>> block(std::size_t i) {
            assert(i < block_count_rows());
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_blocks[m_schema][i];
        }

        [[nodiscard]] Blocks blocks() const {
            std::shared_lock<std::shared_mutex> lockGuard(m_mutex);
            return m_blocks[m_schema];
        }

    private:
        /** Vector dim */
        std::size_t m_nrows;
        /** Total number of non-zero values */
        std::size_t m_nvals = 0;
        /** Size of block (except the edge case) */
        std::size_t m_block_size = 0;
        /** Number of blocks after decomposition */
        std::size_t m_block_count_rows = 0;

        /** Active storage schema of the vector (used to query blocks) */
        VectorSchema m_schema = VectorSchema::Sparse;
        /** Blocks of the vector per schema (some may be cached) */
        mutable std::unordered_map<VectorSchema, Blocks> m_blocks;

        mutable std::shared_mutex m_mutex;
    };

    /**
     * @}
     */

}// namespace spla::detail

#endif//SPLA_REFERENCE_VECTOR_STORAGE_HPP
