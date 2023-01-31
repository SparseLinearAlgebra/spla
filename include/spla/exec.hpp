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

#ifndef SPLA_EXEC_HPP
#define SPLA_EXEC_HPP

#include "config.hpp"
#include "descriptor.hpp"
#include "matrix.hpp"
#include "object.hpp"
#include "op.hpp"
#include "scalar.hpp"
#include "schedule.hpp"
#include "vector.hpp"

namespace spla {

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
     * @brief Execute (schedule) r = Card({v[i]: select(v[i])})
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
    SPLA_API Status exec_v_count_if(
            ref_ptr<Scalar>        r,
            ref_ptr<Vector>        v,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) r = Card({v[i]: v[i] != 0})
     *
     * Count number of non-zero entries in the provided vector container.
     * Use this function to obtain actual number of non-zero values in a container.
     * Since container can use sparse or dense storage schema, actual number on
     * non-zeros must be explicitly evaluated. This functions is useful for
     * algorithms which employ sparsity of input.
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Scalar (int) to store count of non-zero entries
     * @param v Vector to count number of non-zero entries
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_count_nz(
            ref_ptr<Scalar>        r,
            ref_ptr<Vector>        v,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

}// namespace spla

#endif//SPLA_EXEC_HPP