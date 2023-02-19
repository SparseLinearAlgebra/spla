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

#include <opencl/cl_alloc_linear.hpp>
#include <opencl/cl_counter.hpp>
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
        Status execute_sparse(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/vxm/sparse");

            auto t = ctx.task.template cast_safe<ScheduleTask_vxm_masked>();

            ref_ptr<TVector<T>>         r           = t->r.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         mask        = t->mask.template cast_safe<TVector<T>>();
            ref_ptr<TVector<T>>         v           = t->v.template cast_safe<TVector<T>>();
            ref_ptr<TMatrix<T>>         M           = t->M.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            r->validate_wd(FormatVector::AccCoo);
            mask->validate_rw(FormatVector::AccDense);
            M->validate_rw(FormatMatrix::AccCsr);
            v->validate_rw(FormatVector::AccCoo);
            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, op_add, op_select, program)) return Status::CompilationError;

            auto* p_cl_r    = r->template get<CLCooVec<T>>();
            auto* p_cl_mask = mask->template get<CLDenseVec<T>>();
            auto* p_cl_M    = M->template get<CLCsr<T>>();
            auto* p_cl_v    = v->template get<CLCooVec<T>>();

            auto* p_cl_acc    = get_acc_cl();
            auto* p_tmp_alloc = p_cl_acc->get_alloc_tmp();
            auto& queue       = p_cl_acc->get_queue_default();

            uint             prods_count;
            CLCounterWrapper cl_prods_count;
            CL_COUNTER_SET("init-prods-cnt", queue, cl_prods_count, 0);

            auto kernel_sparse_count = program->make_kernel("vxm_sparse_count");
            kernel_sparse_count.setArg(0, p_cl_v->Ai);
            kernel_sparse_count.setArg(1, p_cl_v->Ax);
            kernel_sparse_count.setArg(2, p_cl_M->Ap);
            kernel_sparse_count.setArg(3, p_cl_M->Aj);
            kernel_sparse_count.setArg(4, p_cl_mask->Ax);
            kernel_sparse_count.setArg(5, cl_prods_count.buffer());
            kernel_sparse_count.setArg(6, p_cl_v->values);

            uint n_groups_to_dispatch_v = div_up_clamp(p_cl_v->values, m_block_size, 1, 512);

            cl::NDRange count_global(m_block_size * n_groups_to_dispatch_v);
            cl::NDRange count_local(m_block_size);
            CL_DISPATCH_PROFILED("count", queue, kernel_sparse_count, cl::NDRange(), count_global, count_local);

            CL_COUNTER_GET("copy_prods_count", queue, cl_prods_count, prods_count);
            LOG_MSG(Status::Ok, "temporary vi * A[,*] count " << prods_count);

            if (prods_count == 0) {
                LOG_MSG(Status::Ok, "nothing to do");

                p_cl_r->Ai     = cl::Buffer();
                p_cl_r->Ax     = cl::Buffer();
                p_cl_r->values = 0;

                return Status::Ok;
            }

            cl::Buffer cl_prodi = p_tmp_alloc->alloc(prods_count * sizeof(uint));
            cl::Buffer cl_prodx = p_tmp_alloc->alloc(prods_count * sizeof(T));

            CLCounterWrapper cl_prods_offset;
            CL_COUNTER_SET("init-offsets-cnt", queue, cl_prods_offset, 0);

            auto kernel_sparse_collect = program->make_kernel("vxm_sparse_collect");
            kernel_sparse_collect.setArg(0, p_cl_v->Ai);
            kernel_sparse_collect.setArg(1, p_cl_v->Ax);
            kernel_sparse_collect.setArg(2, p_cl_M->Ap);
            kernel_sparse_collect.setArg(3, p_cl_M->Aj);
            kernel_sparse_collect.setArg(4, p_cl_M->Ax);
            kernel_sparse_collect.setArg(5, p_cl_mask->Ax);
            kernel_sparse_collect.setArg(6, cl_prodi);
            kernel_sparse_collect.setArg(7, cl_prodx);
            kernel_sparse_collect.setArg(8, cl_prods_offset.buffer());
            kernel_sparse_collect.setArg(9, p_cl_v->values);

            cl::NDRange collect_global(m_block_size * n_groups_to_dispatch_v);
            cl::NDRange collect_local(m_block_size);
            CL_DISPATCH_PROFILED("collect", queue, kernel_sparse_collect, cl::NDRange(), collect_global, collect_local);

            CL_PROFILE_BEGIN("sort", queue)
            const uint max_key    = r->get_n_rows() - 1;
            const uint n_elements = prods_count;
            cl_sort_by_key<T>(queue, cl_prodi, cl_prodx, n_elements, p_tmp_alloc, max_key);
            CL_PROFILE_END();

            uint       reduced_size;
            cl::Buffer reduced_keys;
            cl::Buffer reduced_values;

            CL_PROFILE_BEGIN("reduce", queue)
            cl_reduce_by_key(queue, cl_prodi, cl_prodx, prods_count, reduced_keys, reduced_values, reduced_size, op_add, p_tmp_alloc);
            CL_PROFILE_END();

            p_cl_r->Ai     = reduced_keys;
            p_cl_r->Ax     = reduced_values;
            p_cl_r->values = reduced_size;

            p_tmp_alloc->free_all();

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
                    .set_name("vxm")
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .add_op("OP_BINARY2", op_add.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .set_source(source_vxm)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

    private:
        uint m_block_size  = 0;
        uint m_block_count = 0;
    };

}// namespace spla

#endif//SPLA_CL_VXM_HPP
