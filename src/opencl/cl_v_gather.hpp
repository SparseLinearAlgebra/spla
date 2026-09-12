/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
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

#ifndef SPLA_CL_V_GATHER_HPP
#define SPLA_CL_V_GATHER_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_vector_gather.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_gather_cl final : public RegistryAlgo {
    public:
        ~Algo_v_gather_cl() override = default;

        std::string get_name() override {
            return "v_gather";
        }

        std::string get_description() override {
            return "parallel vector gather on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("opencl/v_gather");

            auto t = ctx.task.template cast_safe<ScheduleTask_v_gather>();

            auto r       = t->r.template cast_safe<TVector<T>>();
            auto source  = t->source.template cast_safe<TVector<T>>();
            auto indices = t->indices.template cast_safe<TVector<T_UINT>>();

            r->validate_wd(FormatVector::AccDense);
            source->validate_rw(FormatVector::AccDense);
            indices->validate_rw(FormatVector::AccDense);

            auto* p_cl_r       = r->template get<CLDenseVec<T>>();
            auto* p_cl_source  = source->template get<CLDenseVec<T>>();
            auto* p_cl_indices = indices->template get<CLDenseVec<T_UINT>>();
            auto* p_cl_acc     = get_acc_cl();
            auto& queue        = p_cl_acc->get_queue_default();

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(program)) return Status::CompilationError;

            auto kernel = program->make_kernel("dense_to_dense");
            kernel.setArg(0, p_cl_source->Ax);
            kernel.setArg(1, p_cl_indices->Ax);
            kernel.setArg(2, p_cl_r->Ax);
            kernel.setArg(3, r->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(r->get_n_rows(), m_block_size, 1, 256);

            cl::NDRange global(m_block_size * n_groups_to_dispatch);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);

            return Status::Ok;
        }

    private:
        bool ensure_kernel(std::shared_ptr<CLProgram>& program) {
            m_block_size = get_acc_cl()->get_default_wgs();

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("v_gather")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .set_source(source_vector_gather)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

    private:
        uint m_block_size = 0;
    };

}// namespace spla

#endif//SPLA_CL_V_GATHER_HPP