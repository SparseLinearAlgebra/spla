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

#ifndef SPLA_CL_REDUCE_HPP
#define SPLA_CL_REDUCE_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_reduce.hpp>

namespace spla {

    template<typename T>
    void cl_reduce(cl::CommandQueue& queue, const cl::Buffer& values, uint n, T init, const ref_ptr<TOpBinary<T, T, T>>& op_reduce, T& result) {
        if (n == 0) {
            result = init;
            return;
        }
        if (n == 1) {
            cl::Buffer cl_result(get_acc_cl()->get_context(), CL_MEM_HOST_READ_ONLY, sizeof(T));
            queue.enqueueCopyBuffer(values, cl_result, 0, 0, sizeof(T));
            queue.enqueueReadBuffer(cl_result, true, 0, sizeof(T), &result);
            return;
        }

        auto*      cl_acc               = get_acc_cl();
        const uint max_block_size       = 1024;
        const uint max_small_block_size = 128u;
        const uint block_size           = std::min(max_block_size, get_acc_cl()->get_max_wgs());
        const uint block_size_small     = std::min(block_size, max_small_block_size);

        cl::Buffer cl_sum(cl_acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(T));

        if (n <= block_size) {
            CLProgramBuilder builder;
            builder.set_name("reduce_small")
                    .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                    .add_define("BLOCK_SIZE", block_size_small)
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_op("OP_BINARY", op_reduce.template as<OpBinary>())
                    .set_source(source_reduce)
                    .acquire();

            auto kernel = builder.make_kernel("reduce");
            kernel.setArg(0, values);
            kernel.setArg(1, cl_sum);
            kernel.setArg(2, init);
            kernel.setArg(3, n);

            cl::NDRange global(block_size_small);
            cl::NDRange local(block_size_small);
            queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);
            queue.enqueueReadBuffer(cl_sum, true, 0, sizeof(result), &result);
            return;
        }

        CLProgramBuilder builder;
        builder.set_name("reduce_wide")
                .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                .add_define("BLOCK_SIZE", block_size)
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_op("OP_BINARY", op_reduce.template as<OpBinary>())
                .set_source(source_reduce)
                .acquire();

        const uint optimal_split = 64;
        const uint groups_count  = div_up_clamp(n, block_size, 1, optimal_split);

        cl::Buffer cl_sum_group(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(T) * groups_count);

        auto kernel_phase_1 = builder.make_kernel("reduce");
        kernel_phase_1.setArg(0, values);
        kernel_phase_1.setArg(1, cl_sum_group);
        kernel_phase_1.setArg(2, init);
        kernel_phase_1.setArg(3, n);

        cl::NDRange global_phase_1(block_size * groups_count);
        cl::NDRange local_phase_1(block_size);
        queue.enqueueNDRangeKernel(kernel_phase_1, cl::NDRange(), global_phase_1, local_phase_1);

        if (groups_count == 1) {
            queue.enqueueReadBuffer(cl_sum_group, true, 0, sizeof(result), &result);
            return;
        }

        auto kernel_phase_2 = builder.make_kernel("reduce");
        kernel_phase_2.setArg(0, cl_sum_group);
        kernel_phase_2.setArg(1, cl_sum);
        kernel_phase_2.setArg(2, init);
        kernel_phase_2.setArg(3, groups_count);

        cl::NDRange global_phase_2(block_size);
        cl::NDRange local_phase_2(block_size);
        queue.enqueueNDRangeKernel(kernel_phase_2, cl::NDRange(), global_phase_2, local_phase_2);
        queue.enqueueReadBuffer(cl_sum, true, 0, sizeof(result), &result);
    }

}// namespace spla

#endif//SPLA_CL_REDUCE_HPP
