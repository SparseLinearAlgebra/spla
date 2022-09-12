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

#ifndef SPLA_SCHEDULE_TASKS_HPP
#define SPLA_SCHEDULE_TASKS_HPP

#include <spla/schedule.hpp>

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
        void               set_label(std::string label) override;
        const std::string& get_label() const override;

        std::string label;
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
        std::vector<ref_ptr<Object>> get_args() override;

        ScheduleCallback callback;
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
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   mask;
        ref_ptr<Matrix>   M;
        ref_ptr<Vector>   v;
        ref_ptr<OpBinary> op_multiply;
        ref_ptr<OpBinary> op_add;
        bool              opt_complement;
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
        std::vector<ref_ptr<Object>> get_args() override;

        ref_ptr<Vector>   r;
        ref_ptr<Vector>   mask;
        ref_ptr<Scalar>   value;
        ref_ptr<OpBinary> op_assign;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SCHEDULE_TASKS_HPP
