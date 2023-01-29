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

#ifndef SPLA_CL_FILL_HPP
#define SPLA_CL_FILL_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_fill.hpp>

namespace spla {

    template<typename T>
    void cl_fill_zero(cl::CommandQueue& queue, cl::Buffer& values, uint n) {
        CLProgramBuilder builder;
        builder.set_name("fill")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .set_source(source_fill)
                .acquire();

        auto  fill_zero = builder.make_kernel("fill_zero");
        auto* acc       = get_acc_cl();

        uint block_size           = acc->get_wave_size();
        uint n_groups_to_dispatch = std::max(std::min(n / block_size, uint(512)), uint(1));

        cl::NDRange global(block_size * n_groups_to_dispatch);
        cl::NDRange local(block_size);

        fill_zero.setArg(0, values);
        fill_zero.setArg(1, n);
        queue.enqueueNDRangeKernel(fill_zero, cl::NDRange(), global, local);
    }

    template<typename T>
    void cl_fill_value(cl::CommandQueue& queue, const cl::Buffer& values, uint n, T value) {
        CLProgramBuilder builder;
        builder.set_name("fill")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .set_source(source_fill)
                .acquire();

        auto  fill_value = builder.make_kernel("fill_value");
        auto* acc        = get_acc_cl();

        uint block_size           = acc->get_wave_size();
        uint n_groups_to_dispatch = std::max(std::min(n / block_size, uint(512)), uint(1));

        cl::NDRange global(block_size * n_groups_to_dispatch);
        cl::NDRange local(block_size);

        fill_value.setArg(0, values);
        fill_value.setArg(1, n);
        fill_value.setArg(2, value);
        queue.enqueueNDRangeKernel(fill_value, cl::NDRange(), global, local);
    }

}// namespace spla

#endif//SPLA_CL_FILL_HPP
