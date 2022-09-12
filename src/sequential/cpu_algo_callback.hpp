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

#ifndef SPLA_CPU_ALGO_CALLBACK_HPP
#define SPLA_CPU_ALGO_CALLBACK_HPP

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <schedule/schedule_tasks.hpp>

namespace spla {

    class Algo_callback final : public RegistryAlgo {
    public:
        ~Algo_callback() override = default;
        std::string get_name() override {
            return "callback";
        }
        std::string get_description() override {
            return "call user callback function on cpu";
        }
        Status execute(const DispatchContext& ctx) override {
            auto* task = dynamic_cast<ScheduleTask_callback*>(ctx.task.get());

            if (task->callback) {
                // Invoke callback if present
                task->callback();
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_ALGO_CALLBACK_HPP
