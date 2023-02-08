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

#ifndef SPLA_CL_REDUCE_BY_KEY_HPP
#define SPLA_CL_REDUCE_BY_KEY_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_prefix_sum.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_reduce_by_key.hpp>
#include <spla/timer.hpp>

namespace spla {

    template<typename T>
    void cl_reduce_by_key(cl::CommandQueue& queue,
                          const cl::Buffer& keys, const cl::Buffer& values, const uint size,
                          cl::Buffer& unique_keys, cl::Buffer& reduce_values, uint& reduced_size,
                          const ref_ptr<TOpBinary<T, T, T>>& reduce_op) {
        auto* cl_acc = get_acc_cl();

        CLProgramBuilder builder;
        builder.set_name("reduce_by_key")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_define("BLOCK_SIZE", cl_acc->get_max_wgs())
                .add_op("OP_BINARY", reduce_op.template as<OpBinary>())
                .set_source(source_reduce_by_key)
                .acquire();

        const uint block_size        = cl_acc->get_default_wgz();
        const uint sequential_switch = 32;
        const uint small_switch      = cl_acc->get_max_wgs();

        if (size == 0) {
            reduced_size = 0;
            return;
        }
        if (size == 1) {
            reduced_size  = 1;
            unique_keys   = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * reduced_size);
            reduce_values = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * reduced_size);
            queue.enqueueCopyBuffer(keys, unique_keys, 0, 0, sizeof(uint) * reduced_size);
            queue.enqueueCopyBuffer(values, reduce_values, 0, 0, sizeof(T) * reduced_size);
            return;
        }
        if (size <= sequential_switch) {
            cl::Buffer cl_reduced_count(cl_acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(uint));
            unique_keys   = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * size);
            reduce_values = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * size);

            auto kernel_sequential = builder.make_kernel("reduce_by_key_sequential");
            kernel_sequential.setArg(0, keys);
            kernel_sequential.setArg(1, values);
            kernel_sequential.setArg(2, unique_keys);
            kernel_sequential.setArg(3, reduce_values);
            kernel_sequential.setArg(4, cl_reduced_count);
            kernel_sequential.setArg(5, size);

            cl::NDRange global(cl_acc->get_wave_size());
            cl::NDRange local(cl_acc->get_wave_size());
            queue.enqueueNDRangeKernel(kernel_sequential, cl::NDRange(), global, local);
            queue.enqueueReadBuffer(cl_reduced_count, true, 0, sizeof(uint), &reduced_size);
            return;
        }
        if (size <= small_switch) {
            cl::Buffer cl_reduced_count(cl_acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(uint));
            unique_keys   = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * size);
            reduce_values = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * size);

            auto kernel_small = builder.make_kernel("reduce_by_key_small");
            kernel_small.setArg(0, keys);
            kernel_small.setArg(1, values);
            kernel_small.setArg(2, unique_keys);
            kernel_small.setArg(3, reduce_values);
            kernel_small.setArg(4, cl_reduced_count);
            kernel_small.setArg(5, size);

            cl::NDRange global(align(size, cl_acc->get_wave_size()));
            cl::NDRange local = global;
            queue.enqueueNDRangeKernel(kernel_small, cl::NDRange(), global, local);
            queue.enqueueReadBuffer(cl_reduced_count, true, 0, sizeof(uint), &reduced_size);
            return;
        }

        cl::Buffer offsets(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * size);

        auto kernel_gen_offsets = builder.make_kernel("reduce_by_key_generate_offsets");
        kernel_gen_offsets.setArg(0, keys);
        kernel_gen_offsets.setArg(1, offsets);
        kernel_gen_offsets.setArg(2, size);

        cl::NDRange gen_offsets_global(align(size, block_size));
        cl::NDRange gen_offsets_local(block_size);
        queue.enqueueNDRangeKernel(kernel_gen_offsets, cl::NDRange(), gen_offsets_global, gen_offsets_local);

        cl_exclusive_scan(queue, offsets, size, PLUS_UINT.template cast<TOpBinary<uint, uint, uint>>());

        uint       scan_last;
        cl::Buffer cl_scan_last(cl_acc->get_context(), CL_MEM_HOST_READ_ONLY, sizeof(scan_last));
        queue.enqueueCopyBuffer(offsets, cl_scan_last, sizeof(uint) * (size - 1), 0, sizeof(scan_last));
        queue.enqueueReadBuffer(cl_scan_last, true, 0, sizeof(scan_last), &scan_last);

        reduced_size  = scan_last + 1;
        unique_keys   = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint) * reduced_size);
        reduce_values = cl::Buffer(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * reduced_size);

        auto kernel_reduce_scalar = builder.make_kernel("reduce_by_key_scalar");
        kernel_reduce_scalar.setArg(0, keys);
        kernel_reduce_scalar.setArg(1, values);
        kernel_reduce_scalar.setArg(2, offsets);
        kernel_reduce_scalar.setArg(3, unique_keys);
        kernel_reduce_scalar.setArg(4, reduce_values);
        kernel_reduce_scalar.setArg(5, size);
        kernel_reduce_scalar.setArg(6, reduced_size);

        cl::NDRange reduce_naive_global(align(reduced_size, block_size));
        cl::NDRange reduce_naive_local(block_size);
        queue.enqueueNDRangeKernel(kernel_reduce_scalar, cl::NDRange(), reduce_naive_global, reduce_naive_local);
    }

}// namespace spla

#endif//SPLA_CL_REDUCE_BY_KEY_HPP
