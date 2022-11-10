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

#ifndef SPLA_CPU_VECTOR_SELECT_COUNT_HPP
#define SPLA_CPU_VECTOR_SELECT_COUNT_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_select_count_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_select_count_cpu() override = default;

        std::string get_name() override {
            return "v_reduce";
        }

        std::string get_description() override {
            return "sequential vector select count on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast<ScheduleTask_v_select_count>();

            auto r         = t->r;
            auto v         = t->v.template cast<TVector<T>>();
            auto op_select = t->op_select.template cast<TOpSelect<T>>();

            v->ensure_dense_format();
            const auto* p_dense  = v->template get_dec_p<CpuDenseVec<T>>();
            const auto& function = op_select->function;

            int count = 0;

            for (const auto& value : p_dense->Ax) {
                if (function(value)) ++count;
            }

            r->set_int(count);

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_VECTOR_SELECT_COUNT_HPP
