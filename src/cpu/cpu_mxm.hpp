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

#ifndef SPLA_CPU_MXM_HPP
#define SPLA_CPU_MXM_HPP

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
    class Algo_mxm_cpu final : public RegistryAlgo {
    public:
        ~Algo_mxm_cpu() override = default;

        std::string get_name() override {
            return "mxm";
        }

        std::string get_description() override {
            return "sequential sparse matrix sparse matrix product on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/mxm");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxm>();

            auto R           = t->R.template cast_safe<TMatrix<T>>();
            auto A           = t->A.template cast_safe<TMatrix<T>>();
            auto B           = t->B.template cast_safe<TMatrix<T>>();
            auto op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            auto op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            auto init        = t->init.template cast_safe<TScalar<T>>();

            R->validate_wd(FormatMatrix::CpuLil);
            A->validate_rw(FormatMatrix::CpuLil);
            B->validate_rw(FormatMatrix::CpuLil);

            CpuLil<T>*       p_lil_R = R->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_A = A->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_B = B->template get<CpuLil<T>>();

            auto& func_multiply = op_multiply->function;
            auto& func_add      = op_add->function;

            auto DM = R->get_n_rows();
            auto DN = R->get_n_cols();
            auto I  = init->get_value();

            std::vector<T> R_tmp(DN, I);

            for (uint row_R = 0; row_R < DM; row_R++) {
                const auto& A_lst = p_lil_A->Ar[row_R];
                auto&       R_lst = p_lil_R->Ar[row_R];

                assert(R_lst.empty());

                std::fill(R_tmp.begin(), R_tmp.end(), I);

                for (const typename CpuLil<T>::Entry& entry_A : A_lst) {
                    const uint i       = entry_A.first;
                    const T    value_A = entry_A.second;

                    const auto& B_lst = p_lil_B->Ar[i];

                    for (const typename CpuLil<T>::Entry& entry_B : B_lst) {
                        const uint j       = entry_B.first;
                        const T    value_B = entry_B.second;

                        R_tmp[j] = func_add(R_tmp[j], func_multiply(value_A, value_B));
                    }
                }

                for (uint col_R = 0; col_R < DN; col_R++) {
                    if (R_tmp[col_R] != I) {
                        R_lst.emplace_back(col_R, R_tmp[col_R]);
                    }
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_MXM_HPP
