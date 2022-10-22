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
            auto t = ctx.task.template cast<ScheduleTask_v_reduce>();

            auto r         = t->r.template cast<TScalar<T>>();
            auto s         = t->s.template cast<TScalar<T>>();
            auto v         = t->v.template cast<TVector<T>>();
            auto op_reduce = t->op_reduce.template cast<TOpBinary<T, T, T>>();

            v->cl_ensure_dense();
            if (!ensure_kernel(op_reduce)) return Status::Error;

            const auto* p_cl_dense_vec = v->template get_dec_p<CLDenseVec<T>>();
            auto*       p_cl_acc       = get_acc_cl();
            auto&       queue          = p_cl_acc->get_queue_default();

            T          sum[] = {s->get_value()};
            cl::Buffer cl_sum(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(T), sum);

            m_kernel.setArg(0, p_cl_dense_vec->Ax);
            m_kernel.setArg(1, cl_sum);
            m_kernel.setArg(2, v->get_n_rows());

            cl::NDRange global(m_block_size);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel, cl::NDRange(), global, local);
            cl::copy(queue, cl_sum, sum, sum + 1);

            r->get_value() = sum[0];

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_reduce) {
            if (m_compiled) return true;

            m_block_size = get_acc_cl()->get_max_work_group_size();

            CLKernelBuilder kernel_builder;
            kernel_builder
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP1", op_reduce.template as<OpBinary>())
                    .add_code(source_vector_reduce);

            if (!kernel_builder.build()) return false;

            m_program  = kernel_builder.get_program();
            m_kernel   = cl::Kernel(m_program, "reduce");
            m_compiled = true;

            return true;
        }

        cl::Kernel  m_kernel;
        cl::Program m_program;
        uint        m_block_size = 0;
        bool        m_compiled   = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_REDUCE_HPP
