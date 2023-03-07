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

#ifndef SPLA_CL_VECTOR_HPP
#define SPLA_CL_VECTOR_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_counter.hpp>
#include <opencl/cl_fill.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/generated/auto_vector_eadd.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_eadd_cl final : public RegistryAlgo {
    public:
        ~Algo_v_eadd_cl() override = default;

        std::string get_name() override {
            return "v_eadd";
        }

        std::string get_description() override {
            return "parallel vector element-wise add on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_eadd>();
            ref_ptr<TVector<T>> u = t->u.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (u->is_valid(FormatVector::AccDense) && v->is_valid(FormatVector::AccDense)) {
                return execute_dn2dn(ctx);
            }

            return execute_dn2dn(ctx);
        }

    private:
        Status execute_dn2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cl/vector_eadd_dn2dn");

            auto                        t  = ctx.task.template cast_safe<ScheduleTask_v_eadd>();
            ref_ptr<TVector<T>>         r  = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         u  = t->u.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v  = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op = t->op.template cast_safe<TOpBinary<T, T, T>>();

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op, program)) return Status::CompilationError;

            r->validate_wd(FormatVector::AccDense);
            u->validate_rw(FormatVector::AccDense);
            v->validate_rw(FormatVector::AccDense);

            auto*       p_cl_r   = r->template get<CLDenseVec<T>>();
            const auto* p_cl_u   = u->template get<CLDenseVec<T>>();
            const auto* p_cl_v   = v->template get<CLDenseVec<T>>();
            auto*       p_cl_acc = get_acc_cl();
            auto&       queue    = p_cl_acc->get_queue_default();

            const uint n = r->get_n_rows();

            auto kernel = program->make_kernel("dense_to_dense");
            kernel.setArg(0, p_cl_r->Ax);
            kernel.setArg(1, p_cl_u->Ax);
            kernel.setArg(2, p_cl_v->Ax);
            kernel.setArg(3, n);

            cl::NDRange global(p_cl_acc->get_default_wgz() * div_up_clamp(n, p_cl_acc->get_default_wgz(), 1u, 1024u));
            cl::NDRange local(p_cl_acc->get_default_wgz());
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op, std::shared_ptr<CLProgram>& program) {
            CLProgramBuilder program_builder;
            program_builder
                    .set_name("vector_eadd")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op.template as<OpBinary>())
                    .set_source(source_vector_eadd)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_HPP
