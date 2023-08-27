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

#ifndef SPLA_CPU_V_EADD_FDB_HPP
#define SPLA_CPU_V_EADD_FDB_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_eadd_fdb_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_eadd_fdb_cpu() override = default;

        std::string get_name() override {
            return "v_eadd_fdb";
        }

        std::string get_description() override {
            return "sequential element-wise add vector operation";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>> r = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (r->is_valid(FormatVector::CpuDense) && v->is_valid(FormatVector::CpuCoo)) {
                return execute_sp2dn(ctx);
            }
            if (r->is_valid(FormatVector::CpuDense) && v->is_valid(FormatVector::CpuDense)) {
                return execute_dn2dn(ctx);
            }

            return execute_sp2dn(ctx);
        }

    private:
        Status execute_sp2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_eadd_fdb_sp2dn");

            auto                        t   = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>>         r   = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v   = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         fdb = t->fdb.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op  = t->op.template cast_safe<TOpBinary<T, T, T>>();

            r->validate_rwd(FormatVector::CpuDense);
            v->validate_rw(FormatVector::CpuCoo);
            fdb->validate_wd(FormatVector::CpuCoo);

            auto*       p_r      = r->template get<CpuDenseVec<T>>();
            const auto* p_v      = v->template get<CpuCooVec<T>>();
            auto*       p_fdb    = fdb->template get<CpuCooVec<T>>();
            const auto& function = op->function;

            assert(p_fdb->values == 0);

            for (uint k = 0; k < p_v->values; k++) {
                uint i     = p_v->Ai[k];
                T    prev  = p_r->Ax[i];
                p_r->Ax[i] = function(prev, p_v->Ax[k]);

                if (prev != p_r->Ax[i]) {
                    p_fdb->values += 1;
                    p_fdb->Ai.push_back(i);
                    p_fdb->Ax.push_back(p_r->Ax[i]);
                }
            }

            return Status::Ok;
        }

        Status execute_dn2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_eadd_fdb_dn2dn");

            auto                        t   = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>>         r   = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v   = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         fdb = t->fdb.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op  = t->op.template cast_safe<TOpBinary<T, T, T>>();

            r->validate_rwd(FormatVector::CpuDense);
            v->validate_rw(FormatVector::CpuDense);
            fdb->validate_wd(FormatVector::CpuDense);

            auto*       p_r      = r->template get<CpuDenseVec<T>>();
            const auto* p_v      = v->template get<CpuDenseVec<T>>();
            auto*       p_fdb    = fdb->template get<CpuDenseVec<T>>();
            const auto& function = op->function;

            const uint N          = v->get_n_rows();
            const T    fill_value = fdb->get_fill_value();

            cpu_dense_vec_fill(fdb->get_fill_value(), *p_fdb);

            for (uint i = 0; i < N; i++) {
                T prev     = p_r->Ax[i];
                p_r->Ax[i] = function(prev, p_v->Ax[i]);

                if (prev != p_r->Ax[i]) {
                    p_fdb->Ax[i] = p_r->Ax[i];
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_V_EADD_FDB_HPP
