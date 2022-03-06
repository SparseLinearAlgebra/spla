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

#include <utility>
#include <vector>

#include <spla/detail/ref.hpp>
#include <spla/expression/subtask_builder.hpp>

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
        explicit ExpressionNode(Descriptor desc, std::size_t id, Expression &expression)
            : m_desc(std::move(desc)), m_id(id), m_expression(&expression) {}

        ~ExpressionNode() override = default;

        void set_name(std::string name) { m_name = std::move(name); }

        [[nodiscard]] virtual std::string type() const = 0;

        [[nodiscard]] std::size_t id() const { return m_id; }
        [[nodiscard]] Expression *expression() const { return m_expression; }
        [[nodiscard]] const Descriptor &desc() const { return m_desc; }
        [[nodiscard]] const std::string &name() const { return m_name; }
        [[nodiscard]] const std::vector<ExpressionNode *> &predecessors() const { return m_predecessors; }
        [[nodiscard]] const std::vector<ExpressionNode *> &successors() const { return m_successors; }

    private:
        friend class Expression;

        void link(ExpressionNode *next) {
            m_successors.push_back(next);
            next->m_predecessors.push_back(this);
        }

        virtual void prepare() = 0;
        virtual void finalize() = 0;
        virtual void execute(expression::SubtaskBuilder &builder) = 0;

    private:
        Expression *m_expression;
        Descriptor m_desc;

        std::size_t m_id;
        std::string m_name;
        std::vector<ExpressionNode *> m_predecessors;
        std::vector<ExpressionNode *> m_successors;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_EXPRESSION_NODE_HPP
