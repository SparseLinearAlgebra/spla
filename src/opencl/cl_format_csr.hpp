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

#ifndef SPLA_CL_FORMAT_CSR_HPP
#define SPLA_CL_FORMAT_CSR_HPP

#include <opencl/cl_formats.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cl_csr_init(std::size_t n_rows,
                     std::size_t n_values,
                     const uint* Ap,
                     const uint* Aj,
                     const T*    Ax,
                     CLCsr<T>&   storage) {
        auto&      ctx   = get_acc_cl()->get_context();
        const auto flags = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR;

        cl::Buffer cl_Ap(ctx, flags, (n_rows + 1) * sizeof(uint), (void*) Ap);
        cl::Buffer cl_Aj(ctx, flags, n_values * sizeof(uint), (void*) Aj);
        cl::Buffer cl_Ax(ctx, flags, n_values * sizeof(T), (void*) Ax);

        storage.Ap = std::move(cl_Ap);
        storage.Aj = std::move(cl_Aj);
        storage.Ax = std::move(cl_Ax);

        storage.values = n_values;
    }

    template<typename T>
    void cl_csr_resize(std::size_t n_rows,
                       std::size_t n_values,
                       CLCsr<T>&   storage) {
        auto&      ctx   = get_acc_cl()->get_context();
        const auto flags = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS;

        cl::Buffer cl_Ap(ctx, flags, (n_rows + 1) * sizeof(uint));
        cl::Buffer cl_Aj(ctx, flags, n_values * sizeof(uint));
        cl::Buffer cl_Ax(ctx, flags, n_values * sizeof(T));

        storage.Ap = std::move(cl_Ap);
        storage.Aj = std::move(cl_Aj);
        storage.Ax = std::move(cl_Ax);

        storage.values = n_values;
    }

    template<typename T>
    void cl_csr_read(std::size_t       n_rows,
                     std::size_t       n_values,
                     uint*             Ap,
                     uint*             Aj,
                     T*                Ax,
                     CLCsr<T>&         storage,
                     cl::CommandQueue& queue,
                     bool              blocking = true) {
        const std::size_t buffer_size_Ap = (n_rows + 1) * sizeof(uint);
        const std::size_t buffer_size_Aj = n_values * sizeof(uint);
        const std::size_t buffer_size_Ax = n_values * sizeof(T);

        const auto flags = CL_MEM_READ_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR;

        cl::Buffer staging_Ap(get_acc_cl()->get_context(), flags, buffer_size_Ap);
        cl::Buffer staging_Aj(get_acc_cl()->get_context(), flags, buffer_size_Aj);
        cl::Buffer staging_Ax(get_acc_cl()->get_context(), flags, buffer_size_Ax);

        queue.enqueueCopyBuffer(storage.Ap, staging_Ap, 0, 0, buffer_size_Ap);
        queue.enqueueCopyBuffer(storage.Aj, staging_Aj, 0, 0, buffer_size_Aj);
        queue.enqueueCopyBuffer(storage.Ax, staging_Ax, 0, 0, buffer_size_Ax);

        queue.enqueueReadBuffer(staging_Ap, false, 0, buffer_size_Ap, Ap);
        queue.enqueueReadBuffer(staging_Aj, false, 0, buffer_size_Aj, Aj);
        queue.enqueueReadBuffer(staging_Ax, blocking, 0, buffer_size_Ax, Ax);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_FORMAT_CSR_HPP
