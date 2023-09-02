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

#ifndef SPLA_CPU_M_EADD_HPP
#define SPLA_CPU_M_EADD_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>

namespace spla {

    template<typename T>
    class Algo_m_eadd_cpu final : public RegistryAlgo {
    public:
        ~Algo_m_eadd_cpu() override = default;

        std::string get_name() override {
            return "m_eadd";
        }

        std::string get_description() override {
            return "sequential element-wise add matrix operation";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_m_eadd>();
            ref_ptr<TMatrix<T>> A = t->A.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>> B = t->B.template cast_safe<TMatrix<T>>();

            if (A->is_valid(FormatMatrix::CpuLil) && B->is_valid(FormatMatrix::CpuLil)) {
                return execute_lil(ctx);
            }

            return execute_lil(ctx);
        }

    private:
        Status execute_lil(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cpu/matrix_eadd_lil");

            auto                        t  = ctx.task.template cast_safe<ScheduleTask_m_eadd>();
            ref_ptr<TMatrix<T>>         R  = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         A  = t->A.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         B  = t->B.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op = t->op.template cast_safe<TOpBinary<T, T, T>>();

            R->validate_wd(FormatMatrix::CpuLil);
            A->validate_rw(FormatMatrix::CpuLil);
            B->validate_rw(FormatMatrix::CpuLil);

            auto*       p_R      = R->template get<CpuLil<T>>();
            const auto* p_A      = A->template get<CpuLil<T>>();
            const auto* p_B      = B->template get<CpuLil<T>>();
            const auto& function = op->function;

            const uint N            = R->get_n_rows();
            const auto fill_value_R = R->get_fill_value();

            p_R->values = 0;

            for (uint i = 0; i < N; i++) {
                auto&       row_R = p_R->Ar[i];
                const auto& row_A = p_A->Ar[i];
                const auto& row_B = p_B->Ar[i];

                auto iter_A = row_A.begin();
                auto iter_B = row_B.begin();

                auto end_A = row_A.end();
                auto end_B = row_B.end();

                while (iter_A != end_A && iter_B != end_B) {
                    const auto [i_A, x_A] = *iter_A;
                    const auto [i_B, x_B] = *iter_B;

                    T    r;
                    uint j;

                    if (i_A < i_B) {
                        r = x_A;
                        j = i_A;
                        ++iter_A;
                    } else if (i_B < i_A) {
                        r = x_B;
                        j = i_B;
                        ++iter_B;
                    } else {
                        r = function(x_A, x_B);
                        j = i_A;
                        ++iter_A;
                        ++iter_B;
                    }

                    if (r != fill_value_R) {
                        row_R.emplace_back(j, r);
                        p_R->values += 1;
                    }
                }
                while (iter_A != end_A) {
                    const auto [i_A, x_A] = *iter_A;
                    auto r                = x_A;
                    if (r != fill_value_R) {
                        row_R.emplace_back(i_A, r);
                        p_R->values += 1;
                    }
                    ++iter_A;
                }
                while (iter_B != end_B) {
                    const auto [i_B, x_B] = *iter_B;
                    auto r                = x_B;
                    if (r != fill_value_R) {
                        row_R.emplace_back(i_B, r);
                        p_R->values += 1;
                    }
                    ++iter_B;
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_EADD_HPP
