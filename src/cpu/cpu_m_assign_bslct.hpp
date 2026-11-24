/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
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

#ifndef SPLA_CPU_M_ASSIGN_BSLCT_HPP
#define SPLA_CPU_M_ASSIGN_BSLCT_HPP

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
    class Algo_m_assign_bslct_masked_cpu final : public RegistryAlgo {
    public:
        ~Algo_m_assign_bslct_masked_cpu() override = default;

        std::string get_name() override {
            return "m_assign_bslct_masked";
        }

        std::string get_description() override {
            return "sequential masked matrix assignment";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/m_assign_bslct");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_assign_bslct_masked>();

            auto r             = t->r.template cast_safe<TMatrix<T>>();
            auto mask          = t->mask.template cast_safe<TVector<T>>();
            auto value         = t->value.template cast_safe<TScalar<T>>();
            auto op_assign     = t->op_assign.template cast_safe<TOpBinary<T, T, T>>();
            auto op_select_bin = t->op_select_bin.template cast_safe<TOpSelectBinary<T>>();

            auto assign_val = value->get_value();

            r->validate_rwd(FormatMatrix::CpuCsr);
            mask->validate_rw(FormatVector::CpuDense);

            auto*       p_r_csr         = r->template get<CpuCsr<T>>();
            const auto* p_mask          = mask->template get<CpuDenseVec<T>>();
            const auto& func_assign     = op_assign->function;
            const auto& func_select_bin = op_select_bin->function;

            uint N = r->get_n_rows();

            for (uint row = 0; row < N; ++row) {
                const T    mask_row = p_mask->Ax[row];
                const uint start    = p_r_csr->Ap[row];
                const uint end      = p_r_csr->Ap[row + 1];

                for (uint idx = start; idx < end; ++idx) {
                    const uint col = p_r_csr->Aj[idx];
                    if (func_select_bin(mask_row, p_mask->Ax[col])) {
                        p_r_csr->Ax[idx] = func_assign(p_r_csr->Ax[idx], assign_val);
                    }
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_ASSIGN_BSLCT_HPP
