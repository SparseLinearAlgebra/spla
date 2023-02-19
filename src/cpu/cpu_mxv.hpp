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

#ifndef SPLA_CPU_MXV_HPP
#define SPLA_CPU_MXV_HPP

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
    class Algo_mxv_masked_cpu final : public RegistryAlgo {
    public:
        ~Algo_mxv_masked_cpu() override = default;

        std::string get_name() override {
            return "mxv_masked";
        }

        std::string get_description() override {
            return "sequential masked matrix-vector product on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("cpu/mxv");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxv_masked>();

            auto r           = t->r.template cast_safe<TVector<T>>();
            auto mask        = t->mask.template cast_safe<TVector<T>>();
            auto M           = t->M.template cast_safe<TMatrix<T>>();
            auto v           = t->v.template cast_safe<TVector<T>>();
            auto op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            auto op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            auto op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            auto init        = t->init.template cast_safe<TScalar<T>>();

            const uint DM       = M->get_n_rows();
            const T    sum_init = init->get_value();

            r->validate_wd(FormatVector::CpuDense);
            mask->validate_rw(FormatVector::CpuDense);
            v->validate_rw(FormatVector::CpuDense);
            M->validate_rw(FormatMatrix::CpuLil);

            CpuDenseVec<T>*       p_dense_r    = r->template get<CpuDenseVec<T>>();
            const CpuDenseVec<T>* p_dense_mask = mask->template get<CpuDenseVec<T>>();
            const CpuDenseVec<T>* p_dense_v    = v->template get<CpuDenseVec<T>>();
            const CpuLil<T>*      p_lil_M      = M->template get<CpuLil<T>>();
            auto                  early_exit   = t->get_desc_or_default()->get_early_exit();

            auto& func_multiply = op_multiply->function;
            auto& func_add      = op_add->function;
            auto& func_select   = op_select->function;

            for (uint i = 0; i < DM; ++i) {
                T sum = sum_init;

                if (func_select(p_dense_mask->Ax[i])) {
                    const auto& row = p_lil_M->Ar[i];

                    for (const auto& j_x : row) {
                        const uint j = j_x.first;
                        sum          = func_add(sum, func_multiply(j_x.second, p_dense_v->Ax[j]));

                        if ((sum != sum_init) && early_exit) break;
                    }
                }

                p_dense_r->Ax[i] = sum;
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_MXV_HPP
