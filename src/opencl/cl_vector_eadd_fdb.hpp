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

#ifndef SPLA_CL_VECTOR_EADD_HPP
#define SPLA_CL_VECTOR_EADD_HPP

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
    class Algo_v_eadd_fdb_cl final : public RegistryAlgo {
    public:
        ~Algo_v_eadd_fdb_cl() override = default;

        std::string get_name() override {
            return "v_eadd_fdb";
        }

        std::string get_description() override {
            return "parallel vector element-wise add on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>> r = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>> v = t->v.template cast_safe<TVector<T>>();

            if (r->is_valid(FormatVector::AccDense) && v->is_valid(FormatVector::AccCoo)) {
                return execute_sp2dn(ctx);
            }
            if (r->is_valid(FormatVector::AccDense) && v->is_valid(FormatVector::AccDense)) {
                return execute_dn2dn(ctx);
            }

            return execute_sp2dn(ctx);
        }

    private:
        Status execute_sp2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cl/vector_eadd_fdb_sp2dn");

            auto                        t   = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>>         r   = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v   = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         fdb = t->fdb.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op  = t->op.template cast_safe<TOpBinary<T, T, T>>();

            std::shared_ptr<CLProgram> m_program;
            if (!ensure_kernel(op, m_program)) return Status::CompilationError;

            r->validate_rwd(FormatVector::AccDense);
            v->validate_rw(FormatVector::AccCoo);
            fdb->validate_wd(FormatVector::AccCoo);

            auto*       p_cl_r   = r->template get<CLDenseVec<T>>();
            const auto* p_cl_v   = v->template get<CLCooVec<T>>();
            auto*       p_cl_fdb = fdb->template get<CLCooVec<T>>();
            auto*       p_cl_acc = get_acc_cl();
            auto&       queue    = p_cl_acc->get_queue_default();

            const uint n = p_cl_v->values;

            if (n == 0) return Status::Ok;

            CLCounterWrapper cl_fdb_size;
            cl::Buffer       cl_fdb_i(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * n);
            cl::Buffer       cl_fdb_x(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * n);

            cl_fdb_size.set(queue, 0);

            auto kernel = m_program->make_kernel("sparse_to_dense");
            kernel.setArg(0, p_cl_r->Ax);
            kernel.setArg(1, p_cl_v->Ai);
            kernel.setArg(2, p_cl_v->Ax);
            kernel.setArg(3, cl_fdb_i);
            kernel.setArg(4, cl_fdb_x);
            kernel.setArg(5, cl_fdb_size.buffer());
            kernel.setArg(6, n);

            cl::NDRange global(align(n, p_cl_acc->get_default_wgz()));
            cl::NDRange local(p_cl_acc->get_default_wgz());
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

            p_cl_fdb->values = cl_fdb_size.get(queue);
            p_cl_fdb->Ai     = cl_fdb_i;
            p_cl_fdb->Ax     = cl_fdb_x;

            return Status::Ok;
        }

        Status execute_dn2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cl/vector_eadd_fdb_dn2dn");

            auto                        t   = ctx.task.template cast_safe<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>>         r   = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v   = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         fdb = t->fdb.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op  = t->op.template cast_safe<TOpBinary<T, T, T>>();

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op, program)) return Status::CompilationError;

            r->validate_rwd(FormatVector::AccDense);
            v->validate_rw(FormatVector::AccDense);
            fdb->validate_wd(FormatVector::AccDense);

            auto*       p_cl_r   = r->template get<CLDenseVec<T>>();
            const auto* p_cl_v   = v->template get<CLDenseVec<T>>();
            auto*       p_cl_fdb = fdb->template get<CLDenseVec<T>>();
            auto*       p_cl_acc = get_acc_cl();
            auto&       queue    = p_cl_acc->get_queue_default();

            const uint n = r->get_n_rows();

            cl_fill_value(queue, p_cl_fdb->Ax, n, fdb->get_fill_value());

            auto kernel = program->make_kernel("dense_to_dense");
            kernel.setArg(0, p_cl_r->Ax);
            kernel.setArg(1, p_cl_v->Ax);
            kernel.setArg(2, p_cl_fdb->Ax);
            kernel.setArg(3, n);

            cl::NDRange global(align(n, p_cl_acc->get_default_wgz()));
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

#endif//SPLA_CL_VECTOR_EADD_HPP
