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

#ifndef SPLA_SPLATASKBUILDER_HPP
#define SPLA_SPLATASKBUILDER_HPP

#include <spla-cpp/SplaExpression.hpp>
#include <spla-cpp/SplaExpressionNode.hpp>
#include <taskflow/taskflow.hpp>

#include <functional>
#include <vector>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class TaskBuilder
     * @brief Task builder for expression nodes.
     *
     * Allows to compose dynamically subflows inside expression node
     * task flow graph. Automates exception handling, expression owner notification
     * about errors, and cancellation of the computation if error occurs.
     */
    class TaskBuilder {
    public:
        /**
         * Emplace work to the subflow.
         *
         * @param work Function to execute as work inside task.
         * @return Taskflow task handle.
         */
        tf::Task Emplace(std::function<void()> work);

        /**
         * Emplace work to the subflow.
         * Work will be executed only after all previously submitted tasks to the builder.
         *
         * @param work Function to execute as work inside task.
         * @return Taskflow task handle.
         */
        tf::Task EmplaceAfterAll(std::function<void()> work);

    private:
        friend class ExpressionManager;
        TaskBuilder(Expression *expression, tf::Subflow &subflow);

        Expression *mExpression;
        tf::Subflow &mSubflow;
        std::vector<tf::Task> mTasks;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLATASKBUILDER_HPP
