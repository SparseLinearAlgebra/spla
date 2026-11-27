/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2025 SparseLinearAlgebra                                         */
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

#ifndef SPLA_CL_M_EXTRACT_ROW_HPP
#define SPLA_CL_M_EXTRACT_ROW_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_counter.hpp>
#include <opencl/cl_debug.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_m_extract_row.hpp>

namespace spla {

    template<typename T>
    class Algo_m_extract_row_cl final : public RegistryAlgo {
    public:
        ~Algo_m_extract_row_cl() override = default;

        std::string get_name() override {
            return "m_extract_row";
        }

        std::string get_description() override {
            return "opencl extract row from matrix";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_m_extract_row>();

            ref_ptr<TVector<T>> r        = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>> M        = t->M.template cast_safe<TMatrix<T>>();
            auto                op_apply = t->op_apply.template cast_safe<TOpUnary<T, T>>();

            r->validate_wd(FormatVector::AccDense);
            M->validate_rw(FormatMatrix::AccCsr);

            auto* p_cl_r   = r->template get<CLDenseVec<T>>();
            auto* p_cl_M   = M->template get<CLCsr<T>>();
            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            // get the row boundaries from M->Ap
            uint       row_bounds[2];
            cl::Buffer cl_row_bounds(p_cl_acc->get_context(),
                                     CL_MEM_READ_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                     sizeof(row_bounds), row_bounds);

            queue.enqueueCopyBuffer(p_cl_M->Ap, cl_row_bounds, t->index * sizeof(uint), 0, sizeof(row_bounds));
            queue.finish();

            std::shared_ptr<CLProgram> program;
            ensure_kernel(op_apply, program);

            auto kernel = program->make_kernel("extract_row");
            kernel.setArg(0, p_cl_r->Ax);
            kernel.setArg(1, p_cl_M->Ax);
            kernel.setArg(2, p_cl_M->Aj);
            kernel.setArg(3, row_bounds[1]);

            // amount of elements in the row
            const uint n = row_bounds[1] - row_bounds[0] - 1;

            cl::NDRange global(p_cl_acc->get_default_wgs() * div_up_clamp(n, p_cl_acc->get_default_wgs(), 1u, 1024u));
            cl::NDRange local(p_cl_acc->get_default_wgs());
            queue.enqueueNDRangeKernel(kernel, cl::NDRange(row_bounds[0]), global, local);

            return Status::Ok;
        }

    private:
        void ensure_kernel(const ref_ptr<TOpUnary<T, T>>& op_apply, std::shared_ptr<CLProgram>& program) {
            CLProgramBuilder program_builder;
            program_builder
                    .set_name("m_extract_row")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_APPLY", op_apply.template as<OpUnary>())
                    .set_source(source_m_extract_row)
                    .acquire();
            program = program_builder.get_program();
        }
    };

}// namespace spla

#endif//SPLA_CL_M_EXTRACT_ROW_HPP
