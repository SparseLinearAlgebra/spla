/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#ifndef SPLA_SCHEDULE_HPP
#define SPLA_SCHEDULE_HPP

#include "config.hpp"
#include "descriptor.hpp"
#include "matrix.hpp"
#include "object.hpp"
#include "op.hpp"
#include "scalar.hpp"
#include "vector.hpp"

#include <string>
#include <vector>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class ScheduleTask
     * @brief Represent single smallest evaluation tasks which can scheduled
     */
    class ScheduleTask : public Object {
    public:
        SPLA_API ~ScheduleTask() override                                   = default;
        SPLA_API virtual std::string                  get_name()            = 0;
        SPLA_API virtual std::string                  get_key()             = 0;
        SPLA_API virtual std::vector<ref_ptr<Object>> get_args()            = 0;
        SPLA_API virtual ref_ptr<Descriptor>          get_desc()            = 0;
        SPLA_API virtual ref_ptr<Descriptor>          get_desc_or_default() = 0;
    };

    /**
     * @class Schedule
     * @brief Object with sequence of steps with tasks forming schedule for execution
     */
    class Schedule : public Object {
    public:
        SPLA_API ~Schedule() override                                                = default;
        SPLA_API virtual Status step_task(ref_ptr<ScheduleTask> task)                = 0;
        SPLA_API virtual Status step_tasks(std::vector<ref_ptr<ScheduleTask>> tasks) = 0;
        SPLA_API virtual Status submit()                                             = 0;
    };

    /**
     * @brief Makes new schedule for making execution schedule
     *
     * @return New created schedule
     */
    SPLA_API ref_ptr<Schedule> make_schedule();

    /**
     * @brief Execute (schedule) callback function
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param callback User-defined function to call as scheduled task
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_callback(
            ScheduleCallback       callback,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r<select(mask)> = Mxv
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     * @note Semantic `r[i] = select(mask)? M[i,*] * v: init`
     *
     * @param r Vector to store operation result
     * @param mask Vector to select for which values to compute product
     * @param M Matrix for product
     * @param v Vector for product
     * @param op_multiply Element-wise binary operator for matrix vector elements product
     * @param op_add Element-wise binary operator for matrix vector products sum
     * @param op_select Selection op to filter mask
     * @param init Init of matrix row and vector product
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_mxv_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Matrix>        M,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r<select(mask)> = vxM
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     * @note Semantic `r[i] = select(mask)? v * M[*,i]: init`
     *
     * @param r Vector to store operation result
     * @param mask Vector to select for which values to compute product
     * @param v Vector for product
     * @param M Matrix for product
     * @param op_multiply Element-wise binary operator for matrix vector elements product
     * @param op_add Element-wise binary operator for matrix vector products sum
     * @param op_select Selection op to filter mask
     * @param init Init of matrix row and vector product
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_vxm_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Vector>        v,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r<select(mask)> = value
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r
     * @param mask
     * @param value
     * @param op_assign
     * @param op_select
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_assign_masked(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        mask,
            ref_ptr<Scalar>        value,
            ref_ptr<OpBinary>      op_assign,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r = reduce(s, v)
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r
     * @param s
     * @param v
     * @param op_reduce
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_reduce(
            ref_ptr<Scalar>        r,
            ref_ptr<Scalar>        s,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r = Card({select(v[i])})
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Scalar (int) to save count of selected values
     * @param v Vector to select values from
     * @param op_select Option to select
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_select_count(
            ref_ptr<Scalar>        r,
            ref_ptr<Vector>        v,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SCHEDULE_HPP
