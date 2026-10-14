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

#ifndef SPLA_CL_MERGE_HPP
#define SPLA_CL_MERGE_HPP

#include <core/top.hpp>
#include <core/ttype.hpp>

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_alloc.hpp>
#include <opencl/cl_counter.hpp>
#include <opencl/cl_fill.hpp>
#include <opencl/cl_prefix_sum.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_merge.hpp>

namespace spla {

    template<typename T>
    uint cl_merge_intersect(cl::CommandQueue&                  queue,
                            const cl::Buffer&                  a_keys,
                            const cl::Buffer&                  a_vals,
                            const uint                         a_size,
                            const cl::Buffer&                  b_keys,
                            const cl::Buffer&                  b_vals,
                            const uint                         b_size,
                            const cl::Buffer&                  r_keys_ai,
                            const cl::Buffer&                  r_keys_ax,
                            const cl::Buffer&                  r_vals_ai,
                            const cl::Buffer&                  r_vals_ax,
                            const ref_ptr<TOpBinary<T, T, T>>& op,
                            CLAlloc*                           tmp_alloc) {
        const uint tile_size = 128;

        CLProgramBuilder builder;
        builder.set_name("merge")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_op("OP_BINARY", op.template as<OpBinary>())
                .set_source(source_merge)
                .acquire();

        auto* cl_acc = get_acc_cl();

        const uint count_tile = div_up(a_size, tile_size);
        const uint wgs        = cl_acc->get_default_wgs();

        cl::Buffer cl_tile_counts = tmp_alloc->alloc(sizeof(uint) * (count_tile + 1));

        cl_fill_zero<uint>(queue, cl_tile_counts, count_tile + 1);

        const uint        global_size = align(count_tile, wgs);
        const cl::NDRange global(global_size);
        const cl::NDRange local(wgs);

        auto kernel_count = builder.make_kernel("merge_intersect_count");
        kernel_count.setArg(0, a_keys);
        kernel_count.setArg(1, a_size);
        kernel_count.setArg(2, b_keys);
        kernel_count.setArg(3, b_size);
        kernel_count.setArg(4, cl_tile_counts);
        kernel_count.setArg(5, tile_size);
        queue.enqueueNDRangeKernel(kernel_count, cl::NDRange(), global, local);

        cl_exclusive_scan<uint>(queue, cl_tile_counts, count_tile + 1,
                                PLUS_UINT.template cast_safe<TOpBinary<uint, uint, uint>>(), tmp_alloc);

        auto kernel_scatter = builder.make_kernel("merge_intersect_scatter");
        kernel_scatter.setArg(0, a_keys);
        kernel_scatter.setArg(1, a_vals);
        kernel_scatter.setArg(2, a_size);
        kernel_scatter.setArg(3, b_keys);
        kernel_scatter.setArg(4, b_vals);
        kernel_scatter.setArg(5, b_size);
        kernel_scatter.setArg(6, cl_tile_counts);
        kernel_scatter.setArg(7, r_keys_ai);
        kernel_scatter.setArg(8, r_keys_ax);
        kernel_scatter.setArg(9, r_vals_ai);
        kernel_scatter.setArg(10, r_vals_ax);
        kernel_scatter.setArg(11, tile_size);
        queue.enqueueNDRangeKernel(kernel_scatter, cl::NDRange(), global, local);

        CLCounterWrapper cl_total;
        queue.enqueueCopyBuffer(cl_tile_counts, cl_total.buffer(), sizeof(uint) * count_tile, 0, sizeof(uint));
        return cl_total.get(queue);
    }

}// namespace spla

#endif//SPLA_CL_MERGE_HPP
