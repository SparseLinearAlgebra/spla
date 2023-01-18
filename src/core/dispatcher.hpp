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

#ifndef SPLA_DISPATCHER_HPP
#define SPLA_DISPATCHER_HPP

#include <spla/schedule.hpp>

#include <core/registry.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class DispatchContext
     * @brief Execution context of a single task
     */
    struct DispatchContext {
        ref_ptr<Schedule>     schedule;
        ref_ptr<ScheduleTask> task;
        int                   thread_id = 0;
        int                   step_id   = 0;
        int                   task_id   = 0;
    };

    /**
     * @class Dispatcher
     * @brief Class responsible for dispatching of execution of a single task
     */
    class Dispatcher {
    public:
        virtual ~Dispatcher() = default;
        virtual Status dispatch(const DispatchContext& ctx);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_DISPATCHER_HPP
