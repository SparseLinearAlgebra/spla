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

#ifndef SPLA_CPU_M_TRANSPOSE_HPP
#define SPLA_CPU_M_TRANSPOSE_HPP

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
    class Algo_m_transpose_cpu final : public RegistryAlgo {
    public:
        ~Algo_m_transpose_cpu() override = default;

        std::string get_name() override {
            return "m_transpose";
        }

        std::string get_description() override {
            return "transpose matrix on cpu sequentially";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_m_transpose>();
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
            TIME_PROFILE_SCOPE("cpu/m_transpose_dok");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_transpose>();

            ref_ptr<TMatrix<T>>     R         = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>     M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_reduce = t->op_apply.template cast_safe<TOpUnary<T, T>>();

            R->validate_wd(FormatMatrix::CpuDok);
            M->validate_rw(FormatMatrix::CpuDok);

            CpuDok<T>*       p_dok_R    = R->template get<CpuDok<T>>();
            const CpuDok<T>* p_dok_M    = M->template get<CpuDok<T>>();
            auto&            func_apply = op_reduce->function;

            assert(p_dok_R->Ax.empty());

            p_dok_R->Ax.reserve(p_dok_M->Ax.size());

            for (const auto& entry : p_dok_M->Ax) {
                p_dok_R->Ax[{entry.first.second, entry.first.first}] = func_apply(entry.second);
            }

            p_dok_R->values = p_dok_M->values;

            return Status::Ok;
        }

        Status execute_lil(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_transpose_lil");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_transpose>();

            ref_ptr<TMatrix<T>>     R         = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>     M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_reduce = t->op_apply.template cast_safe<TOpUnary<T, T>>();

            R->validate_wd(FormatMatrix::CpuLil);
            M->validate_rw(FormatMatrix::CpuLil);

            CpuLil<T>*       p_lil_R    = R->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_M    = M->template get<CpuLil<T>>();
            auto&            func_apply = op_reduce->function;

            const uint DM = M->get_n_rows();
            const uint DN = M->get_n_cols();

            assert(M->get_n_rows() == R->get_n_cols());
            assert(M->get_n_cols() == R->get_n_rows());

            assert(p_lil_R->Ar.size() == DN);

            for (uint i = 0; i < DM; i++) {
                for (const auto [j, x] : p_lil_M->Ar[i]) {
                    p_lil_R->Ar[j].emplace_back(i, func_apply(x));
                }
            }

            p_lil_R->values = p_lil_M->values;

            return Status::Ok;
        }

        Status execute_csr(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/m_transpose_csr");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_transpose>();

            ref_ptr<TMatrix<T>>     R         = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>     M         = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpUnary<T, T>> op_reduce = t->op_apply.template cast_safe<TOpUnary<T, T>>();

            R->validate_wd(FormatMatrix::CpuCsr);
            M->validate_rw(FormatMatrix::CpuCsr);

            CpuCsr<T>*       p_csr_R    = R->template get<CpuCsr<T>>();
            const CpuCsr<T>* p_csr_M    = M->template get<CpuCsr<T>>();
            auto&            func_apply = op_reduce->function;

            const uint DM = M->get_n_rows();
            const uint DN = M->get_n_cols();

            assert(M->get_n_rows() == R->get_n_cols());
            assert(M->get_n_cols() == R->get_n_rows());

            std::vector<uint> sizes(DN + 1, 0);

            for (uint i = 0; i < DM; i++) {
                for (uint k = p_csr_M->Ap[i]; k < p_csr_M->Ap[i + 1]; k++) {
                    uint j = p_csr_M->Aj[k];
                    sizes[j] += 1;
                }
            }

            cpu_csr_resize(DN, uint(p_csr_M->Ax.size()), *p_csr_R);
            std::exclusive_scan(sizes.begin(), sizes.end(), p_csr_R->Ap.begin(), 0);

            std::vector<uint> offsets(DN, 0);

            for (uint i = 0; i < DM; i++) {
                for (uint k = p_csr_M->Ap[i]; k < p_csr_M->Ap[i + 1]; k++) {
                    uint j = p_csr_M->Aj[k];
                    T    x = p_csr_M->Ax[k];

                    p_csr_R->Aj[p_csr_R->Ap[j] + offsets[j]] = i;
                    p_csr_R->Ax[p_csr_R->Ap[j] + offsets[j]] = func_apply(x);

                    offsets[j] += 1;
                }
            }

            p_csr_R->values = p_csr_M->values;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_TRANSPOSE_HPP
