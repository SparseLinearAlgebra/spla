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

#ifndef SPLA_CL_M_REDUCE_HPP
#define SPLA_CL_M_REDUCE_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_reduce.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_m_reduce_cl final : public RegistryAlgo {
    public:
        ~Algo_m_reduce_cl() override = default;

        std::string get_name() override {
            return "m_reduce";
        }

        std::string get_description() override {
            return "parallel matrix reduction on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();
            ref_ptr<TMatrix<T>> M = t->M.template cast_safe<TMatrix<T>>();

            if (M->is_valid(FormatMatrix::AccCsr)) {
                return execute_csr(ctx);
            }
            if (M->is_valid(FormatMatrix::CpuCsr)) {
                return execute_csr(ctx);
            }

            return execute_csr(ctx);
        }

    private:
        Status execute_csr(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/m_reduce_csr");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_reduce>();

            auto r         = t->r.template cast_safe<TScalar<T>>();
            auto s         = t->s.template cast_safe<TScalar<T>>();
            auto M         = t->M.template cast_safe<TMatrix<T>>();
            auto op_reduce = t->op_reduce.template cast_safe<TOpBinary<T, T, T>>();

            M->validate_rw(FormatMatrix::AccCsr);

            const auto* p_cl_csr = M->template get<CLCsr<T>>();
            auto*       p_cl_acc = get_acc_cl();
            auto&       queue    = p_cl_acc->get_queue_default();

            cl_reduce<T>(queue, p_cl_csr->Ax, p_cl_csr->values, s->get_value(), op_reduce, r->get_value());

            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CL_M_REDUCE_HPP
