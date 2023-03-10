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

#ifndef SPLA_CL_MXMT_MASKED_HPP
#define SPLA_CL_MXMT_MASKED_HPP

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
#include <opencl/generated/auto_mxmT_masked.hpp>

#include <algorithm>
#include <sstream>

namespace spla {

    template<typename T>
    class Algo_mxmT_masked_cl final : public RegistryAlgo {
    public:
        ~Algo_mxmT_masked_cl() override = default;

        std::string get_name() override {
            return "mxmT_masked";
        }

        std::string get_description() override {
            return "parallel masked matrix matrix-transposed product on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("opencl/mxmT_masked");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxmT_masked>();

            ref_ptr<TMatrix<T>>         R           = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         mask        = t->mask.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         A           = t->A.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         B           = t->B.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpSelect<T>>       op_select   = t->op_select.template cast_safe<TOpSelect<T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            R->validate_wd(FormatMatrix::AccCsr);
            mask->validate_rw(FormatMatrix::AccCsr);
            A->validate_rw(FormatMatrix::AccCsr);
            B->validate_rw(FormatMatrix::AccCsr);

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, op_add, op_select, program)) return Status::CompilationError;

            auto*       p_cl_R    = R->template get<CLCsr<T>>();
            const auto* p_cl_mask = mask->template get<CLCsr<T>>();
            const auto* p_cl_A    = A->template get<CLCsr<T>>();
            const auto* p_cl_B    = B->template get<CLCsr<T>>();

            if (p_cl_mask->values == 0) {
                return Status::Ok;
            }
            if (p_cl_A->values == 0) {
                return Status::Ok;
            }
            if (p_cl_B->values == 0) {
                return Status::Ok;
            }

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            cl_csr_resize<T>(R->get_n_rows(), p_cl_mask->values, *p_cl_R);
            queue.enqueueCopyBuffer(p_cl_mask->Ap, p_cl_R->Ap, 0, 0, sizeof(uint) * (R->get_n_rows() + 1));
            queue.enqueueCopyBuffer(p_cl_mask->Aj, p_cl_R->Aj, 0, 0, sizeof(uint) * (p_cl_R->values));

            auto kernel = program->make_kernel("mxmT_masked_csr_scalar");
            kernel.setArg(0, p_cl_A->Ap);
            kernel.setArg(1, p_cl_A->Aj);
            kernel.setArg(2, p_cl_A->Ax);
            kernel.setArg(3, p_cl_B->Ap);
            kernel.setArg(4, p_cl_B->Aj);
            kernel.setArg(5, p_cl_B->Ax);
            kernel.setArg(6, p_cl_mask->Ap);
            kernel.setArg(7, p_cl_mask->Aj);
            kernel.setArg(8, p_cl_mask->Ax);
            kernel.setArg(9, p_cl_R->Ax);
            kernel.setArg(10, T(init->get_value()));
            kernel.setArg(11, R->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(R->get_n_rows(), m_block_count, 1, 1024);

            cl::NDRange exec_global(m_block_count * n_groups_to_dispatch, m_block_size);
            cl::NDRange exec_local(m_block_count, m_block_size);
            CL_DISPATCH_PROFILED("exec", queue, kernel, cl::NDRange(), exec_global, exec_local);

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_multiply,
                           const ref_ptr<TOpBinary<T, T, T>>& op_add,
                           const ref_ptr<TOpSelect<T>>&       op_select,
                           std::shared_ptr<CLProgram>&        program) {
            m_block_size  = get_acc_cl()->get_default_wgs();
            m_block_count = 1;

            assert(m_block_count >= 1);
            assert(m_block_size * m_block_count == get_acc_cl()->get_wave_size());

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("mxmT_masked")
                    .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_define("BLOCK_COUNT", m_block_count)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .add_op("OP_BINARY2", op_add.template as<OpBinary>())
                    .add_op("OP_SELECT", op_select.template as<OpSelect>())
                    .set_source(source_mxmT_masked)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

    private:
        uint m_block_size  = 0;
        uint m_block_count = 0;
    };

}// namespace spla

#endif//SPLA_CL_MXMT_MASKED_HPP
