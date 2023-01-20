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
#include <opencl/cl_program_builder.hpp>
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
            auto                t    = ctx.task.template cast<ScheduleTask_v_assign_masked>();
            ref_ptr<TVector<T>> mask = t->mask.template cast<TVector<T>>();

            if (mask->is_valid(Format::CLCooVec))
                return execute_sp2dn(ctx);
            if (mask->is_valid(Format::CLDenseVec))
                return execute_dn2dn(ctx);

            return execute_sp2dn(ctx);
        }

    private:
        Status execute_dn2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE(assign, "opencl/vector_assign_dense2dense");

            auto t = ctx.task.template cast<ScheduleTask_v_assign_masked>();

            auto r         = t->r.template cast<TVector<T>>();
            auto mask      = t->mask.template cast<TVector<T>>();
            auto value     = t->value.template cast<TScalar<T>>();
            auto op_assign = t->op_assign.template cast<TOpBinary<T, T, T>>();
            auto op_select = t->op_select.template cast<TOpSelect<T>>();

            r->validate_rwd(Format::CLDenseVec);
            mask->validate_rw(Format::CLDenseVec);
            if (!ensure_kernel(op_assign, op_select)) return Status::Error;

            auto*       p_cl_r_dense    = r->template get<CLDenseVec<T>>();
            const auto* p_cl_mask_dense = mask->template get<CLDenseVec<T>>();
            auto*       p_cl_acc        = get_acc_cl();
            auto&       queue           = p_cl_acc->get_queue_default();

            m_kernel_dense_to_dense.setArg(0, p_cl_r_dense->Ax);
            m_kernel_dense_to_dense.setArg(1, p_cl_mask_dense->Ax);
            m_kernel_dense_to_dense.setArg(2, value->get_value());
            m_kernel_dense_to_dense.setArg(3, r->get_n_rows());

            uint n_groups_to_dispatch = std::max(std::min(r->get_n_rows() / m_block_size, uint(256)), uint(1));

            cl::NDRange global(m_block_size * n_groups_to_dispatch);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel_dense_to_dense, cl::NDRange(), global, local);
            queue.finish();

            return Status::Ok;
        }

        Status execute_sp2dn(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE(assign, "opencl/vector_assign_sparse2dense");

            auto t = ctx.task.template cast<ScheduleTask_v_assign_masked>();

            auto r         = t->r.template cast<TVector<T>>();
            auto mask      = t->mask.template cast<TVector<T>>();
            auto value     = t->value.template cast<TScalar<T>>();
            auto op_assign = t->op_assign.template cast<TOpBinary<T, T, T>>();
            auto op_select = t->op_select.template cast<TOpSelect<T>>();

            r->validate_rwd(Format::CLDenseVec);
            mask->validate_rw(Format::CLCooVec);
            if (!ensure_kernel(op_assign, op_select)) return Status::Error;

            auto*       p_cl_r_dense  = r->template get<CLDenseVec<T>>();
            const auto* p_cl_mask_coo = mask->template get<CLCooVec<T>>();
            auto*       p_cl_acc      = get_acc_cl();
            auto&       queue         = p_cl_acc->get_queue_default();

            m_kernel_sparse_to_dense.setArg(0, p_cl_r_dense->Ax);
            m_kernel_sparse_to_dense.setArg(1, p_cl_mask_coo->Ai);
            m_kernel_sparse_to_dense.setArg(2, p_cl_mask_coo->Ax);
            m_kernel_sparse_to_dense.setArg(3, value->get_value());
            m_kernel_sparse_to_dense.setArg(4, p_cl_mask_coo->values);

            uint n_groups_to_dispatch = std::max(std::min(p_cl_mask_coo->values / m_block_size, uint(256)), uint(1));

            cl::NDRange global(m_block_size * n_groups_to_dispatch);
            cl::NDRange local(m_block_size);
            queue.enqueueNDRangeKernel(m_kernel_sparse_to_dense, cl::NDRange(), global, local);
            queue.finish();

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_assign, const ref_ptr<TOpSelect<T>>& op_select) {
            if (m_compiled) return true;

            m_block_size = get_acc_cl()->get_wave_size();

            CLProgramBuilder program_builder;
            program_builder
                    .set_key("vector_assign")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op_assign.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .add_code(source_vector_assign);

            if (!program_builder.build()) return false;

            m_program                = program_builder.get_program();
            m_kernel_dense_to_dense  = m_program->make_kernel("assign_dense_to_dense");
            m_kernel_sparse_to_dense = m_program->make_kernel("assign_sparse_to_dense");
            m_compiled               = true;

            return true;
        }

        std::shared_ptr<CLProgram> m_program;
        cl::Kernel                 m_kernel_dense_to_dense;
        cl::Kernel                 m_kernel_sparse_to_dense;
        uint                       m_block_size = 0;
        bool                       m_compiled   = false;
    };

}// namespace spla

#endif//SPLA_CL_VECTOR_ASSIGN_HPP
