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

#ifndef SPLA_VXM_HPP
#define SPLA_VXM_HPP

#include <algorithm>
#include <optional>
#include <vector>

#include <spla/backend.hpp>
#include <spla/descriptor.hpp>
#include <spla/expression_node.hpp>
#include <spla/matrix.hpp>
#include <spla/types.hpp>
#include <spla/vector.hpp>

#include <spla/detail/backend_utils.hpp>
#include <spla/detail/storage_utils.hpp>

namespace spla::expression {

    template<typename T,
             typename M,
             typename U,
             typename V,
             typename AccumOp,
             typename MultiplyOp,
             typename ReduceOp>
    class VxM final : public ExpressionNode {
    public:
        VxM(Vector<T> w,
            std::optional<Vector<M>> mask,
            AccumOp accumOp,
            MultiplyOp multiplyOp,
            ReduceOp reduceOp,
            Vector<U> v,
            Matrix<V> m,
            const Descriptor &desc,
            std::size_t id,
            Expression &expression)
            : ExpressionNode(desc, id, expression),
              m_w(std::move(w)),
              m_mask(std::move(mask)),
              m_accumOp(std::move(accumOp)),
              m_multiplyOp(std::move(multiplyOp)),
              m_reduceOp(std::move(reduceOp)),
              m_v(std::move(v)),
              m_m(std::move(m)) {
        }

        ~VxM() override = default;

        std::string type() const override {
            return "vxm";
        }

    private:
        void prepare() const override {
            m_w.storage()->lock_write();
            if (m_mask.has_value()) m_mask.value().storage()->lock_read();
            m_v.storage()->lock_read();
            m_m.storage()->lock_read();
        }

        void finalize() const override {
            m_w.storage()->unlock_write();
            if (m_mask.has_value()) m_mask.value().storage()->unlock_read();
            m_v.storage()->unlock_read();
            m_m.storage()->unlock_read();
        }

        void execute(detail::SubtaskBuilder &builder) const override {
            auto use_mask = m_mask.has_value();
            auto use_replace = desc().replace();

            if (!use_mask)
                throw std::runtime_error("unmasked vxm not supported yet");
            if (!use_replace)
                throw std::runtime_error("only replace supported yet");

            auto result_blocks = detail::make_blocks(m_w.storage());
            auto build_result = builder.emplace("set-blocks", [result_blocks, this] {
                m_w.storage()->build(VectorSchema::Sparse, std::move(*result_blocks));
            });

            // Tmp products stored in the grid as v[i] * M[i, j]
            auto tmp_products = std::make_shared<detail::GridTS<typename detail::VectorStorage<T>::Block>>(m_m.storage()->block_count());

            // Temporary blocks evaluation tasks per result block of the w vector
            std::vector<std::vector<tf::Task>> tasks_vxm(m_w.storage()->block_count_rows());

            // Tasks per matrix entry to evaluate tmp[i,j] = v[i] * M[i, j]
            for (auto &entry : m_m.storage()->blocks()->elements()) {
                auto block_idx = entry.first;
                auto task_vxm = builder.emplace("block-vxm", [this, block_idx, tmp_products]() {
                    auto i = block_idx.first;
                    auto j = block_idx.second;

                    auto mask_block = m_mask.value().storage()->block(j);
                    auto v_block = m_v.storage()->block(i);
                    auto m_block = m_m.storage()->block(block_idx);

                    auto v_schema = m_v.storage()->schema();
                    auto m_schema = m_m.storage()->schema();

                    if (v_schema == VectorSchema::Sparse && m_schema == MatrixSchema::Csr) {
                        auto w_block = detail::Ref<backend::VectorCoo<T>>();
                        backend::vxm(w_block,
                                     mask_block.template cast<backend::VectorDense<M>>(),
                                     m_accumOp,
                                     m_multiplyOp,
                                     m_reduceOp,
                                     v_block.template cast<backend::VectorCoo<U>>(),
                                     m_block.template cast<backend::MatrixCsr<V>>(),
                                     desc(),
                                     {i, j, backend::device_for_task(j)});
                        tmp_products->set_block(block_idx, w_block.template as<detail::VectorBlock<T>>());
                    }
                });
                tasks_vxm[block_idx.second].push_back(task_vxm);
            }

            // Tasks to evaluate final w[j]
            for (std::size_t j = 0; j < tasks_vxm.size(); j++) {
                auto task_sum_tmp = builder.emplace("block-tmp-reduce", [j, tmp_products, result_blocks, this]() {
                    const auto &tmp_grid = tmp_products->grid();
                    std::vector<detail::Ref<detail::VectorBlock<T>>> blocks = tmp_grid.column(j);

                    // Nothing to do, no tmp blocks for j
                    if (blocks.empty())
                        return;

                    // Reduce blocks
                    detail::Ref<detail::VectorBlock<T>> result = blocks.front();
                    auto device = backend::device_for_task(j);

                    for (std::size_t i = 1; i < blocks.size(); i++) {
                        auto w_block = detail::Ref<backend::VectorCoo<T>>();
                        backend::ewiseadd(w_block,
                                          m_reduceOp,
                                          result.template cast<backend::VectorCoo<T>>(),
                                          blocks[i].template cast<backend::VectorCoo<T>>(),
                                          desc(),
                                          {j, device});
                        result = w_block.template as<detail::VectorBlock<T>>();
                        SPLA_LOG_INFO("vxm: reduce tmp vector block (" << i << " " << j << ")");
                    }

                    (*result_blocks)[j] = result;
                    SPLA_LOG_INFO("set vector block " << j);
                });

                // Tmp tasks dependency
                for (auto &t : tasks_vxm[j])
                    t.precede(task_sum_tmp);

                // Final build dependency
                task_sum_tmp.precede(build_result);
            }
        }

    private:
        Vector<T> m_w;
        std::optional<Vector<M>> m_mask;
        AccumOp m_accumOp;
        MultiplyOp m_multiplyOp;
        ReduceOp m_reduceOp;
        Vector<U> m_v;
        Matrix<V> m_m;
    };

}// namespace spla::expression

#endif//SPLA_VXM_HPP
