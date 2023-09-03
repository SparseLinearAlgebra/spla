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

#ifndef SPLA_CPU_M_EXTRACT_ROW_HPP
#define SPLA_CPU_M_EXTRACT_ROW_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <algorithm>
#include <numeric>

namespace spla {

    template<typename T>
    class Algo_m_extract_row_cpu final : public RegistryAlgo {
    public:
        ~Algo_m_extract_row_cpu() override = default;

        std::string get_name() override {
            return "m_extract_row";
        }

        std::string get_description() override {
            return "extract matrix row on cpu sequentially";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_m_extract_row>();
            auto M = t->M.template cast_safe<TMatrix<T>>();

            if (M->is_valid(FormatMatrix::CpuCsr)) {
                return execute_csr(ctx);
            }
            if (M->is_valid(FormatMatrix::CpuLil)) {
                return execute_lil(ctx);
            }
            if (M->is_valid(FormatMatrix::CpuDok)) {
                return execute_dok(ctx);
            }

            return execute_csr(ctx);
        }

    private:
        Status execute_dok(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_extract_row_dok");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_extract_row>();

            ref_ptr<TVector<T>>     r        = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>     M        = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_apply = t->op_apply.template cast_safe<TOpUnary<T, T>>();
            uint                    index    = t->index;

            r->validate_wd(FormatVector::CpuCoo);
            M->validate_rw(FormatMatrix::CpuDok);

            CpuCooVec<T>*    p_coo_r    = r->template get<CpuCooVec<T>>();
            const CpuDok<T>* p_dok_M    = M->template get<CpuDok<T>>();
            auto&            func_apply = op_apply->function;

            for (const auto [key, value] : p_dok_M->Ax) {
                if (key.first == index) {
                    p_coo_r->values += 1;
                    p_coo_r->Ai.push_back(key.second);
                    p_coo_r->Ax.push_back(func_apply(value));
                }
            }

            cpu_coo_vec_sort(*p_coo_r);

            return Status::Ok;
        }

        Status execute_lil(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_extract_row_lil");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_extract_row>();

            ref_ptr<TVector<T>>     r        = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>     M        = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_apply = t->op_apply.template cast_safe<TOpUnary<T, T>>();
            uint                    index    = t->index;

            r->validate_wd(FormatVector::CpuCoo);
            M->validate_rw(FormatMatrix::CpuLil);

            CpuCooVec<T>*    p_coo_r    = r->template get<CpuCooVec<T>>();
            const CpuLil<T>* p_lil_M    = M->template get<CpuLil<T>>();
            auto&            func_apply = op_apply->function;

            assert(index < M->get_n_rows());

            p_coo_r->Ai.reserve(p_lil_M->Ar[index].size());
            p_coo_r->Ax.reserve(p_lil_M->Ar[index].size());

            for (const auto [key, value] : p_lil_M->Ar[index]) {
                p_coo_r->values += 1;
                p_coo_r->Ai.push_back(key);
                p_coo_r->Ax.push_back(func_apply(value));
            }

            return Status::Ok;
        }

        Status execute_csr(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_extract_row_csr");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_extract_row>();

            ref_ptr<TVector<T>>     r        = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>     M        = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_apply = t->op_apply.template cast_safe<TOpUnary<T, T>>();
            uint                    index    = t->index;

            r->validate_wd(FormatVector::CpuCoo);
            M->validate_rw(FormatMatrix::CpuCsr);

            CpuCooVec<T>*    p_coo_r    = r->template get<CpuCooVec<T>>();
            const CpuCsr<T>* p_csr_M    = M->template get<CpuCsr<T>>();
            auto&            func_apply = op_apply->function;

            assert(index < M->get_n_rows());

            const uint start = p_csr_M->Ap[index];
            const uint end   = p_csr_M->Ap[index + 1];
            const uint count = end - start;

            p_coo_r->Ai.reserve(count);
            p_coo_r->Ax.reserve(count);

            for (uint k = start; k < end; k++) {
                p_coo_r->values += 1;
                p_coo_r->Ai.push_back(p_csr_M->Aj[k]);
                p_coo_r->Ax.push_back(func_apply(p_csr_M->Ax[k]));
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_EXTRACT_ROW_HPP
