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

#ifndef SPLA_CPU_VECTOR_COUNT_NZ_HPP
#define SPLA_CPU_VECTOR_COUNT_NZ_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_count_nz_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_count_nz_cpu() override = default;

        std::string get_name() override {
            return "v_count_nz";
        }

        std::string get_description() override {
            return "sequential count nz";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v = t->v.template cast<TVector<T>>();

            if (v->is_valid(Format::CpuDokVec))
                return execute_dok(ctx);
            if (v->is_valid(Format::CpuCooVec))
                return execute_coo(ctx);
            if (v->is_valid(Format::CpuDenseVec))
                return execute_dense(ctx);

            return execute_coo(ctx);
        }

    private:
        Status execute_dok(const DispatchContext& ctx) {
            auto                t     = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v     = t->v.template cast<TVector<T>>();
            CpuDokVec<T>*       dec_v = v->template get<CpuDokVec<T>>();

            t->r->set_uint(dec_v->values);

            return Status::Ok;
        }
        Status execute_coo(const DispatchContext& ctx) {
            auto                t     = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v     = t->v.template cast<TVector<T>>();
            CpuCooVec<T>*       dec_v = v->template get<CpuCooVec<T>>();

            t->r->set_uint(dec_v->values);

            return Status::Ok;
        }
        Status execute_dense(const DispatchContext& ctx) {
            auto                t     = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v     = t->v.template cast<TVector<T>>();
            CpuDenseVec<T>*     dec_v = v->template get<CpuDenseVec<T>>();

            uint values = 0;

            for (uint i = 0; i < v->get_n_rows(); i++) {
                if (dec_v->Ax[i] != 0) {
                    values += 1;
                }
            }

            t->r->set_uint(values);

            return Status::Ok;
        }
    };

}// namespace spla


#endif//SPLA_CPU_VECTOR_COUNT_NZ_HPP