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

#ifndef SPLA_CL_VECTOR_MAP_HPP
#define SPLA_CL_VECTOR_MAP_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_map.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_map_cl final : public RegistryAlgo {
    public:
        ~Algo_v_map_cl() override = default;

        std::string get_name() override {
            return "v_map";
        }

        std::string get_description() override {
            return "parallel vector map on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_map>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (v->is_valid(FormatVector::AccCoo)) {
                return execute_sp(ctx);
            }
            if (v->is_valid(FormatVector::AccDense)) {
                return execute_dn(ctx);
            }
            if (v->is_valid(FormatVector::CpuCoo)) {
                return execute_sp(ctx);
            }
            if (v->is_valid(FormatVector::CpuDense)) {
                return execute_dn(ctx);
            }

            return execute_sp(ctx);
        }

    private:
        Status execute_dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vector_map_dense");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_map>();

            auto r  = t->r.template cast_safe<TVector<T>>();
            auto v  = t->v.template cast_safe<TVector<T>>();
            auto op = t->op.template cast_safe<TOpUnary<T, T>>();

            r->validate_wd(FormatVector::AccDense);
            v->validate_rw(FormatVector::AccDense);

            auto*       p_cl_dense_r = r->template get<CLDenseVec<T>>();
            const auto* p_cl_dense_v = v->template get<CLDenseVec<T>>();
            auto*       p_cl_acc     = get_acc_cl();
            auto&       queue        = p_cl_acc->get_queue_default();

            cl_map(queue, p_cl_dense_v->Ax, p_cl_dense_r->Ax, r->get_n_rows(), op);

            return Status::Ok;
        }

        Status execute_sp(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vector_map_sparse");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_map>();

            auto r  = t->r.template cast_safe<TVector<T>>();
            auto v  = t->v.template cast_safe<TVector<T>>();
            auto op = t->op.template cast_safe<TOpUnary<T, T>>();

            r->validate_wd(FormatVector::AccCoo);
            v->validate_rw(FormatVector::AccCoo);

            auto*       p_cl_coo_r = r->template get<CLCooVec<T>>();
            const auto* p_cl_coo_v = v->template get<CLCooVec<T>>();
            auto*       p_cl_acc   = get_acc_cl();
            auto&       queue      = p_cl_acc->get_queue_default();

            const uint N = p_cl_coo_v->values;

            cl_coo_vec_resize(N, *p_cl_coo_r);

            queue.enqueueCopyBuffer(p_cl_coo_v->Ai, p_cl_coo_r->Ai, 0, 0, sizeof(uint) * N);
            cl_map(queue, p_cl_coo_v->Ax, p_cl_coo_r->Ax, N, op);

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_MAP_HPP
