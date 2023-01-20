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

#ifndef SPLA_CL_COO_VEC_HPP
#define SPLA_CL_COO_VEC_HPP

#include <opencl/cl_formats.hpp>
#include <opencl/cl_utils.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    void cl_coo_vec_init(const std::size_t n_values,
                         const uint*       Ai,
                         const T*          Ax,
                         CLCooVec<T>&      storage) {
        assert(n_values > 0);

        const std::size_t buffer_size_Ai = n_values * sizeof(uint);
        const std::size_t buffer_size_Ax = n_values * sizeof(T);
        const auto        flags          = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR;

        cl::Buffer buffer_Ai(get_acc_cl()->get_context(), flags, buffer_size_Ai, (void*) Ai);
        cl::Buffer buffer_Ax(get_acc_cl()->get_context(), flags, buffer_size_Ax, (void*) Ax);

        storage.Ai = std::move(buffer_Ai);
        storage.Ax = std::move(buffer_Ax);

        storage.values = n_values;
    }

    template<typename T>
    void cl_coo_vec_resize(const std::size_t n_values,
                           CLCooVec<T>&      storage) {
        assert(n_values > 0);

        const std::size_t buffer_size_Ai = n_values * sizeof(uint);
        const std::size_t buffer_size_Ax = n_values * sizeof(T);
        const auto        flags          = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS;

        cl::Buffer buffer_Ai(get_acc_cl()->get_context(), flags, buffer_size_Ai);
        cl::Buffer buffer_Ax(get_acc_cl()->get_context(), flags, buffer_size_Ax);

        storage.Ai = std::move(buffer_Ai);
        storage.Ax = std::move(buffer_Ax);
    }

    template<typename T>
    void cl_coo_vec_clear(CLCooVec<T>& storage) {
        storage.Ai     = cl::Buffer();
        storage.Ax     = cl::Buffer();
        storage.values = 0;
    }

    template<typename T>
    void cl_coo_vec_read(const std::size_t  n_values,
                         uint*              Ai,
                         T*                 Ax,
                         const CLCooVec<T>& storage,
                         cl::CommandQueue&  queue,
                         bool               blocking = true) {
        const std::size_t buffer_size_Ai = n_values * sizeof(uint);
        const std::size_t buffer_size_Ax = n_values * sizeof(T);
        const auto        flags          = CL_MEM_READ_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR;

        cl::Buffer staging_Ai(get_acc_cl()->get_context(), flags, buffer_size_Ai);
        cl::Buffer staging_Ax(get_acc_cl()->get_context(), flags, buffer_size_Ax);

        queue.enqueueCopyBuffer(storage.Ai, staging_Ai, 0, 0, buffer_size_Ai);
        queue.enqueueCopyBuffer(storage.Ax, staging_Ax, 0, 0, buffer_size_Ax);
        queue.enqueueReadBuffer(staging_Ai, blocking, 0, buffer_size_Ai, Ai);
        queue.enqueueReadBuffer(staging_Ax, blocking, 0, buffer_size_Ax, Ax);
    }

    template<typename T>
    void cl_coo_vec_to_dense(const std::size_t  n_rows,
                             const std::size_t  n_values,
                             const CLCooVec<T>& in,
                             CLDenseVec<T>&     out,
                             cl::CommandQueue&  queue) {
        auto* acc   = get_acc_cl();
        auto* utils = acc->get_utils();

        utils->template fill_zero<T>(out.Ax, n_rows, queue);
        utils->template vec_coo_to_dense<T>(in.Ai, in.Ax, out.Ax, n_values, queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_COO_VEC_HPP
