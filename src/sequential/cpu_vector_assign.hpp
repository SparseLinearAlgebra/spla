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

#ifndef SPLA_CPU_VECTOR_ASSIGN_HPP
#define SPLA_CPU_VECTOR_ASSIGN_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

namespace spla {

    template<typename T>
    class Algo_v_assign_masked_cpu final : public RegistryAlgo {
    public:
        ~Algo_v_assign_masked_cpu() override = default;

        std::string get_name() override {
            return "v_assign_masked";
        }

        std::string get_description() override {
            return "sequential masked vector assignment";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast<ScheduleTask_v_assign_masked>();

            auto r         = t->r.template cast<TVector<T>>();
            auto mask      = t->mask.template cast<TVector<T>>();
            auto value     = t->value.template cast<TScalar<T>>();
            auto op_assign = t->op_assign.template cast<TOpBinary<T, T, T>>();

            auto skip_value   = T();
            auto assign_value = value->get_value();

            r->ensure_dense_format();
            mask->ensure_dense_format();

            auto*       p_r_dense    = r->template get_dec_p<CpuDenseVec<T>>();
            const auto* p_mask_dense = mask->template get_dec_p<CpuDenseVec<T>>();
            const auto& function     = op_assign->function;

            uint N = r->get_n_rows();

            for (uint i = 0; i < N; ++i) {
                if (p_mask_dense->Ax[i] != skip_value) {
                    p_r_dense->Ax[i] = function(p_r_dense->Ax[i], assign_value);
                }
            }

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_VECTOR_ASSIGN_HPP
