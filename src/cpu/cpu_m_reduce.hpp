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

#ifndef SPLA_CPU_M_REDUCE_HPP
#define SPLA_CPU_M_REDUCE_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <algorithm>

namespace spla {

    template<typename T>
    class Algo_m_reduce_cpu final : public RegistryAlgo {
    public:
        ~Algo_m_reduce_cpu() override = default;

        std::string get_name() override {
            return "m_reduce";
        }

        std::string get_description() override {
            return "reduce matrix on cpu sequentially";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();
            auto M = t->M.template cast_safe<TMatrix<T>>();

            if (M->is_valid(FormatMatrix::CpuDok)) {
                return execute_dok(ctx);
            }
            if (M->is_valid(FormatMatrix::CpuLil)) {
                return execute_lil(ctx);
            }
            if (M->is_valid(FormatMatrix::CpuCsr)) {
                return execute_csr(ctx);
            }

            return execute_dok(ctx);
        }

    private:
        Status execute_dok(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_reduce_dok");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();

            ref_ptr<TScalar<T>>         r         = t->r.template cast_safe<TScalar<T>>();
            ref_ptr<TScalar<T>>         s         = t->s.template cast_safe<TScalar<T>>();
            ref_ptr<TMatrix<T>>         M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            M->validate_rw(FormatMatrix::CpuDok);

            const CpuDok<T>* p_dok_M     = M->template get<CpuDok<T>>();
            auto&            func_reduce = op_reduce->function;

            T result = s->get_value();

            for (const auto& entry : p_dok_M->Ax) {
                result = func_reduce(result, entry.second);
            }

            r->get_value() = result;

            return Status::Ok;
        }

        Status execute_lil(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_reduce_lil");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();

            ref_ptr<TScalar<T>>         r         = t->r.template cast_safe<TScalar<T>>();
            ref_ptr<TScalar<T>>         s         = t->s.template cast_safe<TScalar<T>>();
            ref_ptr<TMatrix<T>>         M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            M->validate_rw(FormatMatrix::CpuLil);

            const CpuLil<T>* p_lil_M     = M->template get<CpuLil<T>>();
            auto&            func_reduce = op_reduce->function;

            T result = s->get_value();

            for (const auto& row : p_lil_M->Ar) {
                for (const auto& entry : row) {
                    result = func_reduce(result, entry.second);
                }
            }

            r->get_value() = result;

            return Status::Ok;
        }

        Status execute_csr(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_reduce_csr");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();

            ref_ptr<TScalar<T>>         r         = t->r.template cast_safe<TScalar<T>>();
            ref_ptr<TScalar<T>>         s         = t->s.template cast_safe<TScalar<T>>();
            ref_ptr<TMatrix<T>>         M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            M->validate_rw(FormatMatrix::CpuCsr);

            const CpuCsr<T>* p_csr_M     = M->template get<CpuCsr<T>>();
            auto&            func_reduce = op_reduce->function;

            T result = s->get_value();

            for (const auto v : p_csr_M->Ax) {
                result = func_reduce(result, v);
            }

            r->get_value() = result;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_REDUCE_HPP
