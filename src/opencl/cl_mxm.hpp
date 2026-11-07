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

#ifndef SPLA_CL_MXM_HPP
#define SPLA_CL_MXM_HPP

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
#include <opencl/cl_fill.hpp>
#include <opencl/cl_format_csr.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_prefix_sum.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/cl_reduce_by_key.hpp>
#include <opencl/cl_sort_by_key.hpp>
#include <opencl/generated/auto_mxm.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace spla {

    namespace mxm_detail {

        constexpr uint   THREADS                    = 256;
        constexpr uint   INPUT_ELEMENTS_PER_THREAD  = 4;
        constexpr uint   RETAIN_ELEMENTS_PER_THREAD = 4;
        constexpr uint   ACCUMULATION_CAPACITY      = 2048;
        constexpr uint   FLUSH_THRESHOLD            = 1024;
        constexpr uint   DIRECT_ROW_THRESHOLD       = 1024;
        constexpr uint   HEAVY_DIRECT_THRESHOLD     = 4096;
        constexpr uint   MAX_CHUNKS_TO_MERGE        = 16;
        constexpr uint   POOL_RETRIES               = 3;
        constexpr double POOL_OVERALLOC             = 1.1;
        constexpr double MIN_EXPECTED_NNZ           = 10000000.0;
        constexpr double MAX_EXPECTED_NNZ           = 500000000.0;

        /**
         * Statistical estimate of the number of chunk (col, value) entries that
         * the multiplication will produce, including intermediate chunks and the
         * final output. Follows the GALATIC pool estimate from
         * `GALATIC/source/device/Multiply.cuh`.
         */
        inline std::size_t estimate_pool_elements(uint a_rows, uint a_nnz, uint b_rows, uint b_nnz, uint b_cols) {
            const double a_avg   = static_cast<double>(a_nnz) / static_cast<double>(a_rows);
            const double b_avg   = static_cast<double>(b_nnz) / static_cast<double>(b_rows);
            const double overlap = b_avg / static_cast<double>(b_cols);

            const double output_estimate =
                    POOL_OVERALLOC * a_rows * b_avg * (1.0 - std::pow(1.0 - overlap, a_avg)) / overlap;

            double single_chunk = b_avg;
            double cur_overlap  = overlap;
            double merges       = 1.0;
            for (; merges < a_avg + 1.0; merges += 1.0) {
                if (single_chunk >= static_cast<double>(RETAIN_ELEMENTS_PER_THREAD * THREADS)) break;
                single_chunk += (1.0 - cur_overlap) * b_avg;
                cur_overlap = cur_overlap + (1.0 - cur_overlap) * overlap;
            }
            const double intermediate_estimate =
                    POOL_OVERALLOC * a_avg / std::min(merges, a_avg) * single_chunk * a_rows;

            const double expected =
                    std::max(MIN_EXPECTED_NNZ, std::min(MAX_EXPECTED_NNZ, std::max(intermediate_estimate, output_estimate)));

            return static_cast<std::size_t>(expected);
        }

    }// namespace mxm_detail

    template<typename T>
    class Algo_mxm_cl final : public RegistryAlgo {
    public:
        ~Algo_mxm_cl() override = default;

        std::string get_name() override {
            return "mxm";
        }

        std::string get_description() override {
            return "parallel sparse matrix sparse matrix product on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            TIME_PROFILE_SCOPE("opencl/mxm");

            auto t = ctx.task.template cast_safe<ScheduleTask_mxm>();

            ref_ptr<TMatrix<T>>         R           = t->R.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         A           = t->A.template cast_safe<TMatrix<T>>();
            ref_ptr<TMatrix<T>>         B           = t->B.template cast_safe<TMatrix<T>>();
            ref_ptr<TOpBinary<T, T, T>> op_multiply = t->op_multiply.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TOpBinary<T, T, T>> op_add      = t->op_add.template cast_safe<TOpBinary<T, T, T>>();
            ref_ptr<TScalar<T>>         init        = t->init.template cast_safe<TScalar<T>>();

            R->validate_wd(FormatMatrix::AccCsr);
            A->validate_rw(FormatMatrix::AccCsr);
            B->validate_rw(FormatMatrix::AccCsr);

            if (A->get_n_cols() != B->get_n_rows()) {
                return Status::InvalidArgument;
            }

            auto*       p_cl_R = R->template get<CLCsr<T>>();
            const auto* p_cl_A = A->template get<CLCsr<T>>();
            const auto* p_cl_B = B->template get<CLCsr<T>>();

            const uint M = R->get_n_rows();
            const uint K = R->get_n_cols();

            if (K == 0 || p_cl_A->values == 0 || p_cl_B->values == 0) {
                return produce_empty(M, *p_cl_R);
            }

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op_multiply, program)) return Status::CompilationError;

            auto* p_cl_acc = get_acc_cl();
            auto* p_tmp    = p_cl_acc->get_alloc_tmp();
            auto& queue    = p_cl_acc->get_queue_default();

            const T init_val = init->get_value();

            // 1. count total number of symbols
            CLCounterWrapper cl_total;
            CL_COUNTER_SET("init-total", queue, cl_total, 0);

            auto kernel_count = program->make_kernel("mxm_count");
            kernel_count.setArg(0, p_cl_A->Ap);
            kernel_count.setArg(1, p_cl_A->Aj);
            kernel_count.setArg(2, p_cl_B->Ap);
            kernel_count.setArg(3, cl_total.buffer());
            kernel_count.setArg(4, M);
            dispatch_grid(program, queue, kernel_count, M);

            uint total;
            CL_COUNTER_GET("get-total", queue, cl_total, total);

            if (total == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            // 2. collect symbols: composite key = i * K + j, value = op_multiply
            cl::Buffer cl_keys = p_tmp->alloc(total * sizeof(uint));
            cl::Buffer cl_vals = p_tmp->alloc(total * sizeof(T));

            CLCounterWrapper cl_offset;
            CL_COUNTER_SET("init-offset", queue, cl_offset, 0);

            auto kernel_collect = program->make_kernel("mxm_collect");
            kernel_collect.setArg(0, p_cl_A->Ap);
            kernel_collect.setArg(1, p_cl_A->Aj);
            kernel_collect.setArg(2, p_cl_A->Ax);
            kernel_collect.setArg(3, p_cl_B->Ap);
            kernel_collect.setArg(4, p_cl_B->Aj);
            kernel_collect.setArg(5, p_cl_B->Ax);
            kernel_collect.setArg(6, cl_keys);
            kernel_collect.setArg(7, cl_vals);
            kernel_collect.setArg(8, cl_offset.buffer());
            kernel_collect.setArg(9, K);
            kernel_collect.setArg(10, M);
            dispatch_grid(program, queue, kernel_collect, M);

            // 3. sort by composite key
            const uint max_key = M * K - 1;
            cl_sort_by_key<T>(queue, cl_keys, cl_vals, total, p_tmp, max_key);

            // 4. reduce duplicate (row, col) entries with op_add
            uint       usize;
            cl::Buffer cl_ukeys;
            cl::Buffer cl_uvals;
            cl_reduce_by_key(queue, cl_keys, cl_vals, total, cl_ukeys, cl_uvals, usize, op_add, p_tmp);

            if (usize == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            // 5. flag identity entries (value == init) for removal
            cl::Buffer cl_flags = p_tmp->alloc(usize * sizeof(uint));

            CLCounterWrapper cl_keep;
            CL_COUNTER_SET("init-keep", queue, cl_keep, 0);

            auto kernel_flag = program->make_kernel("mxm_identity_flag");
            kernel_flag.setArg(0, cl_uvals);
            kernel_flag.setArg(1, cl_flags);
            kernel_flag.setArg(2, cl_keep.buffer());
            kernel_flag.setArg(3, init_val);
            kernel_flag.setArg(4, usize);
            dispatch_grid(program, queue, kernel_flag, usize);

            uint keep_count;
            CL_COUNTER_GET("get-keep", queue, cl_keep, keep_count);

            if (keep_count == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            // 6. exclusive scan of flags -> compaction offsets
            cl::Buffer cl_offsets = p_tmp->alloc(usize * sizeof(uint));
            queue.enqueueCopyBuffer(cl_flags, cl_offsets, 0, 0, usize * sizeof(uint));
            cl_exclusive_scan<uint>(queue, cl_offsets, usize,
                                    PLUS_UINT.template cast_safe<TOpBinary<uint, uint, uint>>(), p_tmp);

            // 7. scatter non-identity entries to compact arrays
            cl::Buffer cl_okeys = p_tmp->alloc(keep_count * sizeof(uint));
            cl::Buffer cl_ovals = p_tmp->alloc(keep_count * sizeof(T));

            auto kernel_scatter = program->make_kernel("mxm_identity_scatter");
            kernel_scatter.setArg(0, cl_ukeys);
            kernel_scatter.setArg(1, cl_uvals);
            kernel_scatter.setArg(2, cl_offsets);
            kernel_scatter.setArg(3, cl_flags);
            kernel_scatter.setArg(4, cl_okeys);
            kernel_scatter.setArg(5, cl_ovals);
            kernel_scatter.setArg(6, usize);
            dispatch_grid(program, queue, kernel_scatter, usize);

            // 8. write compacted entries into CSR and build row histogram
            cl_csr_resize<T>(M, keep_count, *p_cl_R);
            cl_fill_zero<uint>(queue, p_cl_R->Ap, M + 1);

            auto kernel_write = program->make_kernel("mxm_write");
            kernel_write.setArg(0, cl_okeys);
            kernel_write.setArg(1, cl_ovals);
            kernel_write.setArg(2, p_cl_R->Aj);
            kernel_write.setArg(3, p_cl_R->Ax);
            kernel_write.setArg(4, p_cl_R->Ap);
            kernel_write.setArg(5, K);
            kernel_write.setArg(6, keep_count);
            dispatch_grid(program, queue, kernel_write, keep_count);

            // 9. exclusive scan of row histogram -> Ap
            cl_exclusive_scan<uint>(queue, p_cl_R->Ap, M + 1,
                                    PLUS_UINT.template cast_safe<TOpBinary<uint, uint, uint>>(), p_tmp);

            p_tmp->free_all();

            return Status::Ok;
        }

    private:
        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op_multiply, std::shared_ptr<CLProgram>& program) {
            m_block_size = get_acc_cl()->get_default_wgs();

            CLProgramBuilder program_builder;
            program_builder
                    .set_name("mxm")
                    .add_define("BLOCK_SIZE", m_block_size)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY1", op_multiply.template as<OpBinary>())
                    .set_source(source_mxm)
                    .acquire();

            program = program_builder.get_program();

            return true;
        }

        void dispatch_grid(const std::shared_ptr<CLProgram>& program,
                           cl::CommandQueue&                 queue,
                           cl::Kernel&                       kernel,
                           uint                              n) {
            uint n_groups = std::max(std::min((n + m_block_size - 1) / m_block_size, uint(1024)), uint(1));

            cl::NDRange exec_global(m_block_size * n_groups);
            cl::NDRange exec_local(m_block_size);
            CL_DISPATCH_PROFILED("dispatch", queue, kernel, cl::NDRange(), exec_global, exec_local);
        }

        Status produce_empty(std::size_t n_rows, CLCsr<T>& storage) {
            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            cl_csr_resize<T>(n_rows, 0, storage);
            cl_fill_zero<uint>(queue, storage.Ap, static_cast<uint>(n_rows + 1));

            return Status::Ok;
        }

    private:
        uint m_block_size = 0;
    };

}// namespace spla

#endif//SPLA_CL_MXM_HPP
