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

#ifndef SPLA_CPU_V_MAP_HPP
#define SPLA_CPU_V_MAP_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <cstring>

namespace spla {

    template<typename T>
    class Algo_v_map_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_map_cpu() override = default;

        std::string get_name() override {
            return "v_map";
        }

        std::string get_description() override {
            return "sequential vector map on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_map>();
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
            TIME_PROFILE_SCOPE("cpu/vector_map_sparse");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_map>();

            auto r  = t->r.template cast_safe<TVector<T>>();
            auto v  = t->v.template cast_safe<TVector<T>>();
            auto op = t->op.template cast_safe<TOpUnary<T, T>>();

            r->validate_wd(FormatVector::CpuCoo);
            v->validate_rw(FormatVector::CpuCoo);
            auto*       p_sparse_r = r->template get<CpuCooVec<T>>();
            const auto* p_sparse_v = v->template get<CpuCooVec<T>>();
            const auto& function   = op->function;

            const uint N = p_sparse_v->values;
            cpu_coo_vec_resize(N, *p_sparse_r);

            if (N > 0) {
                std::memcpy(p_sparse_r->Ai.data(), p_sparse_v->Ai.data(), sizeof(uint) * N);

                for (uint i = 0; i < N; i += 1) {
                    p_sparse_r->Ax[i] = function(p_sparse_v->Ax[i]);
                }
            }

            return Status::Ok;
        }

        Status execute_dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/vector_map_dense");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_map>();

            auto r  = t->r.template cast_safe<TVector<T>>();
            auto v  = t->v.template cast_safe<TVector<T>>();
            auto op = t->op.template cast_safe<TOpUnary<T, T>>();

            r->validate_wd(FormatVector::CpuDense);
            v->validate_rw(FormatVector::CpuDense);
            auto*       p_dense_r = r->template get<CpuDenseVec<T>>();
            const auto* p_dense_v = v->template get<CpuDenseVec<T>>();
            const auto& function  = op->function;

            const uint N = r->get_n_rows();

            for (uint i = 0; i < N; i += 1) {
                p_dense_r->Ax[i] = function(p_dense_v->Ax[i]);
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_V_MAP_HPP
