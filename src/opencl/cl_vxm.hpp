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

#ifndef SPLA_CL_VXM_HPP
#define SPLA_CL_VXM_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/tmatrix.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_debug.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/cl_reduce_by_key.hpp>
#include <opencl/cl_sort_by_key.hpp>
#include <opencl/generated/auto_vxm.hpp>

#include <algorithm>
#include <sstream>

namespace spla {

    template<typename T>
    class Algo_vxm_masked_cl final : public RegistryAlgo {
    public:
        ~Algo_vxm_masked_cl() override = default;

        std::string get_name() override {
            return "vxm_masked";
        }

        std::string get_description() override {
            return "parallel vector-matrix masked product on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            return execute_sparse(ctx);
        }

    private:
        Status execute_vector(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vxm/vector");

            auto t = ctx.task.template cast<ScheduleTask_vxm_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast<TScalar<T>>();

            r->validate_rwd(Format::CLDenseVec);
            mask->validate_rw(Format::CLDenseVec);
            M->validate_rw(Format::CLCsr);
            v->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_multiply, op_add, op_select)) return Status::CompilationError;

            auto* p_cl_r    = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M    = M->template get<CLCsr<T>>();
            auto* p_cl_v    = v->template get<CLDenseVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            m_kernel_prepare.setArg(0, p_cl_r->Ax);
            m_kernel_prepare.setArg(1, init->get_value());
            m_kernel_prepare.setArg(2, r->get_n_rows());

            cl::NDRange prepare_global(align(r->get_n_rows(), p_cl_acc->get_wave_size()));
            cl::NDRange prepare_local(p_cl_acc->get_wave_size());
            CL_DISPATCH_PROFILED("config", queue, m_kernel_prepare, cl::NDRange(), prepare_global, prepare_local);

            m_kernel_atomic_vector.setArg(0, p_cl_v->Ax);
            m_kernel_atomic_vector.setArg(1, p_cl_M->Ap);
            m_kernel_atomic_vector.setArg(2, p_cl_M->Aj);
            m_kernel_atomic_vector.setArg(3, p_cl_M->Ax);
            m_kernel_atomic_vector.setArg(4, p_cl_mask->Ax);
            m_kernel_atomic_vector.setArg(5, p_cl_r->Ax);
            m_kernel_atomic_vector.setArg(6, v->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(v->get_n_rows(), m_block_count, 1, 512);

            cl::NDRange exec_global(m_block_count * n_groups_to_dispatch, m_block_size);
            cl::NDRange exec_local(m_block_count, m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, m_kernel_atomic_vector, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        Status execute_scalar(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vxm/scalar");

            auto t = ctx.task.template cast<ScheduleTask_vxm_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast<TScalar<T>>();

            r->validate_rwd(Format::CLDenseVec);
            mask->validate_rw(Format::CLDenseVec);
            M->validate_rw(Format::CLCsr);
            v->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_multiply, op_add, op_select)) return Status::CompilationError;

            auto* p_cl_r     = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask  = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M     = M->template get<CLCsr<T>>();
            auto* p_cl_v     = v->template get<CLDenseVec<T>>();
            auto  early_exit = t->get_desc_or_default()->get_early_exit();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            m_kernel_prepare.setArg(0, p_cl_r->Ax);
            m_kernel_prepare.setArg(1, init->get_value());
            m_kernel_prepare.setArg(2, r->get_n_rows());

            cl::NDRange prepare_global(get_grid_dim(r->get_n_rows(), p_cl_acc->get_wave_size()));
            cl::NDRange prepare_local(p_cl_acc->get_wave_size());
            CL_DISPATCH_PROFILED("config", queue, m_kernel_prepare, cl::NDRange(), prepare_global, prepare_local);

            m_kernel_atomic_scalar.setArg(0, p_cl_v->Ax);
            m_kernel_atomic_scalar.setArg(1, p_cl_M->Ap);
            m_kernel_atomic_scalar.setArg(2, p_cl_M->Aj);
            m_kernel_atomic_scalar.setArg(3, p_cl_M->Ax);
            m_kernel_atomic_scalar.setArg(4, p_cl_mask->Ax);
            m_kernel_atomic_scalar.setArg(5, p_cl_r->Ax);
            m_kernel_atomic_scalar.setArg(6, v->get_n_rows());
            m_kernel_atomic_scalar.setArg(7, uint(early_exit));

            uint n_groups_to_dispatch = div_up_clamp(v->get_n_rows(), m_block_size, 1, 512);

            cl::NDRange exec_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange exec_local(m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, m_kernel_atomic_scalar, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        Status execute_config_scalar(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vxm/config-scalar");

            auto t = ctx.task.template cast<ScheduleTask_vxm_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast<TScalar<T>>();

            r->validate_wd(Format::CLDenseVec);
            mask->validate_rw(Format::CLDenseVec);
            M->validate_rw(Format::CLCsr);
            v->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_multiply, op_add, op_select)) return Status::CompilationError;

            auto* p_cl_r     = r->template get<CLDenseVec<T>>();
            auto* p_cl_mask  = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M     = M->template get<CLCsr<T>>();
            auto* p_cl_v     = v->template get<CLDenseVec<T>>();
            auto  early_exit = t->get_desc_or_default()->get_early_exit();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            uint       config_size[] = {0};
            cl::Buffer cl_config(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * M->get_n_rows());
            cl::Buffer cl_config_size(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), config_size);

