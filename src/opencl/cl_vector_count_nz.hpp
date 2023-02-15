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

#ifndef SPLA_CL_VECTOR_COUNT_NZ_HPP
#define SPLA_CL_VECTOR_COUNT_NZ_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_counter.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_count.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_count_nz_cl final : public RegistryAlgo {
    public:
        ~Algo_v_count_nz_cl() override = default;

        std::string get_name() override {
            return "v_count_nz";
        }

        std::string get_description() override {
            return "parallel vector count nz";
        }

        Status execute(const DispatchContext& ctx) override {
            auto                t = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v = t->v.template cast<TVector<T>>();

            if (v->is_valid(FormatVector::AccCoo))
                return execute_sp(ctx);
            if (v->is_valid(FormatVector::AccDense))
                return execute_dn(ctx);

            return execute_sp(ctx);
        }

    private:
        Status execute_sp(const DispatchContext& ctx) {
            auto                t     = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v     = t->v.template cast<TVector<T>>();
            CLCooVec<T>*        dec_v = v->template get<CLCooVec<T>>();

            t->r->set_uint(dec_v->values);

            return Status::Ok;
        }

        Status execute_dn(const DispatchContext& ctx) {
            auto                t     = ctx.task.template cast<ScheduleTask_v_count_nz>();
            ref_ptr<TVector<T>> v     = t->v.template cast<TVector<T>>();
            CLDenseVec<T>*      dec_v = v->template get<CLDenseVec<T>>();

            if (!ensure_kernel()) return Status::CompilationError;

            auto* cl_acc = get_acc_cl();
            auto& queue  = cl_acc->get_queue_default();

            CLCounterWrapper cl_count;
            cl_count.set(queue, 0);

            auto kernel = m_program->make_kernel("count_nz");
            kernel.setArg(0, dec_v->Ax);
            kernel.setArg(1, cl_count.buffer());
            kernel.setArg(2, v->get_n_rows());

            const uint n_groups = div_up_clamp(v->get_n_rows(), m_block_size, 1, 1024);

            cl::NDRange global(m_block_size * n_groups);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

            t->r->set_uint(cl_count.get(queue));

            return Status::Ok;
        }

        bool ensure_kernel() {
            if (m_compiled) return true;

            m_block_size = get_acc_cl()->get_default_wgz();

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("count")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .set_source(source_count)
                    .acquire();

            m_program  = program_builder.get_program();
            m_compiled = true;

            return true;
        }

        std::shared_ptr<CLProgram> m_program;
        uint                       m_block_size = 0;
        bool                       m_compiled   = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_COUNT_NZ_HPP
