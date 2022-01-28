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
#include <string>

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
         * @brief Emplace work to the subflow.
         *
         * @param workName Name of the task (for debugging and error handling)
         * @param work Function to execute as work inside task.
         * @return Taskflow task handle.
         */
        tf::Task Emplace(const std::string &workName, std::function<void()> work);

    private:
        friend class ExpressionManager;
        TaskBuilder(Expression *expression, tf::Subflow &subflow);

        Expression *mExpression;
        tf::Subflow &mSubflow;
        std::size_t mTaskID = 0;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLATASKBUILDER_HPP
