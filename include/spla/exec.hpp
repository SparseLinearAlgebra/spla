/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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
     * @brief Execute (schedule) sparse-matrix sparse-matrix product
     *
     * @note Operation equivalent semantic is `R = AB`
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param R Matrix to store result of the operation
     * @param A Left matrix for product
     * @param B Right matrix for product
     * @param op_multiply Element-wise binary operator for matrices elements product
     * @param op_add Element-wise binary operator for matrices elements products sum
     * @param init Init of matrix row and column product
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_mxm(
            ref_ptr<Matrix>        R,
            ref_ptr<Matrix>        A,
            ref_ptr<Matrix>        B,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) sparse masked matrix matrix-transposed product
     *
     * @note Operation equivalent semantic is `R = AB^t .mask`
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param R Matrix to store result of the operation
     * @param mask Mask to filter product result
     * @param A Left matrix for product
     * @param B Right matrix for product
     * @param op_multiply Element-wise binary operator for matrices elements product
     * @param op_add Element-wise binary operator for matrices elements products sum
     * @param op_select Selection op to filter mask
     * @param init Init of matrix row and column product
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_mxmT_masked(
            ref_ptr<Matrix>        R,
            ref_ptr<Matrix>        mask,
            ref_ptr<Matrix>        A,
            ref_ptr<Matrix>        B,
            ref_ptr<OpBinary>      op_multiply,
            ref_ptr<OpBinary>      op_add,
            ref_ptr<OpSelect>      op_select,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) dense-masked sparse matrix by dense vector product
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
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
     * @brief Execute (schedule) dense-masked sparse vector by sparse matrix product
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
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
     * @brief Execute (schedule) matrix by row reduction to single vector column
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Vector to store reduction of rows
     * @param M Matrix to reduce rows
     * @param op_reduce Binary op to sum elements of single row
     * @param init Scalar identity element for reduction
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_m_reduce_by_row(
            ref_ptr<Vector>        r,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) matrix by column reduction to single vector column
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Vector to store reduction of columns
     * @param M Matrix to reduce columns
     * @param op_reduce Binary op to sum elements of single column
     * @param init Scalar identity element for reduction
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_m_reduce_by_column(
            ref_ptr<Vector>        r,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Scalar>        init,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) matrix by structure reduction to a single scalar value
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Scalar to store reduction result
     * @param s Scalar neutral init value for reduction
     * @param M Matrix to reduce
     * @param op_reduce Binary op to reduce to values
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_m_reduce(
            ref_ptr<Scalar>        r,
            ref_ptr<Scalar>        s,
            ref_ptr<Matrix>        M,
            ref_ptr<OpBinary>      op_reduce,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) element-wise addition by structure of two vectors
     *
     * @param r Vector to store result of operation
     * @param u Vector input to sum
     * @param v Vector input to sum
     * @param op Element-wise binary operator sum elements of vectors
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_eadd(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        u,
            ref_ptr<Vector>        v,
            ref_ptr<OpBinary>      op,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) element-wise addition by structure of two vectors with feedback
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Vector to store operation result
     * @param v Vector add to r element-wise
     * @param fdb feedback vector storing affected r values
     * @param op Element-wise binary operator sum elements of vectors
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_eadd_fdb(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        v,
            ref_ptr<Vector>        fdb,
            ref_ptr<OpBinary>      op,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) masked scalar assignment to a vector
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Vector result to store assigned values
     * @param mask Vector mask to chose where to assign
     * @param value Scalar value to assign
     * @param op_assign Binary op to assign values
     * @param op_select Select op to chose values for assignment
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
     * @brief Execute (schedule) by structure map of one vector to another using unary operation
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Vector result to store mapped values
     * @param v Vector source to map
     * @param op binary op to transform one value to another
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_map(
            ref_ptr<Vector>        r,
            ref_ptr<Vector>        v,
            ref_ptr<OpUnary>       op,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

    /**
     * @brief Execute (schedule) vector by structure reduction to a single scalar value
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Scalar to store reduction result
     * @param s Scalar neutral init value for reduction
     * @param v Vector to reduce
     * @param op_reduce Binary op to reduce to values
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
     * @brief Execute (schedule) count number of meaningful values by vector structure
     *
     * Count number of entries in the provided vector container not equal to fill value.
     * Use this function to obtain actual number of meaningful values in a container.
     * Since container can use sparse or dense storage schema, actual number of
     * meaningful elements must be explicitly evaluated. This functions is useful for
     * algorithms which employ sparsity of input.
     *
     * @note Pass valid `task_hnd` to store as a task, rather then execute immediately.
     *
     * @param r Scalar (int) to store count of meaningful entries
     * @param v Vector to count number of meaningful entries
     * @param desc Scheduled task descriptor; default is null
     * @param task_hnd Optional task hnd; pass not-null pointer to store task
     *
     * @return Status on task execution or status on hnd creation
     */
    SPLA_API Status exec_v_count_mf(
            ref_ptr<Scalar>        r,
            ref_ptr<Vector>        v,
            ref_ptr<Descriptor>    desc     = ref_ptr<Descriptor>(),
            ref_ptr<ScheduleTask>* task_hnd = nullptr);

}// namespace spla

#endif//SPLA_EXEC_HPP
