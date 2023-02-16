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

#include "cl_alloc_general.hpp"

namespace spla {

    cl::Buffer CLAllocGeneral::alloc(std::size_t size) {
        return cl::Buffer(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, size);
    }
    void CLAllocGeneral::alloc_paired(std::size_t size1, std::size_t size2, cl::Buffer& buffer1, cl::Buffer& buffer2) {
        auto* cl_acc = get_acc_cl();

        if (cl_acc->get_vendor_code() == VENDOR_CODE_NVIDIA) {
            buffer1 = alloc(size1);
            buffer2 = alloc(size2);
            return;
        }

        const uint  alignment     = cl_acc->get_addr_align();
        std::size_t offset        = aligns(size1, alignment);
        std::size_t size          = offset + aligns(size2, alignment);
        cl::Buffer  source_buffer = alloc(size);

        std::size_t range1[2] = {0, size1};
        buffer1               = source_buffer.createSubBuffer(CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, range1);
        std::size_t range2[2] = {offset, size2};
        buffer2               = source_buffer.createSubBuffer(CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, range2);
    }
    void CLAllocGeneral::free(cl::Buffer buffer) {
        // nothing to do
    }
    void CLAllocGeneral::free_all() {
        // nothing to do
    }

}// namespace spla
