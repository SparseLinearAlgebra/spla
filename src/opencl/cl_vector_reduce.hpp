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

            cl::NDRange global(256);
            cl::NDRange local(256);
            queue.enqueueNDRangeKernel(m_kernel, cl::NDRange(), global, local);
            cl::copy(queue, cl_sum, sum, sum + 1);

            r->get_value() = sum[0];

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_reduce) {
            if (m_compiled) return true;

            std::stringstream code;
            code << "#define BLOCK_SIZE " << 256 << "\n";
            code << "#define WARP_SIZE " << 32 << "\n";
            code << "#define TYPE " << get_ttype<T>()->get_cpp() << "\n";
            code << op_reduce->get_type_res()->get_cpp() << " OP1 " << op_reduce->get_source() << "\n";
            code << auto_vector_reduce_cl;
            m_source = code.str();

            auto acc = get_acc_cl();

            m_program = cl::Program(acc->get_context(), m_source);
            if (m_program.build(acc->get_device(), "-cl-std=CL1.2") != CL_SUCCESS) {
                LOG_MSG(Status::Error, "failed to build program: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(acc->get_device()));
                LOG_MSG(Status::Ok, "src\n" << m_source);
                return false;
            }

            m_kernel   = cl::Kernel(m_program, "reduce");
            m_compiled = true;

            return true;
        }

        std::string m_source;
        cl::Kernel  m_kernel;
        cl::Program m_program;
        bool        m_compiled = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_REDUCE_HPP
