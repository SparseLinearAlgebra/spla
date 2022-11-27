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

#ifndef SPLA_CL_VECTOR_SELECT_COUNT_HPP
#define SPLA_CL_VECTOR_SELECT_COUNT_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_kernel_builder.hpp>
#include <opencl/generated/auto_vector_select_count.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_select_count_cl final : public RegistryAlgo {
    public:
        ~Algo_v_select_count_cl() override = default;

        std::string get_name() override {
            return "v_select_count";
        }

        std::string get_description() override {
            return "parallel vector select count on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE(count, "opencl/vector_select_count");

            auto t = ctx.task.template cast<ScheduleTask_v_select_count>();

            auto r         = t->r;
            auto v         = t->v.template cast<TVector<T>>();
            auto op_select = t->op_select.template cast<TOpSelect<T>>();

            v->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_select)) return Status::Error;

            auto* p_cl_v_dense = v->template get<CLDenseVec<T>>();
            auto* p_cl_acc     = get_acc_cl();
            auto& queue        = p_cl_acc->get_queue_default();

            int        count[] = {0};
            cl::Buffer cl_count(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), count);

            m_kernel.setArg(0, p_cl_v_dense->Ax);
            m_kernel.setArg(1, cl_count);
            m_kernel.setArg(2, v->get_n_rows());

            uint n_groups_to_dispatch = std::max(std::min(v->get_n_rows() / m_block_size, uint(256)), uint(1));

            cl::NDRange global(m_block_size * n_groups_to_dispatch);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel, cl::NDRange(), global, local);

            cl::copy(queue, cl_count, count, count + 1);

            r->set_int(count[0]);

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpSelect<T>>& op_select) {
            if (m_compiled) return true;

            m_block_size = get_acc_cl()->get_wave_size();

            CLKernelBuilder kernel_builder;
            kernel_builder
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .add_code(source_vector_select_count);

            if (!kernel_builder.build()) return false;

            m_program  = kernel_builder.get_program();
            m_kernel   = cl::Kernel(m_program, "select_count");
            m_compiled = true;

            return true;
        }

        cl::Kernel  m_kernel;
        cl::Program m_program;
        uint        m_block_size = 0;
        bool        m_compiled   = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_SELECT_COUNT_HPP
