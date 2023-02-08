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
            auto                t = ctx.task.template cast<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>> r = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>> v = t->v.template cast<TVector<T>>();

            if (r->is_valid(Format::CLDenseVec) && v->is_valid(Format::CLCooVec)) {
                return execute_sp2dn(ctx);
            }

            return execute_sp2dn(ctx);
        }

    private:
        Status execute_sp2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("cl/vector_eadd_fdb_sp2dn");

            auto                        t   = ctx.task.template cast<ScheduleTask_v_eadd_fdb>();
            ref_ptr<TVector<T>>         r   = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         v   = t->v.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         fdb = t->fdb.template cast<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op  = t->op.template cast<TOpBinary<T, T, T>>();

            if (!ensure_kernel(op)) return Status::CompilationError;

            r->validate_rwd(Format::CLDenseVec);
            v->validate_rw(Format::CLCooVec);
            fdb->validate_wd(Format::CLCooVec);

            auto*       p_cl_r   = r->template get<CLDenseVec<T>>();
            const auto* p_cl_v   = v->template get<CLCooVec<T>>();
            auto*       p_cl_fdb = fdb->template get<CLCooVec<T>>();
            auto*       p_cl_acc = get_acc_cl();
            auto&       queue    = p_cl_acc->get_queue_default();

            const uint n = p_cl_v->values;

            if (n == 0) return Status::Ok;

            uint       fdb_size = 0;
            cl::Buffer cl_fdb_size(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), &fdb_size);
            cl::Buffer cl_fdb_i(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * n);
            cl::Buffer cl_fdb_x(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * n);

            auto kernel = m_program->make_kernel("sparse_to_dense");
            kernel.setArg(0, p_cl_r->Ax);
            kernel.setArg(1, p_cl_v->Ai);
            kernel.setArg(2, p_cl_v->Ax);
            kernel.setArg(3, cl_fdb_i);
            kernel.setArg(4, cl_fdb_x);
            kernel.setArg(5, cl_fdb_size);
            kernel.setArg(6, n);

            cl::NDRange global(align(n, p_cl_acc->get_default_wgz()));
            cl::NDRange local(p_cl_acc->get_default_wgz());
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
            queue.enqueueReadBuffer(cl_fdb_size, true, 0, sizeof(fdb_size), &fdb_size);

            p_cl_fdb->values = fdb_size;
            p_cl_fdb->Ai     = cl_fdb_i;
            p_cl_fdb->Ax     = cl_fdb_x;

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op) {
            if (m_compiled) return true;

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("vector_eadd")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op.template as<OpBinary>())
                    .set_source(source_vector_eadd)
                    .acquire();

            m_program  = program_builder.get_program();
            m_compiled = true;

            return true;
        }

        std::shared_ptr<CLProgram> m_program;
        bool                       m_compiled = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_EADD_HPP
