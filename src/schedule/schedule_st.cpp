/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#include "schedule_st.hpp"

#include <core/dispatcher.hpp>

namespace spla {

    Status ScheduleSingleThread::step_task(ref_ptr<ScheduleTask> task) {
        m_steps.emplace_back().push_back(std::move(task));
        return Status::Ok;
    }

    Status ScheduleSingleThread::step_tasks(std::vector<ref_ptr<ScheduleTask>> tasks) {
        auto& step = m_steps.emplace_back();
        step.reserve(tasks.size());
        for (auto& task : tasks) step.push_back(std::move(task));
        return Status::Ok;
    }

    Status ScheduleSingleThread::submit() {
        Dispatcher*     g_dispatcher = Library::get()->get_dispatcher();
        DispatchContext ctx{};

        ctx.schedule = ref_ptr<Schedule>(this);

        for (int step_id = 0; step_id < static_cast<int>(m_steps.size()); step_id++) {
            auto& step  = m_steps[step_id];
            ctx.step_id = step_id;

            for (int task_id = 0; task_id < static_cast<int>(step.size()); task_id++) {
                auto& task  = step[task_id];
                ctx.task    = task;
                ctx.task_id = task_id;

                auto status = g_dispatcher->dispatch(ctx);
                if (status != Status::Ok) {
                    return status;
                }
            }
        }

        return Status::Ok;
    }

    void ScheduleSingleThread::set_label(std::string label) {
        m_label = std::move(label);
    }

    const std::string& ScheduleSingleThread::get_label() const {
        return m_label;
    }

}// namespace spla
