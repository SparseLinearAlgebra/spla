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

            R->validate_wd(FormatMatrix::CpuCsc);
            A->validate_rw(FormatMatrix::CpuCsc);
            B->validate_rw(FormatMatrix::CpuCsc);
            mask->validate_rw(FormatMatrix::CpuCsc);

            auto& func_multiply = op_multiply->function;
            auto& func_add      = op_add->function;
            auto& func_select   = op_select->function;

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_MXMT_MASKED_HPP
