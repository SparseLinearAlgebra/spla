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

#ifndef SPLA_SPLANODEPROCESSOR_HPP
#define SPLA_SPLANODEPROCESSOR_HPP

#include <spla-cpp/SplaExpression.hpp>
#include <spla-cpp/SplaExpressionNode.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <taskflow/taskflow.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class NodeProcessor
     *
     * @brief Interface to the expression node processor class.
     *
     * Node processor allows to implement custom node processing logic and
     * select required processor automatically in runtime. Selection is based
     * on the node content and global library setting.
     *
     * Single node operation may be supported in multiple node processors.
     * In runtime expression manager will select best matching processor for execution.
     *
     * @see ExpressionManager
     */
    class NodeProcessor : public RefCnt {
    public:
        /**
         * @brief Select this node processor for processing specified node in expression.
         *
         * If this processor can process specified node, it must return true.
         * If this is the best selection, expression manager will choose this
         * processor for node processor. Otherwise, other node processor will
         * be used for processing this node (if more then one is available).
         *
         * @note Node passed as index in the expression.
         *
         * @param nodeIdx Index of the node in expression
         * @param expression Expression being processed
         *
         * @return True if this node can process this node
         */
        virtual bool Select(std::size_t nodeIdx, const Expression &expression) = 0;

        /**
         * @brief Process specified node in the expression.
         *
         * Node processor is supposed to construct execution task graph and store it
         * in the taskflow of the node. No actual computation must happen
         * or be triggered within Process method call. Only when all nodes are processed,
         * result task graph is submitted for the execution.
         *
         * @note Node passed as index in the expression.
         * @note Expression related info is stored in the context.
         *
         * @param nodeIdx Index of the node in expression
         * @param expression Expression being processed
         * @param taskflow Taskflow graph of node nodeIdx
         */
        virtual void Process(std::size_t nodeIdx, const Expression &expression, tf::Taskflow &taskflow) = 0;

        /** @return Type of the expression node operation, handled by this processor */
        virtual ExpressionNode::Operation GetOperationType() const = 0;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLANODEPROCESSOR_HPP
