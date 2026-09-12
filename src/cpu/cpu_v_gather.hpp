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

#ifndef SPLA_CPU_V_GATHER_HPP
#define SPLA_CPU_V_GATHER_HPP

#include <iostream>
#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_gather_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_gather_cpu() override = default;

        std::string get_name() override {
            return "v_gather";
        }

        std::string get_description() override {
            return "sequential vector gather operation";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/v_gather");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_gather>();
            ref_ptr<TVector<T>> r = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>> source = t->source.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T_UINT>> indices = t->indices.template cast_safe<TVector<T_UINT>>();

            r->validate_wd(FormatVector::CpuDense);
            source->validate_rw(FormatVector::CpuDense);
            indices->validate_rw(FormatVector::CpuDense);

            auto* p_r       = r->template get<CpuDenseVec<T>>();
            auto* p_source  = source->template get<CpuDenseVec<T>>();
            auto* p_indices = indices->template get<CpuDenseVec<T_UINT>>();

            const uint N = r->get_n_rows();

            for (uint k = 0; k < N; ++k) {
                const uint idx = p_indices->Ax[k];
                p_r->Ax[k]     = p_source->Ax[idx];
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_V_GATHER_HPP