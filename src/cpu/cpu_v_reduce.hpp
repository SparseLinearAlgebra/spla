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

#ifndef SPLA_CPU_V_REDUCE_HPP
#define SPLA_CPU_V_REDUCE_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
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
            return "v_reduce";
        }

        std::string get_description() override {
            return "sequential vector reduction on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_reduce>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (v->is_valid(FormatVector::CpuCoo)) {
                return execute_sp(ctx);
            }
            if (v->is_valid(FormatVector::CpuDense)) {
                return execute_dn(ctx);
            }

            return execute_sp(ctx);
        }

    private:
        Status execute_sp(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_reduce_sparse");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_reduce>();

            auto r         = t->r.template cast_safe<TScalar<T>>();
            auto s         = t->s.template cast_safe<TScalar<T>>();
            auto v         = t->v.template cast_safe<TVector<T>>();
            auto op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            T sum = s->get_value();

            v->validate_rw(FormatVector::CpuCoo);
            const auto* p_sparse = v->template get<CpuCooVec<T>>();
            const auto& function = op_reduce->function;

            for (const auto& value : p_sparse->Ax) {
                sum = function(sum, value);
            }

            r->get_value() = sum;

            return Status::Ok;
        }

        Status execute_dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_reduce_dense");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_reduce>();

            auto r         = t->r.template cast_safe<TScalar<T>>();
            auto s         = t->s.template cast_safe<TScalar<T>>();
            auto v         = t->v.template cast_safe<TVector<T>>();
            auto op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            T sum = s->get_value();

            v->validate_rw(FormatVector::CpuDense);
            const auto* p_dense  = v->template get<CpuDenseVec<T>>();
            const auto& function = op_reduce->function;

            for (const auto& value : p_dense->Ax) {
                sum = function(sum, value);
            }

            r->get_value() = sum;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_V_REDUCE_HPP
