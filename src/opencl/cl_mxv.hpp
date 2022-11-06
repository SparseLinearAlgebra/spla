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

#ifndef SPLA_CL_MXV_HPP
#define SPLA_CL_MXV_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_formats.hpp>
#include <opencl/cl_kernel_builder.hpp>
#include <opencl/generated/auto_mxv.hpp>

#include <algorithm>
#include <sstream>

namespace spla {

    template<typename T>
    class Algo_mxv_masked_cl final : public RegistryAlgo {
    public:
        ~Algo_mxv_masked_cl() override = default;

        std::string get_name() override {
            return "mxv_masked";
        }

        std::string get_description() override {
            return "parallel matrix-vector masked product on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast<ScheduleTask_mxv_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast<TMatrix<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast<TScalar<T>>();

            r->cl_ensure_dense();
            mask->cl_ensure_dense();
            M->cl_ensure_csr();
            v->cl_ensure_dense();
            if (!ensure_kernel(op_multiply, op_add, op_select)) return Status::Error;

            auto* p_cl_r    = r->template get_dec_p<CLDenseVec<T>>();
            auto* p_cl_mask = mask->template get_dec_p<CLDenseVec<T>>();
            auto* p_cl_M    = M->template get_dec_p<CLCsr<T>>();
            auto* p_cl_v    = v->template get_dec_p<CLDenseVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            T          init_value[]  = {init->get_value()};
            uint       config_size[] = {0};
            cl::Buffer cl_init_value(p_cl_acc->get_context(), CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(T), init_value);
            cl::Buffer cl_config(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * M->get_n_rows());
            cl::Buffer cl_config_size(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), config_size);

            m_kernel_prepare.setArg(0, p_cl_mask->Ax);
            m_kernel_prepare.setArg(1, cl_init_value);
            m_kernel_prepare.setArg(2, p_cl_r->Ax);
            m_kernel_prepare.setArg(3, cl_config_size);
            m_kernel_prepare.setArg(4, cl_config);
            m_kernel_prepare.setArg(5, M->get_n_rows());

            cl::NDRange prepare_global(p_cl_acc->get_grid_dim(r->get_n_rows()));
            cl::NDRange prepare_local(p_cl_acc->get_default_wgz());
            queue.enqueueNDRangeKernel(m_kernel_prepare, cl::NDRange(), prepare_global, prepare_local);

            cl::copy(queue, cl_config_size, config_size, config_size + 1);

            m_kernel_exec.setArg(0, p_cl_M->Ap);
            m_kernel_exec.setArg(1, p_cl_M->Aj);
            m_kernel_exec.setArg(2, p_cl_M->Ax);
            m_kernel_exec.setArg(3, p_cl_v->Ax);
            m_kernel_exec.setArg(4, cl_init_value);
            m_kernel_exec.setArg(5, cl_config);
            m_kernel_exec.setArg(6, p_cl_r->Ax);
            m_kernel_exec.setArg(7, config_size[0]);

            uint n_groups_to_dispatch = std::max(std::min(config_size[0] / m_block_count, uint(256)), uint(1));

            cl::NDRange exec_global(m_block_count * n_groups_to_dispatch, m_block_size);
            cl::NDRange exec_local(m_block_count, m_block_size);
            queue.enqueueNDRangeKernel(m_kernel_exec, cl::NDRange(), exec_global, exec_local);
            queue.finish();

            r->cl_update_dense();

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_multiply,
                           const ref_ptr<TOpBinary<T, T, T>>& op_add,
                           const ref_ptr<TOpSelect<T>>&       op_select) {
            if (m_compiled) return true;

            m_block_size  = 32;
            m_block_count = get_acc_cl()->get_default_wgz() / 32;

            assert(m_block_count > 1);
            assert(m_block_size * m_block_count == get_acc_cl()->get_default_wgz());

            CLKernelBuilder kernel_builder;
            kernel_builder
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_define("BLOCK_COUNT", m_block_count)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .add_op("OP_BINARY2", op_add.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .add_code(source_mxv);

            if (!kernel_builder.build()) return false;

            m_program        = kernel_builder.get_program();
            m_kernel_prepare = cl::Kernel(m_program, "mxv_pull_prepare");
            m_kernel_exec    = cl::Kernel(m_program, "mxv_pull_exec");
            m_compiled       = true;

            return true;
        }

        cl::Kernel  m_kernel_prepare;
        cl::Kernel  m_kernel_exec;
        cl::Program m_program;
        uint        m_block_size  = 0;
        uint        m_block_count = 0;
        bool        m_compiled    = false;
    };

}// namespace spla

#endif//SPLA_CL_MXV_HPP
