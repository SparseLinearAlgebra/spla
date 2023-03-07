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

#ifndef SPLA_CPU_V_EADD_HPP
#define SPLA_CPU_V_EADD_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_eadd_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_eadd_cpu() override = default;

        std::string get_name() override {
            return "v_eadd";
        }

        std::string get_description() override {
            return "sequential element-wise add vector operation";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_eadd>();
            ref_ptr<TVector<T>> u = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (u->is_valid(FormatVector::CpuDense) && v->is_valid(FormatVector::CpuDense)) {
                return execute_dn2dn(ctx);
            }

            return execute_dn2dn(ctx);
        }

    private:
        Status execute_dn2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_eadd_dn2dn");

            auto                        t  = ctx.task.template cast_safe<ScheduleTask_v_eadd>();
            ref_ptr<TVector<T>>         r  = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         u  = t->u.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v  = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op = t->op.template cast_safe<TOpBinary<T, T, T>>();

            r->validate_wd(FormatVector::CpuDense);
            u->validate_rw(FormatVector::CpuDense);
            v->validate_rw(FormatVector::CpuDense);

            auto*       p_r      = r->template get<CpuDenseVec<T>>();
            const auto* p_u      = u->template get<CpuDenseVec<T>>();
            const auto* p_v      = v->template get<CpuDenseVec<T>>();
            const auto& function = op->function;

            const uint N = r->get_n_rows();

            for (uint i = 0; i < N; i++) {
                p_r->Ax[i] = function(p_u->Ax[i], p_v->Ax[i]);
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_V_EADD_HPP
