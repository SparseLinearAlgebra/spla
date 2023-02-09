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

#ifndef SPLA_CL_FORMAT_DENSE_VEC_HPP
#define SPLA_CL_FORMAT_DENSE_VEC_HPP

#include <opencl/cl_formats.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/cl_sort_by_key.hpp>
#include <opencl/generated/auto_vector_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cl_dense_vec_resize(const std::size_t n_rows,
                             CLDenseVec<T>&    storage) {
        const std::size_t buffer_size = n_rows * sizeof(T);
        const auto        flags       = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS;

        cl::Buffer buffer(get_acc_cl()->get_context(), flags, buffer_size);
        storage.Ax = std::move(buffer);
    }

    template<typename T>
    void cl_dense_vec_fill_zero(const std::size_t n_rows,
                                CLDenseVec<T>&    storage) {
        cl_fill_zero<T>(get_acc_cl()->get_queue_default(), storage.Ax, n_rows);
    }

    template<typename T>
    void cl_dense_vec_init(const std::size_t n_rows,
                           const T*          values,
                           CLDenseVec<T>&    storage) {
        assert(values);

        const std::size_t buffer_size = n_rows * sizeof(T);
        const auto        flags       = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR;

        cl::Buffer buffer(get_acc_cl()->get_context(), flags, buffer_size, (void*) values);
        storage.Ax = std::move(buffer);
    }

    template<typename T>
    void cl_dense_vec_write(const std::size_t n_rows,
                            const T*          values,
                            CLDenseVec<T>&    storage,
                            cl::CommandQueue& queue,
                            bool              blocking = true) {
        queue.enqueueWriteBuffer(storage.Ax, blocking, 0, n_rows * sizeof(T), values);
    }

    template<typename T>
    void cl_dense_vec_read(const std::size_t n_rows,
                           T*                values,
                           CLDenseVec<T>&    storage,
                           cl::CommandQueue& queue,
                           bool              blocking = true) {
        const std::size_t buffer_size = n_rows * sizeof(T);
        cl::Buffer        staging(get_acc_cl()->get_context(), CL_MEM_READ_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, buffer_size);

        queue.enqueueCopyBuffer(storage.Ax, staging, 0, 0, buffer_size);
        queue.enqueueReadBuffer(staging, blocking, 0, buffer_size, values);
    }

    template<typename T>
    void cl_dense_vec_to_coo(const std::size_t    n_rows,
                             const CLDenseVec<T>& in,
                             CLCooVec<T>&         out,
                             cl::CommandQueue&    queue) {

        CLProgramBuilder builder;
        builder.set_name("vector_format")
                .add_type("TYPE", get_ttype<T>().template as<Type>())
                .set_source(source_vector_formats)
                .acquire();

        auto* acc = get_acc_cl();

        uint       count[1] = {0};
        cl::Buffer cl_count(acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), count);
        cl::Buffer temp_Ri(acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS, n_rows * sizeof(uint));
        cl::Buffer temp_Rx(acc->get_context(), CL_MEM_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS, n_rows * sizeof(T));

        uint block_size           = acc->get_wave_size();
        uint n_groups_to_dispatch = std::max(std::min(uint(n_rows) / block_size, uint(512)), uint(1));

        cl::NDRange global(block_size * n_groups_to_dispatch);
        cl::NDRange local(block_size);

        auto kernel = builder.make_kernel("dense_to_sparse");
        kernel.setArg(0, in.Ax);
        kernel.setArg(1, temp_Ri);
        kernel.setArg(2, temp_Rx);
        kernel.setArg(3, cl_count);
        kernel.setArg(4, uint(n_rows));

        queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);
        queue.enqueueReadBuffer(cl_count, true, 0, sizeof(count[0]), count);

        out.values = count[0];
        out.Ai     = cl::Buffer(acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, count[0] * sizeof(uint));
        out.Ax     = cl::Buffer(acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, count[0] * sizeof(T));

        queue.enqueueCopyBuffer(temp_Ri, out.Ai, 0, 0, count[0] * sizeof(uint));
        queue.enqueueCopyBuffer(temp_Rx, out.Ax, 0, 0, count[0] * sizeof(T));
        queue.finish();
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_FORMAT_DENSE_VEC_HPP
