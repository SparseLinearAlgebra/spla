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
        AssignVector(Vector<T> w,
                     std::optional<Vector<M>> mask,
                     AccumOp accumOp,
                     T value,
                     const Descriptor &desc,
                     std::size_t id,
                     Expression &expression)
            : ExpressionNode(desc, id, expression),
              m_w(std::move(w)),
              m_mask(std::move(mask)),
              m_accumOp(std::move(accumOp)),
              m_value(std::move(value)) {}

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
            auto use_replace = desc().replace();

            if (!use_mask)
                throw std::runtime_error("unmasked assignment not supported yet");
            if (use_scmp)
                throw std::runtime_error("structure complement not supported");
            if (use_replace)
                throw std::runtime_error("replace not supported yet");

            auto blocks = detail::make_blocks(storage);

            auto w_schema = storage->schema();
            auto m_schema = storage->schema();
            auto r_schema = m_schema == VectorSchema::Dense ? VectorSchema::Dense : w_schema;

            auto build = builder.emplace("set-blocks", [r_schema, storage, blocks]() {
                storage->build(r_schema, std::move(*blocks));
            });

            for (std::size_t i = 0; i < storage->block_count_rows(); i++) {
                builder.emplace("assign-block", [i, blocks, w_storage = storage, this]() {
                           backend::DispatchParams dispatchParams{i, i};
                           backend::AssignParams assignParams{};

                           auto use_mask = m_mask.has_value();
                           auto use_accum = !null_op<AccumOp>();
                           auto use_scmp = desc().scmp();
                           auto use_replace = desc().replace();

                           auto m_storage = m_mask.value().storage();
                           auto w_schema = w_storage->schema();
                           auto m_schema = m_storage->schema();

                           if (w_schema == VectorSchema::Sparse &&
                               m_schema == VectorSchema::Dense) {
                               // todo: transition w to dense vector
                               SPLA_LOG_ERROR("not conversion from sparse to dense yet");
                           }
                           if (w_schema == VectorSchema::Sparse &&
                               m_schema == VectorSchema::Sparse) {
                               auto block = w_storage->block(i).template cast<backend::VectorCoo<T>>();
                               backend::assign(block, m_storage->block(i).template cast<backend::VectorCoo<M>>(), m_accumOp, m_value, desc(), assignParams, dispatchParams);
                               (*blocks)[i] = block;
                           }
                           if (w_schema == VectorSchema::Dense &&
                               m_schema == VectorSchema::Sparse) {
                               auto block = w_storage->block(i).template cast<backend::VectorDense<T>>();
                               backend::assign(block, m_storage->block(i).template cast<backend::VectorCoo<M>>(), m_accumOp, m_value, desc(), assignParams, dispatchParams);
                               (*blocks)[i] = block;
                           }
                           if (w_schema == VectorSchema::Dense &&
                               m_schema == VectorSchema::Dense) {
                               auto block = w_storage->block(i).template cast<backend::VectorDense<T>>();
                               backend::assign(block, m_storage->block(i).template cast<backend::VectorDense<M>>(), m_accumOp, m_value, desc(), assignParams, dispatchParams);
                               (*blocks)[i] = block;
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