            m_kernel_config.setArg(0, p_cl_v->Ax);
            m_kernel_config.setArg(1, p_cl_r->Ax);
            m_kernel_config.setArg(2, cl_config);
            m_kernel_config.setArg(3, cl_config_size);
            m_kernel_config.setArg(4, init->get_value());
            m_kernel_config.setArg(5, M->get_n_rows());
            m_kernel_config.setArg(6, M->get_n_cols());

            uint n_groups_to_dispatch = div_up_clamp(v->get_n_rows(), m_block_size, 1, 512);

            cl::NDRange config_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange config_local(m_block_size);
            CL_DISPATCH_PROFILED("config", queue, m_kernel_config, cl::NDRange(), config_global, config_local);

            queue.enqueueReadBuffer(cl_config_size, true, 0, sizeof(config_size[0]), config_size);

            m_kernel_config_atomic_scalar.setArg(0, p_cl_v->Ax);
            m_kernel_config_atomic_scalar.setArg(1, p_cl_M->Ap);
            m_kernel_config_atomic_scalar.setArg(2, p_cl_M->Aj);
            m_kernel_config_atomic_scalar.setArg(3, p_cl_M->Ax);
            m_kernel_config_atomic_scalar.setArg(4, p_cl_mask->Ax);
            m_kernel_config_atomic_scalar.setArg(5, cl_config);
            m_kernel_config_atomic_scalar.setArg(6, p_cl_r->Ax);
            m_kernel_config_atomic_scalar.setArg(7, config_size);
            m_kernel_config_atomic_scalar.setArg(8, uint(early_exit));

            cl::NDRange exec_global(m_block_size * n_groups_to_dispatch);
            cl::NDRange exec_local(m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, m_kernel_config_atomic_scalar, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        Status execute_sparse(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vxm/sparse");

            auto t = ctx.task.template cast<ScheduleTask_vxm_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast<TVector<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast<TScalar<T>>();

            r->validate_wd(Format::CLCooVec);
            mask->validate_rw(Format::CLDenseVec);
            M->validate_rw(Format::CLCsr);
            v->validate_rw(Format::CLCooVec);
            if (!ensure_kernel(op_multiply, op_add, op_select)) return Status::CompilationError;

            auto* p_cl_r    = r->template get<CLCooVec<T>>();
            auto* p_cl_mask = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M    = M->template get<CLCsr<T>>();
            auto* p_cl_v    = v->template get<CLCooVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            uint       prods_count[] = {0};
            cl::Buffer cl_prods_count(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), prods_count);
            cl::Buffer cl_prods_offset(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), prods_count);

            m_kernel_sparse_count.setArg(0, p_cl_v->Ai);
            m_kernel_sparse_count.setArg(1, p_cl_v->Ax);
            m_kernel_sparse_count.setArg(2, p_cl_M->Ap);
            m_kernel_sparse_count.setArg(3, p_cl_M->Aj);
            m_kernel_sparse_count.setArg(4, p_cl_mask->Ax);
            m_kernel_sparse_count.setArg(5, cl_prods_count);
            m_kernel_sparse_count.setArg(6, p_cl_v->values);

            uint n_groups_to_dispatch_v = div_up_clamp(p_cl_v->values, m_block_size, 1, 512);

            cl::NDRange count_global(m_block_size * n_groups_to_dispatch_v);
            cl::NDRange count_local(m_block_size);
            CL_DISPATCH_PROFILED("count", queue, m_kernel_sparse_count, cl::NDRange(), count_global, count_local);

