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

#ifndef SPLA_EXPRESSION_NODE_HPP
#define SPLA_EXPRESSION_NODE_HPP

#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

#include <spla/detail/ref.hpp>
#include <spla/detail/subtask_builder.hpp>

namespace spla {

    class Expression;

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class ExpressionNode
     * @brief Represents and operation in the expression.
     *
     * Stores operation type and required arguments for evaluation.
     * Expression nodes form a computational expression (or dag) with specific dependencies ordering.
     */
    class ExpressionNode : public detail::RefCnt {
    public:
        /**
         * @brief Creates an expression node
         * @note For internal usage
         *
         * @param desc Node descriptor
         * @param id Node id inside expression node
         * @param expression Expression this node belongs to
         */
        explicit ExpressionNode(Descriptor desc, std::size_t id, Expression &expression)
            : m_desc(std::move(desc)), m_id(id), m_expression(expression) {}

        ~ExpressionNode() override = default;

        /**
         * @brief Adds precede link from this node to passed node
         *
         * @param node Node execution to precede
         *
         * @return This node
         */
        detail::Ref<ExpressionNode> precede(const detail::Ref<ExpressionNode> &node) {
            assert(node.is_not_null());
            assert(&expression() == &node->expression());

            if (node.is_null())
                throw std::runtime_error("passed null node to precede");
            if (&expression() != &node->expression())
                throw std::runtime_error("nodes must belong to the same expression");

            link(node.get());

            return detail::Ref<ExpressionNode>(this);
        }

        /**
         * @brief Adds succeed link from this node to passed node
         *
         * @param node Node execution to succeed
         *
         * @return This node
         */
        detail::Ref<ExpressionNode> succeed(const detail::Ref<ExpressionNode> &node) {
            assert(node.is_not_null());
            assert(&expression() == &node->expression());

            if (node.is_null())
                throw std::runtime_error("passed null node to succeed");
            if (&expression() != &node->expression())
                throw std::runtime_error("nodes must belong to the same expression");

            node->link(this);

            return detail::Ref<ExpressionNode>(this);
        }

        /**
         * @brief Set node name for debug display
         *
         * @param name Node name to set
         */
        void set_name(std::string name) { m_name = std::move(name); }

        /** @return Type of the node */
        [[nodiscard]] virtual std::string type() const = 0;

        /** @return Node id inside expression */
        [[nodiscard]] std::size_t id() const { return m_id; }

        /** @return Expression this node belongs to */
        [[nodiscard]] Expression &expression() const { return m_expression; }

        /** @return Node descriptor */
        [[nodiscard]] const Descriptor &desc() const { return m_desc; }

        /** @return Node name */
        [[nodiscard]] const std::string &name() const { return m_name; }

        /** @return Nodes dependencies, which precede this node execution */
        [[nodiscard]] const std::vector<ExpressionNode *> &predecessors() const { return m_predecessors; }

        /** @return Nodes dependencies, which succeed this node execution */
        [[nodiscard]] const std::vector<ExpressionNode *> &successors() const { return m_successors; }

    private:
        friend class Expression;

        /**
         * @brief Links this node and passed node as next
         *
         * @param next Next node to link
         */
        void link(ExpressionNode *next) {
            m_successors.push_back(next);
            next->m_predecessors.push_back(this);
        }

        virtual void prepare() const = 0;
        virtual void finalize() const = 0;
        virtual void execute(detail::SubtaskBuilder &builder) const = 0;

    private:
        Descriptor m_desc;
        Expression &m_expression;

        std::size_t m_id;
        std::string m_name;
        std::vector<ExpressionNode *> m_predecessors;
        std::vector<ExpressionNode *> m_successors;
    };

    detail::Ref<ExpressionNode> operator>>(const detail::Ref<ExpressionNode> &pred, const detail::Ref<ExpressionNode> &succ) {
        pred->precede(succ);
        return succ;
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_EXPRESSION_NODE_HPP
