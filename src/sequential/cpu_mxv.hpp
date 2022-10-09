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

#ifndef SPLA_CPU_MXV_HPP
#define SPLA_CPU_MXV_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_mxv_masked_mc final : public RegistryAlgo {
    public:
        ~Algo_mxv_masked_mc() override = default;

        std::string get_name() override {
            return "mxv_masked_mc";
        }

        std::string get_description() override {
            return "sequential matrix-vector with complement-mask product on cpu";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast<ScheduleTask_mxv_masked>();

            auto r           = t->r.template cast<TVector<T>>();
            auto mask        = t->mask.template cast<TVector<T>>();
            auto M           = t->M.template cast<TMatrix<T>>();
            auto v           = t->v.template cast<TVector<T>>();
            auto op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            auto op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();

            uint DM         = M->get_n_rows();
            T    skip_value = T();

            r->ensure_dense_format();
            mask->ensure_dense_format();
            v->ensure_dense_format();
            M->ensure_lil_format();

            CpuDenseVec<T>*       p_dense_r    = r->template get_dec_p<CpuDenseVec<T>>();
            const CpuDenseVec<T>* p_dense_mask = mask->template get_dec_p<CpuDenseVec<T>>();
            const CpuDenseVec<T>* p_dense_v    = v->template get_dec_p<CpuDenseVec<T>>();
            const CpuLil<T>*      p_lil_M      = M->template get_dec_p<CpuLil<T>>();

            auto& func_multiply = op_multiply->function;
            auto& func_add      = op_add->function;

            for (uint i = 0; i < DM; ++i) {
                if (p_dense_mask->Ax[i] == skip_value) {
                    T           sum = p_dense_r->Ax[i];
                    const auto& row = p_lil_M->Ar[i];

                    for (const auto& j_x : row) {
                        sum = func_add(sum, func_multiply(j_x.second, p_dense_v->Ax[j_x.first]));
                    }

                    p_dense_r->Ax[i] = sum;
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_MXV_HPP