            CL_READ_PROFILED("copy_prods_count", queue, cl_prods_count, true, 0, sizeof(prods_count[0]), prods_count);
            LOG_MSG(Status::Ok, "temporary vi * A[,*] count " << prods_count[0]);

            if (prods_count[0] == 0) {
                LOG_MSG(Status::Ok, "nothing to do");

                p_cl_r->Ai     = cl::Buffer();
                p_cl_r->Ax     = cl::Buffer();
                p_cl_r->values = 0;

                return Status::Ok;
            }

            cl::Buffer cl_prodi(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, prods_count[0] * sizeof(uint));
            cl::Buffer cl_prodx(p_cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, prods_count[0] * sizeof(T));

            m_kernel_sparse_collect.setArg(0, p_cl_v->Ai);
            m_kernel_sparse_collect.setArg(1, p_cl_v->Ax);
            m_kernel_sparse_collect.setArg(2, p_cl_M->Ap);
            m_kernel_sparse_collect.setArg(3, p_cl_M->Aj);
            m_kernel_sparse_collect.setArg(4, p_cl_M->Ax);
            m_kernel_sparse_collect.setArg(5, p_cl_mask->Ax);
            m_kernel_sparse_collect.setArg(6, cl_prodi);
            m_kernel_sparse_collect.setArg(7, cl_prodx);
            m_kernel_sparse_collect.setArg(8, cl_prods_offset);
            m_kernel_sparse_collect.setArg(9, p_cl_v->values);

            cl::NDRange collect_global(m_block_size * n_groups_to_dispatch_v);
            cl::NDRange collect_local(m_block_size);
            CL_DISPATCH_PROFILED("collect", queue, m_kernel_sparse_collect, cl::NDRange(), collect_global, collect_local);

            CL_PROFILE_BEGIN("sort", queue)
            const uint max_key    = r->get_n_rows() - 1;
            const uint n_elements = prods_count[0];
            cl_sort_by_key<T>(queue, cl_prodi, cl_prodx, n_elements, max_key);
            CL_PROFILE_END();

            uint       reduced_size;
            cl::Buffer reduced_keys;
            cl::Buffer reduced_values;

            CL_PROFILE_BEGIN("reduce", queue)
            cl_reduce_by_key(queue, cl_prodi, cl_prodx, prods_count[0], reduced_keys, reduced_values, reduced_size, op_add);
            CL_PROFILE_END();

            p_cl_r->Ai     = reduced_keys;
            p_cl_r->Ax     = reduced_values;
            p_cl_r->values = reduced_size;

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_multiply,
                           const ref_ptr<TOpBinary<T, T, T>>& op_add,
                           const ref_ptr<TOpSelect<T>>&       op_select) {
            if (m_compiled) return true;

            m_block_size  = get_acc_cl()->get_wave_size();
            m_block_count = 1;

            assert(m_block_count >= 1);
            assert(m_block_size * m_block_count == get_acc_cl()->get_wave_size());

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("vxm")
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .add_op("OP_BINARY2", op_add.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .set_source(source_vxm)
                    .acquire();

            m_program                     = program_builder.get_program();
            m_kernel_prepare              = m_program->make_kernel("vxm_prepare");
            m_kernel_atomic_vector        = m_program->make_kernel("vxm_atomic_vector");
            m_kernel_atomic_scalar        = m_program->make_kernel("vxm_atomic_scalar");
            m_kernel_config               = m_program->make_kernel("vxm_config");
            m_kernel_config_atomic_scalar = m_program->make_kernel("vxm_config_atomic_scalar");
            m_kernel_sparse_count         = m_program->make_kernel("vxm_sparse_count");
            m_kernel_sparse_collect       = m_program->make_kernel("vxm_sparse_collect");
            m_kernel_sparse_reduce        = m_program->make_kernel("vxm_sparse_reduce");
            m_compiled                    = true;

            return true;
        }

        std::shared_ptr<CLProgram> m_program;
        cl::Kernel                 m_kernel_prepare;
        cl::Kernel                 m_kernel_exec_serial;
        cl::Kernel                 m_kernel_atomic_vector;
        cl::Kernel                 m_kernel_atomic_scalar;
        cl::Kernel                 m_kernel_config;
        cl::Kernel                 m_kernel_config_atomic_scalar;
        cl::Kernel                 m_kernel_sparse_count;
        cl::Kernel                 m_kernel_sparse_collect;
        cl::Kernel                 m_kernel_sparse_reduce;
        uint                       m_block_size  = 0;
        uint                       m_block_count = 0;
        bool                       m_compiled    = false;
    };

}// namespace spla

#endif//SPLA_CL_VXM_HPP
