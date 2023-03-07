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

#include <spla/exec.hpp>
#include <spla/library.hpp>

#include <core/dispatcher.hpp>
#include <schedule/schedule_tasks.hpp>
#include <utility>

namespace spla {

    static Status execute_immediate(ref_ptr<ScheduleTask> task) {
        Dispatcher*     g_dispatcher = Library::get()->get_dispatcher();
        DispatchContext ctx{};
        ctx.thread_id = 0;
        ctx.step_id   = 0;
        ctx.task_id   = 0;
        ctx.task      = std::move(task);
        return g_dispatcher->dispatch(ctx);
    }

#define EXEC_OR_MAKE_TASK                                  \
    if (task_hnd) {                                        \
        *task_hnd = task.as<ScheduleTask>();               \
        return Status::Ok;                                 \
    } else {                                               \
        return execute_immediate(task.as<ScheduleTask>()); \
    }


    Status exec_callback(
            ScheduleCallback       callback,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task      = make_ref<ScheduleTask_callback>();
        task->callback = std::move(callback);
        task->desc     = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_mxmT_masked(
            ref_ptr<Matrix>        R,
            ref_ptr<Matrix>        mask,
            ref_ptr<Matrix>        A,
            ref_ptr<Matrix>        B,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task         = make_ref<ScheduleTask_mxmT_masked>();
        task->R           = std::move(R);
        task->mask        = std::move(mask);
        task->A           = std::move(A);
        task->B           = std::move(B);
        task->op_multiply = std::move(op_multiply);
        task->op_add      = std::move(op_add);
        task->op_select   = std::move(op_select);
        task->init        = std::move(init);
        task->desc        = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_mxv_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Matrix>        M,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task         = make_ref<ScheduleTask_mxv_masked>();
        task->r           = std::move(r);
        task->mask        = std::move(mask);
        task->M           = std::move(M);
        task->v           = std::move(v);
        task->op_multiply = std::move(op_multiply);
        task->op_add      = std::move(op_add);
        task->op_select   = std::move(op_select);
        task->init        = std::move(init);
        task->desc        = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_vxm_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Vector>        v,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task         = make_ref<ScheduleTask_vxm_masked>();
        task->r           = std::move(r);
        task->mask        = std::move(mask);
        task->v           = std::move(v);
        task->M           = std::move(M);
        task->op_multiply = std::move(op_multiply);
        task->op_add      = std::move(op_add);
        task->op_select   = std::move(op_select);
        task->init        = std::move(init);
        task->desc        = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_m_reduce_by_row(
            ref_ptr<Vector>        r,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task       = make_ref<ScheduleTask_m_reduce_by_row>();
        task->r         = std::move(r);
        task->M         = std::move(M);
        task->op_reduce = std::move(op_reduce);
        task->init      = std::move(init);
        task->desc      = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_eadd(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        u,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task  = make_ref<ScheduleTask_v_eadd>();
        task->r    = std::move(r);
        task->u    = std::move(u);
        task->v    = std::move(v);
        task->op   = std::move(op);
        task->desc = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_eadd_fdb(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        v,
            ref_ptr<Vector>        fdb,
            ref_ptr<OpBinary>      op,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task  = make_ref<ScheduleTask_v_eadd_fdb>();
        task->r    = std::move(r);
        task->v    = std::move(v);
        task->fdb  = std::move(fdb);
        task->op   = std::move(op);
        task->desc = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_assign_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Scalar>        value,
            ref_ptr<OpBinary>      op_assign,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task       = make_ref<ScheduleTask_v_assign_masked>();
        task->r         = std::move(r);
        task->mask      = std::move(mask);
        task->value     = std::move(value);
        task->op_assign = std::move(op_assign);
        task->op_select = std::move(op_select);
        task->desc      = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_map(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        v,
            ref_ptr<OpUnary>       op,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task  = make_ref<ScheduleTask_v_map>();
        task->r    = std::move(r);
        task->v    = std::move(v);
        task->op   = std::move(op);
        task->desc = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_reduce(
            ref_ptr<Scalar>        r,
            ref_ptr<Scalar>        s,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task       = make_ref<ScheduleTask_v_reduce>();
        task->r         = std::move(r);
        task->s         = std::move(s);
        task->v         = std::move(v);
        task->op_reduce = std::move(op_reduce);
        task->desc      = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

    Status exec_v_count_mf(
            ref_ptr<Scalar>        r,
            ref_ptr<Vector>        v,
            ref_ptr<Descriptor>    desc,
            ref_ptr<ScheduleTask>* task_hnd) {
        auto task  = make_ref<ScheduleTask_v_count_mf>();
        task->r    = std::move(r);
        task->v    = std::move(v);
        task->desc = std::move(desc);
        EXEC_OR_MAKE_TASK
    }

}// namespace spla