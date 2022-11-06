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

#ifndef SPLA_CL_CSR_HPP
#define SPLA_CL_CSR_HPP

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
        auto& ctx   = get_acc_cl()->get_context();
        auto  flags = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR;

        cl::Buffer cl_Ap(ctx, flags, n_rows * sizeof(uint), (void*) Ap);
        cl::Buffer cl_Aj(ctx, flags, n_values * sizeof(uint), (void*) Aj);
        cl::Buffer cl_Ax(ctx, flags, n_values * sizeof(T), (void*) Ax);

        storage.Ap = std::move(cl_Ap);
        storage.Aj = std::move(cl_Aj);
        storage.Ax = std::move(cl_Ax);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_CSR_HPP
