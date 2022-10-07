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

#include <spla/schedule.hpp>

#include <schedule/schedule_st.hpp>
#include <schedule/schedule_tasks.hpp>

namespace spla {

    ref_ptr<Schedule> make_schedule() {
        return ref_ptr<Schedule>(new ScheduleSingleThread);
    }

    ref_ptr<ScheduleTask> make_sched_callback(
            ScheduleCallback    callback,
            ref_ptr<Descriptor> desc) {
        auto task      = make_ref<ScheduleTask_callback>();
        task->callback = std::move(callback);
        task->desc     = std::move(desc);
        return task.as<ScheduleTask>();
    }

    ref_ptr<ScheduleTask> make_sched_mxv_masked(
            ref_ptr<Vector>     r,
            ref_ptr<Vector>     mask,
            ref_ptr<Matrix>     M,
            ref_ptr<Vector>     v,
            ref_ptr<OpBinary>   op_multiply,
            ref_ptr<OpBinary>   op_add,
            bool                opt_complement,
            ref_ptr<Descriptor> desc) {
        auto task            = make_ref<ScheduleTask_mxv_masked>();
        task->r              = std::move(r);
        task->mask           = std::move(mask);
        task->M              = std::move(M);
        task->v              = std::move(v);
        task->op_multiply    = std::move(op_multiply);
        task->op_add         = std::move(op_add);
        task->opt_complement = opt_complement;
        task->desc           = std::move(desc);
        return task.as<ScheduleTask>();
    }

    ref_ptr<ScheduleTask> make_sched_v_assign_masked(
            ref_ptr<Vector>     r,
            ref_ptr<Vector>     mask,
            ref_ptr<Scalar>     value,
            ref_ptr<OpBinary>   op_assign,
            ref_ptr<Descriptor> desc) {
        auto task       = make_ref<ScheduleTask_v_assign_masked>();
        task->r         = std::move(r);
        task->mask      = std::move(mask);
        task->value     = std::move(value);
        task->op_assign = std::move(op_assign);
        task->desc      = std::move(desc);
        return task.as<ScheduleTask>();
    }

    ref_ptr<ScheduleTask> make_sched_v_reduce(
            ref_ptr<Scalar>     r,
            ref_ptr<Scalar>     s,
            ref_ptr<Vector>     v,
            ref_ptr<OpBinary>   op_reduce,
            ref_ptr<Descriptor> desc) {
        auto task       = make_ref<ScheduleTask_v_reduce>();
        task->r         = std::move(r);
        task->s         = std::move(s);
        task->v         = std::move(v);
        task->op_reduce = std::move(op_reduce);
        task->desc      = std::move(desc);
        return task.as<ScheduleTask>();
    }

}// namespace spla