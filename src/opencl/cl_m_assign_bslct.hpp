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

#ifndef SPLA_CL_M_ASSIGN_BSLCT_HPP
#define SPLA_CL_M_ASSIGN_BSLCT_HPP

#include <iostream>
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
#include <opencl/generated/auto_m_assign_bslct.hpp>

#include <sstream>

namespace spla {

    template<typename T>
    class Algo_m_assign_bslct_masked_cl final : public RegistryAlgo {
    public:
        ~Algo_m_assign_bslct_masked_cl() override = default;

        std::string get_name() override {
            return "m_assign_bslct_masked";
        }

        std::string get_description() override {
            return "parallel matrix masked assignment on opencl device";
        }

        Status execute(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/m_assign_bslct");

            auto t = ctx.task.template cast_safe<ScheduleTask_m_assign_bslct_masked>();

            auto r             = t->r.template cast_safe<TMatrix<T>>();
            auto mask          = t->mask.template cast_safe<TVector<T>>();
            auto value         = t->value.template cast_safe<TScalar<T>>();
            auto op_assign     = t->op_assign.template cast_safe<TOpBinary<T, T, T>>();
            auto op_select_bin = t->op_select_bin.template cast_safe<TOpSelectBinary<T>>();

            r->validate_rwd(FormatMatrix::AccCsr);
            mask->validate_rw(FormatVector::AccDense);

            auto*       p_cl_r    = r->template get<CLCsr<T>>();
            const auto* p_cl_mask = mask->template get<CLDenseVec<T>>();
            auto*       p_cl_acc  = get_acc_cl();
            auto&       queue     = p_cl_acc->get_queue_default();

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_assign, op_select_bin, program)) return Status::CompilationError;

            auto kernel = program->make_kernel("assign_bslct_csr");
            kernel.setArg(0, p_cl_r->Ap);
            kernel.setArg(1, p_cl_r->Aj);
            kernel.setArg(2, p_cl_mask->Ax);
            kernel.setArg(3, p_cl_r->Ax);
            kernel.setArg(4, value->get_value());
            kernel.setArg(5, r->get_n_rows());

            uint n_groups_to_dispatch = div_up_clamp(r->get_n_rows(), m_block_count, 1, 1024);

            cl::NDRange global(m_block_count * n_groups_to_dispatch, m_block_size);
            cl::NDRange local(m_block_count, m_block_size);
            queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_assign, const ref_ptr<TOpSelectBinary<T>>& op_select_bin, std::shared_ptr<CLProgram>& program) {
            m_block_size  = get_acc_cl()->get_default_wgs();
            m_block_count = 1;

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("m_assign_bslct")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op_assign.template as<OpBinary>())
                    .add_op("OP_SELECT_BIN", op_select_bin.template as<OpSelectBinary>())
                    .set_source(source_m_assign_bslct)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

    private:
        uint m_block_size  = 0;
        uint m_block_count = 0;
    };

}// namespace spla

#endif//SPLA_CL_M_ASSIGN_BSLCT_HPP
