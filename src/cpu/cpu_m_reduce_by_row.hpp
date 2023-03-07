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

#ifndef SPLA_CPU_M_REDUCE_BY_ROW_HPP
#define SPLA_CPU_M_REDUCE_BY_ROW_HPP

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
    class Algo_m_reduce_by_row final : public RegistryAlgo {
    public:
        ~Algo_m_reduce_by_row() override = default;

        std::string get_name() override {
            return "m_reduce_by_row";
        }

        std::string get_description() override {
            return "reduce matrix by row on cpu sequentially";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce_by_row>();
            auto M = t->M.template cast_safe<TMatrix<T>>();

            if (M->is_valid(FormatMatrix::CpuDok)) {
                return execute_dok(ctx);
            }

            return execute_dok(ctx);
        }

    private:
        Status execute_dok(const DispatchContext& ctx) {
            auto t         = ctx.task.template cast_safe<ScheduleTask_m_reduce_by_row>();
            auto r         = t->r.template cast_safe<TVector<T>>();
            auto M         = t->M.template cast_safe<TMatrix<T>>();
            auto op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();
            auto init      = t->init.template cast_safe<TScalar<T>>();

            r->validate_wd(FormatVector::CpuDense);
            M->validate_rw(FormatMatrix::CpuDok);

            CpuDenseVec<T>*  p_dense_r = r->template get<CpuDenseVec<T>>();
            const CpuDok<T>* p_dok_M   = M->template get<CpuDok<T>>();

            std::fill(p_dense_r->Ax.begin(), p_dense_r->Ax.end(), init->get_value());

            auto& func_reduce = op_reduce->function;

            for (const auto& entry : p_dok_M->Ax) {
                const uint i = entry.first.first;
                const T    x = entry.second;

                p_dense_r->Ax[i] = func_reduce(p_dense_r->Ax[i], x);
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_M_REDUCE_BY_ROW_HPP
