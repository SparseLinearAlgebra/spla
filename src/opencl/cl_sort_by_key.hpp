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

#ifndef SPLA_CL_SORT_BY_KEY_HPP
#define SPLA_CL_SORT_BY_KEY_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_sort_bitonic.hpp>

namespace spla {

    template<typename T>
    void cl_sort_by_key(cl::CommandQueue& queue, cl::Buffer& keys, cl::Buffer& values, uint size) {
        if (size <= 1) {
            return;
        }

        const uint local_size = 1024 * 4;

        CLProgramBuilder builder;
        builder.set_name("sort_bitonic")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_define("BITONIC_SORT_LOCAL_BUFFER_SIZE", local_size)
                .set_source(source_sort_bitonic);

        if (!builder.build()) return;

        auto* acc = get_acc_cl();

        cl::NDRange global(acc->get_max_wgs());
        cl::NDRange local(acc->get_max_wgs());

        if (size <= local_size) {
            auto kernel_local = builder.make_kernel("bitonic_sort_local");
            kernel_local.setArg(0, keys);
            kernel_local.setArg(1, values);
            kernel_local.setArg(2, size);
            queue.enqueueNDRangeKernel(kernel_local, cl::NDRange(), global, local);
        } else {
            auto kernel_global = builder.make_kernel("bitonic_sort_global");
            kernel_global.setArg(0, keys);
            kernel_global.setArg(1, values);
            kernel_global.setArg(2, size);
            queue.enqueueNDRangeKernel(kernel_global, cl::NDRange(), global, local);
        }
    }

}// namespace spla

#endif//SPLA_CL_SORT_BY_KEY_HPP
