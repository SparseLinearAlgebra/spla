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

#ifndef SPLA_SCHEDULE_ST_HPP
#define SPLA_SCHEDULE_ST_HPP

#include <spla/schedule.hpp>

#include <svector.hpp>

#include <string>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class ScheduleSingleThread
     * @brief Single-thread dispatch sequential execution schedule
     */
    class ScheduleSingleThread final : public Schedule {
    public:
        ~ScheduleSingleThread() override = default;
        Status             step_task(ref_ptr<ScheduleTask> task) override;
        Status             step_tasks(std::vector<ref_ptr<ScheduleTask>> tasks) override;
        Status             submit() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;

    private:
        using vector_step  = ankerl::svector<ref_ptr<ScheduleTask>, 4>;
        using vector_steps = ankerl::svector<vector_step, 4>;

        vector_steps m_steps;
        std::string  m_label;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SCHEDULE_ST_HPP
