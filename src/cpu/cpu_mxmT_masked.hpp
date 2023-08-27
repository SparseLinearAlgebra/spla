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

#ifndef SPLA_CPU_MXMT_MASKED_HPP
#define SPLA_CPU_MXMT_MASKED_HPP

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
    class Algo_mxmT_masked_cpu final : public RegistryAlgo {
    public:
        ~Algo_mxmT_masked_cpu() override = default;

        std::string get_name() override {
            return "mxmT_masked";
        }

        std::string get_description() override {
            return "sequential masked matrix matrix-transposed product on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/mxmT_masked");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxmT_masked>();

            auto R           = t->R.template cast_safe<TMatrix<T>>();
            auto mask        = t->mask.template cast_safe<TMatrix<T>>();
            auto A           = t->A.template cast_safe<TMatrix<T>>();
            auto B           = t->B.template cast_safe<TMatrix<T>>();
            auto op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            auto op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            auto op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            auto init        = t->init.template cast_safe<TScalar<T>>();

            R->validate_wd(FormatMatrix::CpuLil);
            A->validate_rw(FormatMatrix::CpuLil);
            B->validate_rw(FormatMatrix::CpuLil);
            mask->validate_rw(FormatMatrix::CpuLil);

            CpuLil<T>*       p_lil_R    = R->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_A    = A->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_B    = B->template get<CpuLil<T>>();
            const CpuLil<T>* p_lil_mask = mask->template get<CpuLil<T>>();

            auto& func_multiply = op_multiply->function;
            auto& func_add      = op_add->function;
            auto& func_select   = op_select->function;

            auto DM = R->get_n_rows();
            auto I  = init->get_value();

            for (uint row_R = 0; row_R < DM; row_R++) {
                const auto& mask_lst = p_lil_mask->Ar[row_R];
                const auto& A_lst    = p_lil_A->Ar[row_R];
                auto&       R_lst    = p_lil_R->Ar[row_R];

                assert(R_lst.empty());

                for (const typename CpuLil<T>::Entry& entry_mask : mask_lst) {
                    const uint mask_i = entry_mask.first;
                    const T    mask_x = entry_mask.second;

                    T r = I;

                    if (func_select(mask_x)) {
                        const auto& B_lst = p_lil_B->Ar[mask_i];

                        auto       A_it  = A_lst.begin();
                        auto       B_it  = B_lst.begin();
                        const auto A_end = A_lst.end();
                        const auto B_end = B_lst.end();

                        while (A_it != A_end && B_it != B_end) {
                            if (A_it->first == B_it->first) {
                                r = func_add(r, func_multiply(A_it->second, B_it->second));
                                ++A_it;
                                ++B_it;
                            } else if (A_it->first < B_it->first) {
                                ++A_it;
                            } else {
                                ++B_it;
                            }
                        }
                    }

                    R_lst.emplace_back(mask_i, r);
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_MXMT_MASKED_HPP
