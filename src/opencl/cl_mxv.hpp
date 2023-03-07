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

#include <opencl/cl_counter.hpp>
#include <opencl/cl_debug.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
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
            auto t          = ctx.task.template cast_safe<ScheduleTask_mxv_masked>();
            auto early_exit = t->get_desc_or_default()->get_early_exit();

            if (early_exit) {
                return execute_config_scalar(ctx);
            } else {
                return execute_vector(ctx);
            }
        }

    private:
        Status execute_vector(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/mxv/vector");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxv_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            r->validate_wd(FormatVector::AccDense);
            mask->validate_rw(FormatVector::AccDense);
            M->validate_rw(FormatMatrix::AccCsr);
            v->validate_rw(FormatVector::AccDense);

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, op_add, op_select, program)) return Status::CompilationError;

            auto* p_cl_r    = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M    = M->template get<CLCsr<T>>();
            auto* p_cl_v    = v->template get<CLDenseVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            auto kernel_vector = program->make_kernel("mxv_vector");
            kernel_vector.setArg(0, p_cl_M->Ap);
            kernel_vector.setArg(1, p_cl_M->Aj);
            kernel_vector.setArg(2, p_cl_M->Ax);
            kernel_vector.setArg(3, p_cl_v->Ax);
            kernel_vector.setArg(4, p_cl_mask->Ax);
            kernel_vector.setArg(5, p_cl_r->Ax);
            kernel_vector.setArg(6, init->get_value());
            kernel_vector.setArg(7, r->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(r->get_n_rows(), m_block_count, 1, 512);

            cl::NDRange exec_global(m_block_count * n_groups_to_dispatch, m_block_size);
            cl::NDRange exec_local(m_block_count, m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, kernel_vector, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        Status execute_scalar(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/mxv/scalar");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxv_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            r->validate_wd(FormatVector::AccDense);
            mask->validate_rw(FormatVector::AccDense);
            M->validate_rw(FormatMatrix::AccCsr);
            v->validate_rw(FormatVector::AccDense);

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, op_add, op_select, program)) return Status::CompilationError;

            auto* p_cl_r     = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask  = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M     = M->template get<CLCsr<T>>();
            auto* p_cl_v     = v->template get<CLDenseVec<T>>();
            auto  early_exit = t->get_desc_or_default()->get_early_exit();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            auto kernel_scalar = program->make_kernel("mxv_scalar");
            kernel_scalar.setArg(0, p_cl_M->Ap);
            kernel_scalar.setArg(1, p_cl_M->Aj);
            kernel_scalar.setArg(2, p_cl_M->Ax);
            kernel_scalar.setArg(3, p_cl_v->Ax);
            kernel_scalar.setArg(4, p_cl_mask->Ax);
            kernel_scalar.setArg(5, p_cl_r->Ax);
            kernel_scalar.setArg(6, init->get_value());
            kernel_scalar.setArg(7, r->get_n_rows());
            kernel_scalar.setArg(8, uint(early_exit));

            uint n_groups_to_dispatch = div_up_clamp(r->get_n_rows(), m_block_size, 1, 512);

            cl::NDRange exec_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange exec_local(m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, kernel_scalar, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        Status execute_config_scalar(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/mxv/config-scalar");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxv_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            r->validate_wd(FormatVector::AccDense);
            mask->validate_rw(FormatVector::AccDense);
            M->validate_rw(FormatMatrix::AccCsr);
            v->validate_rw(FormatVector::AccDense);

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, op_add, op_select, program)) return Status::CompilationError;

            auto* p_cl_r     = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask  = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M     = M->template get<CLCsr<T>>();
            auto* p_cl_v     = v->template get<CLDenseVec<T>>();
            auto  early_exit = t->get_desc_or_default()->get_early_exit();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            uint       config_size = 0;
            cl::Buffer cl_config(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * M->get_n_rows());
            cl::Buffer cl_config_size(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), &config_size);

            auto kernel_config = program->make_kernel("mxv_config");
            kernel_config.setArg(0, p_cl_mask->Ax);
            kernel_config.setArg(1, p_cl_r->Ax);
            kernel_config.setArg(2, cl_config);
            kernel_config.setArg(3, cl_config_size);
            kernel_config.setArg(4, init->get_value());
            kernel_config.setArg(5, M->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(r->get_n_rows(), m_block_size, 1, 1024);

            cl::NDRange config_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange config_local(m_block_size);
            CL_DISPATCH_PROFILED("config", queue, kernel_config, cl::NDRange(), config_global, config_local);

            CL_READ_PROFILED("config-size", queue, cl_config_size, true, 0, sizeof(config_size), &config_size);

            auto kernel_config_scalar = program->make_kernel("mxv_config_scalar");
            kernel_config_scalar.setArg(0, p_cl_M->Ap);
            kernel_config_scalar.setArg(1, p_cl_M->Aj);
            kernel_config_scalar.setArg(2, p_cl_M->Ax);
            kernel_config_scalar.setArg(3, p_cl_v->Ax);
            kernel_config_scalar.setArg(4, cl_config);
            kernel_config_scalar.setArg(5, p_cl_r->Ax);
            kernel_config_scalar.setArg(6, init->get_value());
            kernel_config_scalar.setArg(7, config_size);
            kernel_config_scalar.setArg(8, uint(early_exit));

            n_groups_to_dispatch = div_up_clamp(config_size, m_block_size, 1, 1024);

            cl::NDRange exec_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange exec_local(m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, kernel_config_scalar, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_multiply,
                           const ref_ptr<TOpBinary<T, T, T>>& op_add,
                           const ref_ptr<TOpSelect<T>>&       op_select,
                           std::shared_ptr<CLProgram>&        program) {
            m_block_size  = get_acc_cl()->get_wave_size();
            m_block_count = 1;

            assert(m_block_count >= 1);
            assert(m_block_size * m_block_count == get_acc_cl()->get_wave_size());

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("mxv")
                    .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_define("BLOCK_COUNT", m_block_count)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .add_op("OP_BINARY2", op_add.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .set_source(source_mxv)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

    private:
        uint m_block_size  = 0;
        uint m_block_count = 0;
    };

}// namespace spla

#endif//SPLA_CL_MXV_HPP
