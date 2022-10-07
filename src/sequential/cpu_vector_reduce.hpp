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

#ifndef SPLA_CPU_VECTOR_REDUCE_HPP
#define SPLA_CPU_VECTOR_REDUCE_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_reduce_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_reduce_cpu() override = default;

        std::string get_name() override {
            return "v_reduce_cpu";
        }

        std::string get_description() override {
            return "sequential vector reduction on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast<ScheduleTask_v_reduce>();

            auto r         = t->r.template cast<TScalar<T>>();
            auto s         = t->s.template cast<TScalar<T>>();
            auto v         = t->v.template cast<TVector<T>>();
            auto op_reduce = t->op_reduce.template cast<TOpBinary<T, T, T>>();

            T sum = s->get_value();

            v->ensure_dense_format();
            const auto* p_dense  = v->template get_dec_p<CpuDenseVec<T>>();
            const auto& function = op_reduce->function;

            for (const auto& value : p_dense->Ax) {
                sum = function(sum, value);
            }

            r->get_value() = sum;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_VECTOR_REDUCE_HPP
