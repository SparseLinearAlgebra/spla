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

#ifndef SPLA_CPU_KRON_HPP
#define SPLA_CPU_KRON_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_kron_cpu final : public RegistryAlgo {
    public:
        ~Algo_kron_cpu() override = default;

        std::string get_name() override {
            return "kron";
        }

        std::string get_description() override {
            return "sequential sparse matrix kronecker product on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/kron");

            auto t = ctx.task.template cast_safe<ScheduleTask_kron>();

            auto R           = t->R.template cast_safe<TMatrix<T>>();
            auto A           = t->A.template cast_safe<TMatrix<T>>();
            auto B           = t->B.template cast_safe<TMatrix<T>>();
            auto op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();

            R->validate_wd(FormatMatrix::CpuLil);
            A->validate_rw(FormatMatrix::CpuLil);
            B->validate_rw(FormatMatrix::CpuLil);

            CpuLil<T>*       p_lil_R = R->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_A = A->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_B = B->template get<CpuLil<T>>();

            auto& func_multiply = op_multiply->function;

            auto D_AM = A->get_n_rows();
            auto D_AN = A->get_n_cols();
            auto D_BM = B->get_n_rows();
            auto D_BN = B->get_n_cols();

            assert(R->get_n_rows() == D_AM * D_BM);
            assert(R->get_n_cols() == D_AN * D_BN);

            assert(p_lil_R->Ar.size() == D_AM * D_BM);

            for (uint i_A = 0; i_A < D_AM; i_A++) {
                const auto& row_A = p_lil_A->Ar[i_A];

                for (const auto [j_A, x_A] : row_A) {

                    for (uint i_B = 0; i_B < D_BM; i_B++) {
                        const auto& row_B = p_lil_B->Ar[i_B];

                        auto& row_R = p_lil_R->Ar[i_A * D_BM + i_B];
                        row_R.reserve(row_A.size() * row_B.size());

                        for (const auto [j_B, x_B] : row_B) {
                            row_R.emplace_back(j_A * D_BN + j_B, func_multiply(x_A, x_B));
                        }
                    }
                }
            }

            p_lil_R->values = p_lil_A->values * p_lil_B->values;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_KRON_HPP
