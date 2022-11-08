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

#ifndef SPLA_CL_VECTOR_ASSIGN_HPP
#define SPLA_CL_VECTOR_ASSIGN_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_kernel_builder.hpp>
#include <opencl/generated/auto_vector_assign.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_v_assign_masked_cl final : public RegistryAlgo {
    public:
        ~Algo_v_assign_masked_cl() override = default;

        std::string get_name() override {
            return "v_assign_masked";
        }

        std::string get_description() override {
            return "parallel vector masked assignment on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("opencl/vector_assign");

            auto t = ctx.task.template cast<ScheduleTask_v_assign_masked>();

            auto r         = t->r.template cast<TVector<T>>();
            auto mask      = t->mask.template cast<TVector<T>>();
            auto value     = t->value.template cast<TScalar<T>>();
            auto op_assign = t->op_assign.template cast<TOpBinary<T, T, T>>();
            auto op_select = t->op_select.template cast<TOpSelect<T>>();

            r->cl_ensure_dense();
            mask->cl_ensure_dense();
            if (!ensure_kernel(op_assign, op_select)) return Status::Error;

            auto*       p_cl_r_dense    = r->template get_dec_p<CLDenseVec<T>>();
            const auto* p_cl_mask_dense = mask->template get_dec_p<CLDenseVec<T>>();
            auto*       p_cl_acc        = get_acc_cl();
            auto&       queue           = p_cl_acc->get_queue_default();

            T          assign_value[] = {value->get_value()};
            cl::Buffer cl_assign_value(p_cl_acc->get_context(), CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(T), assign_value);

            m_kernel.setArg(0, p_cl_r_dense->Ax);
            m_kernel.setArg(1, p_cl_mask_dense->Ax);
            m_kernel.setArg(2, cl_assign_value);
            m_kernel.setArg(3, r->get_n_rows());

            cl::NDRange global(p_cl_acc->get_grid_dim(r->get_n_rows()));
            cl::NDRange local(p_cl_acc->get_default_wgz());
            queue.enqueueNDRangeKernel(m_kernel, cl::NDRange(), global, local);
            queue.finish();

            r->cl_update_dense();

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_assign, const ref_ptr<TOpSelect<T>>& op_select) {
            if (m_compiled) return true;

            CLKernelBuilder kernel_builder;
            kernel_builder
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op_assign.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .add_code(source_vector_assign);

            if (!kernel_builder.build()) return false;

            m_program  = kernel_builder.get_program();
            m_kernel   = cl::Kernel(m_program, "assign");
            m_compiled = true;

            return true;
        }

        cl::Kernel  m_kernel;
        cl::Program m_program;
        bool        m_compiled = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_ASSIGN_HPP