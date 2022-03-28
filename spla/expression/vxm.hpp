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
        void prepare() override {
            m_w.storage()->lock_write();
            if (m_mask.has_value()) m_mask.value().storage()->lock_read();
            m_v.storage()->lock_read();
            m_m.storage()->lock_read();
        }

        void finalize() override {
            m_w.storage()->unlock_write();
            if (m_mask.has_value()) m_mask.value().storage()->unlock_read();
            m_v.storage()->unlock_read();
            m_m.storage()->unlock_read();
        }

        void execute(detail::SubtaskBuilder &builder) override {
            auto storage = m_w.storage();

            auto use_mask = m_mask.has_value();
            auto use_replace = desc().replace();

            if (!use_mask)
                throw std::runtime_error("unmasked vxm not supported yet");
            if (use_replace)
                throw std::runtime_error("replace not supported yet");


        }

    private:
        Vector<T> m_w;
        std::optional<Vector<M>> m_mask;
        AccumOp m_accumOp;
        MultiplyOp m_multiplyOp;
        ReduceOp m_reduceOp;
        Vector<T> m_v;
        Matrix<T> m_m;
    };

}// namespace spla::expression

#endif//SPLA_VXM_HPP
