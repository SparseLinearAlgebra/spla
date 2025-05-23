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

#ifndef SPLA_SCHEDULE_TASKS_HPP
#define SPLA_SCHEDULE_TASKS_HPP

#include <spla/exec.hpp>
#include <spla/schedule.hpp>

#include <profiling/time_profiler.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class ScheduleTaskBase
     * @brief Base schedule task class with common public properties
     */
    class ScheduleTaskBase : public ScheduleTask {
    public:
        ~ScheduleTaskBase() override = default;

        void                set_label(std::string label) override;
        const std::string&  get_label() const override;
        ref_ptr<Descriptor> get_desc() override;
        ref_ptr<Descriptor> get_desc_or_default() override;

        std::string         label;
        ref_ptr<Descriptor> desc;
    };

    /**
     * @class ScheduleTask_callback
     * @brief Callback task
     */
    class ScheduleTask_callback final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_callback() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ScheduleCallback callback;
    };

    /**
     * @class ScheduleTask_mxm
     * @brief Sparse matrix sparse matrix product
     */
    class ScheduleTask_mxm final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_mxm() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>   R;
        ref_ptr<Matrix>   A;
        ref_ptr<Matrix>   B;
        ref_ptr<OpBinary> op_multiply;
        ref_ptr<OpBinary> op_add;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_mxmT_masked
     * @brief Masked matrix matrix-transposed product
     */
    class ScheduleTask_mxmT_masked final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_mxmT_masked() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>   R;
        ref_ptr<Matrix>   mask;
        ref_ptr<Matrix>   A;
        ref_ptr<Matrix>   B;
        ref_ptr<OpBinary> op_multiply;
        ref_ptr<OpBinary> op_add;
        ref_ptr<OpSelect> op_select;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_kron
     * @brief Sparse matrix kronecker product
     */
    class ScheduleTask_kron final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_kron() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>   R;
        ref_ptr<Matrix>   A;
        ref_ptr<Matrix>   B;
        ref_ptr<OpBinary> op_multiply;
    };

    /**
     * @class ScheduleTask_mxv_masked
     * @brief Masked matrix-vector product
     */
    class ScheduleTask_mxv_masked final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_mxv_masked() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   mask;
        ref_ptr<Matrix>   M;
        ref_ptr<Vector>   v;
        ref_ptr<OpBinary> op_multiply;
        ref_ptr<OpBinary> op_add;
        ref_ptr<OpSelect> op_select;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_vxm_masked
     * @brief Masked vector-matrix product
     */
    class ScheduleTask_vxm_masked final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_vxm_masked() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   mask;
        ref_ptr<Vector>   v;
        ref_ptr<Matrix>   M;
        ref_ptr<OpBinary> op_multiply;
        ref_ptr<OpBinary> op_add;
        ref_ptr<OpSelect> op_select;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_m_eadd
     * @brief Matrix ewise add
     */
    class ScheduleTask_m_eadd final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_eadd() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>   R;
        ref_ptr<Matrix>   A;
        ref_ptr<Matrix>   B;
        ref_ptr<OpBinary> op;
    };

    /**
     * @class ScheduleTask_m_emult
     * @brief Matrix ewise add
     */
    class ScheduleTask_m_emult final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_emult() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>   R;
        ref_ptr<Matrix>   A;
        ref_ptr<Matrix>   B;
        ref_ptr<OpBinary> op;
    };

    /**
     * @class ScheduleTask_m_reduce_by_row
     * @brief Matrix by row reduction
     */
    class ScheduleTask_m_reduce_by_row final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_reduce_by_row() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Matrix>   M;
        ref_ptr<OpBinary> op_reduce;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_m_reduce_by_column
     * @brief Matrix by col reduction
     */
    class ScheduleTask_m_reduce_by_column final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_reduce_by_column() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Matrix>   M;
        ref_ptr<OpBinary> op_reduce;
        ref_ptr<Scalar>   init;
    };

    /**
     * @class ScheduleTask_m_reduce
     * @brief Matrix reduction to scalar
     */
    class ScheduleTask_m_reduce final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_reduce() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Scalar>   r;
        ref_ptr<Scalar>   s;
        ref_ptr<Matrix>   M;
        ref_ptr<OpBinary> op_reduce;
    };

    /**
     * @class ScheduleTask_m_transpose
     * @brief Matrix transpose
     */
    class ScheduleTask_m_transpose final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_transpose() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Matrix>  R;
        ref_ptr<Matrix>  M;
        ref_ptr<OpUnary> op_apply;
    };

    /**
     * @class ScheduleTask_m_extract_row
     * @brief Matrix extract vector
     */
    class ScheduleTask_m_extract_row final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_extract_row() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>  r;
        ref_ptr<Matrix>  M;
        uint             index;
        ref_ptr<OpUnary> op_apply;
    };

    /**
     * @class ScheduleTask_m_extract_column
     * @brief Matrix extract vector
     */
    class ScheduleTask_m_extract_column final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_m_extract_column() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>  r;
        ref_ptr<Matrix>  M;
        uint             index;
        ref_ptr<OpUnary> op_apply;
    };

    /**
     * @class ScheduleTask_v_eadd
     * @brief Vector ewise add
     */
    class ScheduleTask_v_eadd final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_eadd() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   u;
        ref_ptr<Vector>   v;
        ref_ptr<OpBinary> op;
    };

    /**
     * @class ScheduleTask_v_emult
     * @brief Vector ewise mult
     */
    class ScheduleTask_v_emult final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_emult() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   u;
        ref_ptr<Vector>   v;
        ref_ptr<OpBinary> op;
    };

    /**
     * @class ScheduleTask_v_eadd_fdb
     * @brief Vector ewise with feedback
     */
    class ScheduleTask_v_eadd_fdb final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_eadd_fdb() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   v;
        ref_ptr<Vector>   fdb;
        ref_ptr<OpBinary> op;
    };

    /**
     * @class ScheduleTask_v_assign_masked
     * @brief Masked vector assignment
     */
    class ScheduleTask_v_assign_masked final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_assign_masked() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   mask;
        ref_ptr<Scalar>   value;
        ref_ptr<OpBinary> op_assign;
        ref_ptr<OpSelect> op_select;
    };

    /**
     * @class ScheduleTask_v_map
     * @brief Vector map to vector
     */
    class ScheduleTask_v_map final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_map() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>  r;
        ref_ptr<Vector>  v;
        ref_ptr<OpUnary> op;
    };

    /**
     * @class ScheduleTask_v_reduce
     * @brief Vector reduction to scalar
     */
    class ScheduleTask_v_reduce final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_reduce() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Scalar>   r;
        ref_ptr<Scalar>   s;
        ref_ptr<Vector>   v;
        ref_ptr<OpBinary> op_reduce;
    };

    /**
     * @class ScheduleTask_v_count_mf
     * @brief Vector count meaningful elements
     */
    class ScheduleTask_v_count_mf final : public ScheduleTaskBase {
    public:
        ~ScheduleTask_v_count_mf() override = default;

        std::string                  get_name() override;
        std::string                  get_key() override;
        std::string                  get_key_full() override;
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Scalar> r;
        ref_ptr<Vector> v;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SCHEDULE_TASKS_HPP
