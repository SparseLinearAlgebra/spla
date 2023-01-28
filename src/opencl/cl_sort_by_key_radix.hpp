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

#ifndef SPLA_CL_SORT_BY_KEY_RADIX_HPP
#define SPLA_CL_SORT_BY_KEY_RADIX_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_prefix_sum.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_sort_radix.hpp>

#include <cmath>

namespace spla {

    template<typename T>
    void cl_sort_by_key_radix(cl::CommandQueue& queue, cl::Buffer& keys, cl::Buffer& values, uint n, uint max_key = 0xffffffff) {
        if (n <= 1) {
            LOG_MSG(Status::Ok, "nothing to do");
            return;
        }

        const uint BITS_COUNT = 2;
        const uint BITS_VALS  = 1 << BITS_COUNT;
        const uint BITS_MASK  = BITS_VALS - 1;

        auto*      cl_acc     = get_acc_cl();
        const uint block_size = cl_acc->get_default_wgz();

        CLProgramBuilder builder;
        builder.set_name("radix_sort")
                .add_define("BLOCK_SIZE", block_size)
                .add_define("BITS_VALS", BITS_VALS)
                .add_define("BITS_MASK", BITS_MASK)
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .set_source(source_sort_radix);

        if (!builder.build()) return;

        const uint n_treads_total = align(n, block_size);
        const uint n_groups       = div_up(n, block_size);
        const uint n_blocks_sizes = n_groups * BITS_VALS;

        cl::Buffer cl_temp_keys(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * n);
        cl::Buffer cl_temp_values(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * n);
        cl::Buffer cl_offsets(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * n);
        cl::Buffer cl_blocks_size(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * n_blocks_sizes);

        auto kernel_local   = builder.make_kernel("radix_sort_local");
        auto kernel_scatter = builder.make_kernel("radix_sort_scatter");

        const uint bits_in_max_key = static_cast<uint>(std::floor(std::log2(float(max_key)))) + 1;
        const uint bits_aligned    = align(bits_in_max_key, BITS_COUNT);
        const uint max_bits        = std::min(32u, bits_aligned);

        cl::Buffer in_keys    = keys;
        cl::Buffer in_values  = values;
        cl::Buffer out_keys   = cl_temp_keys;
        cl::Buffer out_values = cl_temp_values;

        for (uint shift = 0; shift <= max_bits - BITS_COUNT; shift += BITS_COUNT) {
            cl::NDRange global(n_treads_total);
            cl::NDRange local(block_size);

            kernel_local.setArg(0, in_keys);
            kernel_local.setArg(1, cl_offsets);
            kernel_local.setArg(2, cl_blocks_size);
            kernel_local.setArg(3, n);
            kernel_local.setArg(4, shift);
            queue.enqueueNDRangeKernel(kernel_local, cl::NDRange(), global, local);

            cl_exclusive_scan<uint>(queue, cl_blocks_size, n_blocks_sizes, PLUS_UINT.template cast<TOpBinary<uint, uint, uint>>());

            kernel_scatter.setArg(0, in_keys);
            kernel_scatter.setArg(1, in_values);
            kernel_scatter.setArg(2, out_keys);
            kernel_scatter.setArg(3, out_values);
            kernel_scatter.setArg(4, cl_offsets);
            kernel_scatter.setArg(5, cl_blocks_size);
            kernel_scatter.setArg(6, n);
            kernel_scatter.setArg(7, shift);
            queue.enqueueNDRangeKernel(kernel_scatter, cl::NDRange(), global, local);

            std::swap(in_keys, out_keys);
            std::swap(in_values, out_values);
        }

        keys   = in_keys;
        values = in_values;
    }

}// namespace spla

#endif//SPLA_CL_SORT_BY_KEY_RADIX_HPP
