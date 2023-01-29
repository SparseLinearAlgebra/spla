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

#ifndef SPLA_CL_PREFIX_SUM_HPP
#define SPLA_CL_PREFIX_SUM_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_prefix_sum.hpp>

namespace spla {

    template<typename T>
    void cl_exclusive_scan(cl::CommandQueue& queue, cl::Buffer& values, uint n, const ref_ptr<TOpBinary<T, T, T>>& op) {
        auto*      cl_acc           = get_acc_cl();
        const uint block_size       = std::min(cl_acc->get_max_wgs(), uint(256));
        const uint values_per_block = block_size * 2;

        // Note on perf:
        //  - no BC (basic) : 500k values in ms: 1.382, 1.501, 1.461, 1.405, 1.524, 1.354, 1.418, 1.4, 1.43
        //  - no BC (unroll): 500k values in ms: 1.41, 1.402, 1.394, 1.37, 1.4, 1.347, 1.444, 1.408, 1.369
        //  - no BC (basic) : 1M values in ms: 2.727, 2.822, 2.726, 2.83, 2.773, 2.773, 2.841, 2.824, 2.792
        //  - no BC (unroll): 1M values in ms: 2.862, 2.775, 2.776, 2.77, 2.653, 2.677, 2.93, 2.748, 2.693

        CLProgramBuilder builder;
        builder.set_name("prefix_sum")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_define("BLOCK_SIZE", block_size)
                .add_define("WARP_SIZE", cl_acc->get_wave_size())
                .add_define("LM_NUM_MEM_BANKS", cl_acc->get_num_of_mem_banks())
                .add_op("OP_BINARY", op.template as<OpBinary>())
                .set_source(source_prefix_sum)
                .acquire();

        uint       n_groups_to_run = n / values_per_block + (n % values_per_block ? 1 : 0);
        cl::Buffer cl_carry(cl_acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(T) * n_groups_to_run, nullptr);

        auto kernel_prescan = builder.make_kernel("prefix_sum_prescan_unroll");
        kernel_prescan.setArg(0, values);
        kernel_prescan.setArg(1, cl_carry);
        kernel_prescan.setArg(2, n);

        cl::NDRange prescan_global(n_groups_to_run * block_size);
        cl::NDRange prescan_local(block_size);
        queue.enqueueNDRangeKernel(kernel_prescan, cl::NDRange(), prescan_global, prescan_local);

        if (n_groups_to_run > 1) {
            cl_exclusive_scan<T>(queue, cl_carry, n_groups_to_run, op);

            auto kernel_propagate = builder.make_kernel("prefix_sum_propagate");
            kernel_propagate.setArg(0, values);
            kernel_propagate.setArg(1, cl_carry);
            kernel_propagate.setArg(2, n);

            cl::NDRange propagate_global((n_groups_to_run - 1) * values_per_block);
            cl::NDRange propagate_local(block_size);
            queue.enqueueNDRangeKernel(kernel_propagate, cl::NDRange(), propagate_global, propagate_local);
        }
    }

}// namespace spla

#endif//SPLA_CL_PREFIX_SUM_HPP
