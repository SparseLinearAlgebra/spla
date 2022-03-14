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

#ifndef SPLA_ASSIGN_HPP
#define SPLA_ASSIGN_HPP

#include <algorithm>
#include <optional>
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

    template<typename T, typename M, typename AccumOp>
    class AssignVector final : public ExpressionNode {
    public:
        AssignVector(Vector<T> w, std::optional<Vector<M>> mask, AccumOp accumOp, T value, const Descriptor &desc, std::size_t id, Expression &expression)
            : ExpressionNode(desc, id, expression), m_w(std::move(w)), m_mask(std::move(mask)), m_accumOp(std::move(accumOp)), m_value(std::move(value)) {}

        ~AssignVector() override = default;

        std::string type() const override {
            return "vector-assign";
        }

    private:
        void prepare() override {
            m_w.storage()->lock_write();
            if (m_mask) m_mask.value().storage()->lock_read();
        }

        void finalize() override {
            m_w.storage()->unlock_write();
            if (m_mask) m_mask.value().storage()->unlock_read();
        }

        void execute(detail::SubtaskBuilder &builder) override {
            auto storage = m_w.storage();

            auto use_mask = m_mask.has_value();
            auto use_scmp = desc().scmp();

            if (!use_mask)
                throw std::runtime_error("unmasked assignment not supported yet");
            if (use_scmp)
                throw std::runtime_error("structure complement not supported");

            auto b_sparse = detail::make_vector_blocks_sparse(storage);
            auto b_dense = detail::make_vector_blocks_dense(storage);

            auto w_schema = storage->storage_schema();
            auto m_schema = storage->storage_schema();
            auto r_schema = w_schema == StorageSchema::Sparse && m_schema == StorageSchema::Dense ? StorageSchema::Dense : w_schema;

            auto build = builder.emplace("set-blocks", [=]() {
                if (r_schema == StorageSchema::Sparse)
                    storage->build(std::move(*b_sparse));
                else if (r_schema == StorageSchema::Dense)
                    storage->build(std::move(*b_dense));
                else
                    throw std::runtime_error("undefined storage schema");
            });

            for (std::size_t i = 0; i < storage->block_count_rows(); i++) {
                builder.emplace("assign-block", [i, b_sparse, b_dense, w_storage = storage, this]() {
                           backend::DispatchParams dispatchParams{i, i};
                           backend::AssignParams assignParams{};

                           auto use_mask = m_mask.has_value();
                           auto use_accum = !null_op<AccumOp>();
                           auto use_scmp = desc().scmp();
                           auto use_replace = desc().replace();

                           auto mask_storage = m_mask.value().storage();
                           auto w_schema = w_storage->storage_schema();
                           auto mask_schema = mask_storage->storage_schema();

                           if (w_schema == StorageSchema::Sparse) {
                               auto w_block = w_storage->block_sparse(i);

                               if (mask_schema == StorageSchema::Sparse) {
                                   backend::assign(w_block, mask_storage->block_sparse(i), m_accumOp, m_value, desc(), assignParams, dispatchParams);
                                   (*b_sparse)[i] = w_block;
                               } else {
                                   // transition w to dense vector
                               }
                           }

                           if (w_schema == StorageSchema::Dense) {
                               auto w_block = w_storage->block_dense(i);

                               if (mask_schema == StorageSchema::Sparse)
                                   backend::assign(w_block, mask_storage->block_sparse(i), m_accumOp, m_value, desc(), assignParams, dispatchParams);
                               else
                                   backend::assign(w_block, mask_storage->block_dense(i), m_accumOp, m_value, desc(), assignParams, dispatchParams);

                               (*b_dense)[i] = w_block;
                           }
                       })
                        .precede(build);
            }
        }

    private:
        Vector<T> m_w;
        std::optional<Vector<M>> m_mask;
        AccumOp m_accumOp;
        T m_value;
    };

    /**
     * @}
     */

}// namespace spla::expression

#endif//SPLA_ASSIGN_HPP
