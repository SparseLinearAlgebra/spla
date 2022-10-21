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

#ifndef SPLA_CL_DENSE_VEC_HPP
#define SPLA_CL_DENSE_VEC_HPP

#include <opencl/cl_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cl_dense_vec_init(std::size_t    size,
                           const T*       values,
                           CLDenseVec<T>& storage) {
        cl::Buffer buffer(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, size * sizeof(T), (void*) values);
        storage.Ax = std::move(buffer);
    }

    template<typename T>
    void cl_dense_vec_write(std::size_t       size,
                            const T*          values,
                            CLDenseVec<T>&    storage,
                            cl::CommandQueue& queue,
                            bool              blocking = true) {
        queue.enqueueWriteBuffer(storage.Ax, blocking, 0, size * sizeof(T), values);
    }

    template<typename T>
    void cl_dense_vec_read(std::size_t       size,
                           T*                values,
                           CLDenseVec<T>&    storage,
                           cl::CommandQueue& queue,
                           bool              blocking = true) {
        std::size_t buffer_size = size * sizeof(T);
        cl::Buffer  staging(get_acc_cl()->get_context(), CL_MEM_READ_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_USE_HOST_PTR, buffer_size, values);

        queue.enqueueCopyBuffer(storage.Ax, staging, 0, 0, buffer_size);

        if (blocking) queue.finish();
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_DENSE_VEC_HPP
