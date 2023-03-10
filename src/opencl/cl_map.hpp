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

#ifndef SPLA_CL_MAP_HPP
#define SPLA_CL_MAP_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/generated/auto_map.hpp>
#include <spla/op.hpp>

namespace spla {

    template<typename T>
    void cl_map(cl::CommandQueue& queue, const cl::Buffer& source, cl::Buffer& dest, uint n, const ref_ptr<TOpUnary<T, T>>& op) {
        if (n == 0) return;

        auto* acc = get_acc_cl();

        CLProgramBuilder builder;
        builder
                .set_name("map")
                .set_source(source_map)
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .add_op("OP_UNARY", op.template as<OpUnary>())
                .acquire();

        auto kernel = builder.make_kernel("map");
        kernel.setArg(0, dest);
        kernel.setArg(1, source);
        kernel.setArg(2, n);

        cl::NDRange global(align(n, acc->get_default_wgs()));
        cl::NDRange local(acc->get_default_wgs());
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
    }

}// namespace spla

#endif//SPLA_CL_MAP_HPP
