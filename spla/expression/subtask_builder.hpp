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

#ifndef SPLA_SUBTASK_BUILDER_HPP
#define SPLA_SUBTASK_BUILDER_HPP

#include <mutex>
#include <stdexcept>

#include <taskflow/taskflow.hpp>

#include <spla/expression/expression_submission.hpp>
#include <spla/io/log.hpp>

namespace spla::expression {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class SubtaskBuilder
     * @brief Wrapper for tf::Subflow for tasks proxy
     */
    class SubtaskBuilder {
    public:
        SubtaskBuilder(tf::Subflow &subflow, ExpressionSubmission::Private *private_ptr)
            : m_subflow(subflow), m_private_ptr(private_ptr) {}

        /**
         * @brief Emplace new task in subflow
         * @tparam Task Task callable type
         *
         * @param task_name Task name for profiling/debugging
         * @param task Task to emplace
         *
         * @return Taskflow task to make dependencies
         */
        template<typename Task>
        tf::Task emplace(std::string task_name, Task &&task) {
            auto task_id = m_task_id++;
            auto task_func = [task, task_id, task_name = std::move(task_name), private_ptr = m_private_ptr]() {
                if (private_ptr->m_state.load() == ExpressionState::Aborted)
                    return;
                try {
                    task();
                } catch (const std::exception &exception) {
                    private_ptr->m_state.store(ExpressionState::Aborted);
                    std::lock_guard<std::mutex> lockGuard(private_ptr->m_mutex);
                    private_ptr->m_error = exception.what();
                    SPLA_LOG_ERROR("error inside task-" << task_id
                                                        << " '" << task_name << "'"
                                                        << " what: " << exception.what());
                }
            };
            return m_subflow.emplace(std::move(task_func));
        }

    private:
        std::size_t m_task_id = 0;
        tf::Subflow &m_subflow;
        ExpressionSubmission::Private *m_private_ptr;
    };

    /**
     * @}
     */

}// namespace spla::expression

#endif//SPLA_SUBTASK_BUILDER_HPP
