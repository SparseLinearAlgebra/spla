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

#ifndef SPLA_CL_VECTOR_REDUCE_HPP
#define SPLA_CL_VECTOR_REDUCE_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_kernel_builder.hpp>
#include <opencl/generated/auto_vector_reduce.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_reduce_cl final : public RegistryAlgo {
    public:
        ~Algo_v_reduce_cl() override = default;

        std::string get_name() override {
            return "v_reduce";
        }

        std::string get_description() override {
            return "parallel vector reduction on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("opencl/vector_reduce");

            auto t = ctx.task.template cast<ScheduleTask_v_reduce>();

            auto r         = t->r.template cast<TScalar<T>>();
            auto s         = t->s.template cast<TScalar<T>>();
            auto v         = t->v.template cast<TVector<T>>();
            auto op_reduce = t->op_reduce.template cast<TOpBinary<T, T, T>>();

            v->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_reduce)) return Status::Error;

            const auto* p_cl_dense_vec = v->template get<CLDenseVec<T>>();
            auto*       p_cl_acc       = get_acc_cl();
            auto&       queue          = p_cl_acc->get_queue_default();

            const uint N             = v->get_n_rows();
            const uint OPTIMAL_SPLIT = 64;
            const uint STRIDE        = std::max(std::max(uint(N / OPTIMAL_SPLIT), uint((N + m_block_size) / m_block_size)), m_block_size);
            const uint GROUPS_COUNT  = N / STRIDE + (N % STRIDE ? 1 : 0);

            T          init[] = {s->get_value()};
            T          sum[1];
            cl::Buffer cl_init(p_cl_acc->get_context(), CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(T), init);
            cl::Buffer cl_sum(p_cl_acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(T));
            cl::Buffer cl_sum_group(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * GROUPS_COUNT);

            m_kernel_phase_1.setArg(0, p_cl_dense_vec->Ax);
            m_kernel_phase_1.setArg(1, cl_init);
            m_kernel_phase_1.setArg(2, cl_sum_group);
            m_kernel_phase_1.setArg(3, STRIDE);
            m_kernel_phase_1.setArg(4, N);

            cl::NDRange global_phase_1(m_block_size * GROUPS_COUNT);
            cl::NDRange local_phase_1(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel_phase_1, cl::NDRange(), global_phase_1, local_phase_1);

            m_kernel_phase_2.setArg(0, cl_sum_group);
            m_kernel_phase_2.setArg(1, cl_init);
            m_kernel_phase_2.setArg(2, cl_sum);
            m_kernel_phase_2.setArg(3, STRIDE);
            m_kernel_phase_2.setArg(4, GROUPS_COUNT);

            cl::NDRange global_phase_2(m_block_size);
            cl::NDRange local_phase_2(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel_phase_2, cl::NDRange(), global_phase_2, local_phase_2);

            cl::copy(queue, cl_sum, sum, sum + 1);

            r->get_value() = sum[0];

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_reduce) {
            if (m_compiled) return true;

            m_block_size = std::min(uint(1024), get_acc_cl()->get_max_wgs());

            CLKernelBuilder kernel_builder;
            kernel_builder
                    .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op_reduce.template as<OpBinary>())
                    .add_code(source_vector_reduce);

            if (!kernel_builder.build()) return false;

            m_program        = kernel_builder.get_program();
            m_kernel_phase_1 = cl::Kernel(m_program, "reduce");
            m_kernel_phase_2 = cl::Kernel(m_program, "reduce");
            m_compiled       = true;

            return true;
        }

        cl::Kernel  m_kernel_phase_1;
        cl::Kernel  m_kernel_phase_2;
        cl::Program m_program;
        uint        m_block_size = 0;
        bool        m_compiled   = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_REDUCE_HPP
