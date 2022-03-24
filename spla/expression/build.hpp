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

#ifndef SPLA_BUILD_HPP
#define SPLA_BUILD_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include <spla/backend.hpp>
#include <spla/descriptor.hpp>
#include <spla/expression_node.hpp>
#include <spla/types.hpp>
#include <spla/vector.hpp>

#include <spla/detail/backend_utils.hpp>
#include <spla/detail/storage_utils.hpp>

namespace spla::expression {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T, typename ReduceOp>
    class BuildVector final : public ExpressionNode {
    public:
        BuildVector(Vector<T> vector, ReduceOp reduceOp, std::vector<Index> rows, std::vector<T> values, const Descriptor &desc, std::size_t id, Expression &expression)
            : ExpressionNode(desc, id, expression), m_vector(std::move(vector)), m_reduceOp(std::move(reduceOp)), m_rows(std::move(rows)), m_values(std::move(values)) {
            assert(!m_rows.empty());
            assert(!m_values.empty() || !type_has_values<T>());
        }

        ~BuildVector() override = default;

        std::string type() const override {
            return "build-vector";
        }

    private:
        void prepare() override {
            m_vector.storage()->lock_write();
        }

        void finalize() override {
            m_vector.storage()->unlock_write();
        }

        void execute(detail::SubtaskBuilder &builder) override {
            auto storage = m_vector.storage();
            auto blocks = detail::make_blocks(storage);

            auto build = builder.emplace("build-storage", [blocks, storage]() {
                storage->build(StorageSchema::Sparse, std::move(*blocks));
            });

            for (std::size_t i = 0; i < storage->block_count_rows(); i++) {
                builder.emplace("build-block", [=]() {
                           auto nrows = storage->nrows();
                           auto blockSize = storage->block_size();
                           detail::Ref<backend::VectorCoo<T>> w;
                           backend::BuildParams buildParams{detail::block_offset(blockSize, i), detail::block_size_at(nrows, blockSize, i), nrows};
                           backend::build(w, m_reduceOp, m_rows, m_values, desc(), buildParams, {i, i});
                           (*blocks)[i] = w;
                       })
                        .precede(build);
            }
        }

    private:
        Vector<T> m_vector;
        ReduceOp m_reduceOp;
        std::vector<Index> m_rows;
        std::vector<T> m_values;
    };

    /**
     * @}
     */

}// namespace spla::expression

#endif//SPLA_BUILD_HPP
