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
#include <opencl/cl_merge.hpp>
#include <opencl/cl_prefix_sum.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/cl_reduce_by_key.hpp>
#include <opencl/cl_sort_by_key.hpp>
#include <opencl/generated/auto_mxm.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

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

            const T    init_val      = init->get_value();
            const uint direct_thresh = mxm_detail::DIRECT_ROW_THRESHOLD;

            // 1. count normal and direct products
            CLCounterWrapper cl_normal, cl_direct, cl_chunks;
            CL_COUNTER_SET("init-normal", queue, cl_normal, 0);
            CL_COUNTER_SET("init-direct", queue, cl_direct, 0);
            CL_COUNTER_SET("init-chunks", queue, cl_chunks, 0);

            auto kernel_count = program->make_kernel("mxm_count");
            kernel_count.setArg(0, p_cl_A->Ap);
            kernel_count.setArg(1, p_cl_A->Aj);
            kernel_count.setArg(2, p_cl_B->Ap);
            kernel_count.setArg(3, cl_normal.buffer());
            kernel_count.setArg(4, cl_direct.buffer());
            kernel_count.setArg(5, cl_chunks.buffer());
            kernel_count.setArg(6, direct_thresh);
            kernel_count.setArg(7, M);
            dispatch_grid(queue, kernel_count, M);

            uint normal_total, direct_total, chunk_count;
            CL_COUNTER_GET("get-normal", queue, cl_normal, normal_total);
            CL_COUNTER_GET("get-direct", queue, cl_direct, direct_total);
            CL_COUNTER_GET("get-chunks", queue, cl_chunks, chunk_count);

            if (normal_total == 0 && direct_total == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            // 2. collect normal products (into composite-key scratch) and direct chunks (into D)
            cl::Buffer cl_nkeys = p_tmp->alloc(std::max<std::size_t>(normal_total, 1) * sizeof(uint));
            cl::Buffer cl_nvals = p_tmp->alloc(std::max<std::size_t>(normal_total, 1) * sizeof(T));
            cl::Buffer cl_dkeys = p_tmp->alloc(std::max<std::size_t>(direct_total, 1) * sizeof(uint));
            cl::Buffer cl_dvals = p_tmp->alloc(std::max<std::size_t>(direct_total, 1) * sizeof(T));

            cl::Buffer cl_chunk_row    = p_tmp->alloc(std::max<std::size_t>(chunk_count, 1) * sizeof(uint));
            cl::Buffer cl_chunk_offset = p_tmp->alloc(std::max<std::size_t>(chunk_count, 1) * sizeof(uint));
            cl::Buffer cl_chunk_size   = p_tmp->alloc(std::max<std::size_t>(chunk_count, 1) * sizeof(uint));

            CLCounterWrapper cl_noff, cl_doff, cl_chunk_cnt;
            CL_COUNTER_SET("init-noff", queue, cl_noff, 0);
            CL_COUNTER_SET("init-doff", queue, cl_doff, 0);
            CL_COUNTER_SET("init-chunkcnt", queue, cl_chunk_cnt, 0);

            auto kernel_collect = program->make_kernel("mxm_collect");
            kernel_collect.setArg(0, p_cl_A->Ap);
            kernel_collect.setArg(1, p_cl_A->Aj);
            kernel_collect.setArg(2, p_cl_A->Ax);
            kernel_collect.setArg(3, p_cl_B->Ap);
            kernel_collect.setArg(4, p_cl_B->Aj);
            kernel_collect.setArg(5, p_cl_B->Ax);
            kernel_collect.setArg(6, cl_nkeys);
            kernel_collect.setArg(7, cl_nvals);
            kernel_collect.setArg(8, cl_noff.buffer());
            kernel_collect.setArg(9, cl_dkeys);
            kernel_collect.setArg(10, cl_dvals);
            kernel_collect.setArg(11, cl_doff.buffer());
            kernel_collect.setArg(12, cl_chunk_row);
            kernel_collect.setArg(13, cl_chunk_offset);
            kernel_collect.setArg(14, cl_chunk_size);
            kernel_collect.setArg(15, cl_chunk_cnt.buffer());
            kernel_collect.setArg(16, K);
            kernel_collect.setArg(17, direct_thresh);
            kernel_collect.setArg(18, M);
            dispatch_grid(queue, kernel_collect, M);

            // 3. combine normal products: sort + reduce by key (NormalChunk)
            uint       usize = 0;
            cl::Buffer cl_ukeys;
            cl::Buffer cl_uvals;

            if (normal_total > 0) {
                const uint max_key = M * K - 1;
                cl_sort_by_key<T>(queue, cl_nkeys, cl_nvals, normal_total, p_tmp, max_key);
                cl_reduce_by_key(queue, cl_nkeys, cl_nvals, normal_total, cl_ukeys, cl_uvals, usize, op_add, p_tmp);
            }

            // 4. read back normal row boundaries and direct chunk descriptors
            std::vector<uint> normal_bound(M + 1, 0);
            if (usize > 0) {
                std::vector<uint> ukeys_host(usize);
                queue.enqueueReadBuffer(cl_ukeys, true, 0, usize * sizeof(uint), ukeys_host.data());

                uint idx = 0;
                for (uint i = 0; i < M; ++i) {
                    normal_bound[i] = idx;
                    const uint next = (i + 1) * K;
                    while (idx < usize && ukeys_host[idx] < next) ++idx;
                }
                normal_bound[M] = usize;
            }

            struct DirectChunkDesc {
                uint row;
                uint offset;
                uint size;
            };
            std::vector<DirectChunkDesc> chunks;
            if (chunk_count > 0) {
                std::vector<uint> crow(chunk_count), coff(chunk_count), csize(chunk_count);

                auto read_dev_uints = [&](const cl::Buffer& src, std::vector<uint>& dst) {
                    cl::Buffer staging(get_acc_cl()->get_context(),
                                       CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                       dst.size() * sizeof(uint));
                    queue.enqueueCopyBuffer(src, staging, 0, 0, dst.size() * sizeof(uint));
                    queue.enqueueReadBuffer(staging, true, 0, dst.size() * sizeof(uint), dst.data());
                };

                read_dev_uints(cl_chunk_row, crow);
                read_dev_uints(cl_chunk_offset, coff);
                read_dev_uints(cl_chunk_size, csize);

                chunks.reserve(chunk_count);
                for (uint c = 0; c < chunk_count; ++c) {
                    chunks.push_back({crow[c], coff[c], csize[c]});
                }
                std::sort(chunks.begin(), chunks.end(),
                          [](const DirectChunkDesc& a, const DirectChunkDesc& b) { return a.row < b.row; });
            }

            // 5. merge chunks per row into a single pool (composite keys + values)
            const uint pool_cap  = normal_total + direct_total;
            cl::Buffer pool_keys = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(uint));
            cl::Buffer pool_vals = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(T));

            cl::Buffer t0_keys = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(uint));
            cl::Buffer t0_vals = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(T));
            cl::Buffer t1_keys = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(uint));
            cl::Buffer t1_vals = p_tmp->alloc(static_cast<std::size_t>(pool_cap) * sizeof(T));

            uint chunk_it = 0;
            uint pool_off = 0;

            for (uint i = 0; i < M; ++i) {
                const uint ns = normal_bound[i];
                const uint ne = normal_bound[i + 1];

                std::vector<DirectChunkDesc> row_chunks;
                while (chunk_it < chunk_count && chunks[chunk_it].row == i) {
                    row_chunks.push_back(chunks[chunk_it]);
                    ++chunk_it;
                }

                // assemble sources: normal slice + direct chunks
                struct Source {
                    const cl::Buffer* keys;
                    const cl::Buffer* vals;
                    uint              offset;
                    uint              size;
                };
                std::vector<Source> sources;
                if (ne > ns) {
                    sources.push_back({&cl_ukeys, &cl_uvals, ns, ne - ns});
                }
                for (const auto& dc : row_chunks) {
                    sources.push_back({&cl_dkeys, &cl_dvals, dc.offset, dc.size});
                }

                uint row_size = 0;

                if (sources.size() == 1) {
                    row_size = sources[0].size;
                    if (row_size > 0) {
                        queue.enqueueCopyBuffer(*sources[0].keys, pool_keys, sources[0].offset * sizeof(uint), pool_off * sizeof(uint), row_size * sizeof(uint));
                        queue.enqueueCopyBuffer(*sources[0].vals, pool_vals, sources[0].offset * sizeof(T), pool_off * sizeof(T), row_size * sizeof(T));
                    }
                } else if (sources.size() > 1) {
                    // pairwise merge via cl_merge_union, alternating T0/T1
                    queue.enqueueCopyBuffer(*sources[0].keys, t0_keys, sources[0].offset * sizeof(uint), 0, sources[0].size * sizeof(uint));
                    queue.enqueueCopyBuffer(*sources[0].vals, t0_vals, sources[0].offset * sizeof(T), 0, sources[0].size * sizeof(T));

                    uint cur_size  = sources[0].size;
                    bool cur_in_t0 = true;

                    for (std::size_t s = 1; s < sources.size(); ++s) {
                        const Source& src = sources[s];
                        if (cur_in_t0) {
                            cur_size  = cl_merge_union<T>(queue, t0_keys, t0_vals, 0, cur_size,
                                                          *src.keys, *src.vals, src.offset, src.size,
                                                          t1_keys, t1_vals, 0, op_add, p_tmp);
                            cur_in_t0 = false;
                        } else {
                            cur_size  = cl_merge_union<T>(queue, t1_keys, t1_vals, 0, cur_size,
                                                          *src.keys, *src.vals, src.offset, src.size,
                                                          t0_keys, t0_vals, 0, op_add, p_tmp);
                            cur_in_t0 = true;
                        }
                    }

                    row_size = cur_size;
                    if (row_size > 0) {
                        const cl::Buffer& src_keys = cur_in_t0 ? t0_keys : t1_keys;
                        const cl::Buffer& src_vals = cur_in_t0 ? t0_vals : t1_vals;
                        queue.enqueueCopyBuffer(src_keys, pool_keys, 0, pool_off * sizeof(uint), row_size * sizeof(uint));
                        queue.enqueueCopyBuffer(src_vals, pool_vals, 0, pool_off * sizeof(T), row_size * sizeof(T));
                    }
                }

                pool_off += row_size;
            }

            if (pool_off == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            // 6. drop identity entries
            cl::Buffer       cl_flags = p_tmp->alloc(static_cast<std::size_t>(pool_off) * sizeof(uint));
            CLCounterWrapper cl_keep;
            CL_COUNTER_SET("init-keep", queue, cl_keep, 0);

            auto kernel_flag = program->make_kernel("mxm_identity_flag");
            kernel_flag.setArg(0, pool_vals);
            kernel_flag.setArg(1, cl_flags);
            kernel_flag.setArg(2, cl_keep.buffer());
            kernel_flag.setArg(3, init_val);
            kernel_flag.setArg(4, pool_off);
            dispatch_grid(queue, kernel_flag, pool_off);

            uint keep_count;
            CL_COUNTER_GET("get-keep", queue, cl_keep, keep_count);

            if (keep_count == 0) {
                p_tmp->free_all();
                return produce_empty(M, *p_cl_R);
            }

            cl::Buffer cl_offsets = p_tmp->alloc(static_cast<std::size_t>(pool_off) * sizeof(uint));
            queue.enqueueCopyBuffer(cl_flags, cl_offsets, 0, 0, static_cast<std::size_t>(pool_off) * sizeof(uint));
            cl_exclusive_scan<uint>(queue, cl_offsets, pool_off,
                                    PLUS_UINT.template cast_safe<TOpBinary<uint, uint, uint>>(), p_tmp);

            cl::Buffer cl_okeys = p_tmp->alloc(static_cast<std::size_t>(keep_count) * sizeof(uint));
            cl::Buffer cl_ovals = p_tmp->alloc(static_cast<std::size_t>(keep_count) * sizeof(T));

            auto kernel_scatter = program->make_kernel("mxm_identity_scatter");
            kernel_scatter.setArg(0, pool_keys);
            kernel_scatter.setArg(1, pool_vals);
            kernel_scatter.setArg(2, cl_offsets);
            kernel_scatter.setArg(3, cl_flags);
            kernel_scatter.setArg(4, cl_okeys);
            kernel_scatter.setArg(5, cl_ovals);
            kernel_scatter.setArg(6, pool_off);
            dispatch_grid(queue, kernel_scatter, pool_off);

            // 7. write compacted entries into CSR and build row histogram
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
            dispatch_grid(queue, kernel_write, keep_count);

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

        void dispatch_grid(cl::CommandQueue& queue, cl::Kernel& kernel, uint n) {
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
