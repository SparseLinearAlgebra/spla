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

#include "cl_alloc_linear.hpp"

#include <core/logger.hpp>

#include <cassert>

namespace spla {

    CLAllocLinear::CLAllocLinear(std::size_t arena_size, std::size_t alignment) {
        assert(m_arena_size > 0);
        assert(alignment > 0);
        m_fallback_size = arena_size;
        m_arena_size    = arena_size;
        m_alignment     = alignment;
        m_arena.emplace_back(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, m_arena_size);
        LOG_MSG(Status::Ok, "preallocate " << m_arena_size);
    }

    cl::Buffer CLAllocLinear::alloc(std::size_t size) {
        std::size_t size_aligned = aligns(size, m_alignment);

        if (size_aligned >= m_fallback_size) {
            // Fallback in case if too big allocation required
            return cl::Buffer(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, size);
        }
        if (size_aligned + m_curr_offset > m_arena_size) {
            // Allocate new page to fit this allocation
            expand();
        }

        auto&       buffer           = m_arena.back();
        std::size_t buffer_region[2] = {m_curr_offset, size_aligned};
        auto        allocated        = buffer.createSubBuffer(CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, buffer_region);

        m_curr_offset += size_aligned;
        m_total_allocated += size_aligned;

        return allocated;
    }

    void CLAllocLinear::free(cl::Buffer) {
        // nothing to do
    }

    void CLAllocLinear::free_all() {
        LOG_MSG(Status::Ok, "free all allocated=" << m_total_allocated << " bytes");
        shrink();
        m_curr_offset     = 0;
        m_total_allocated = 0;
    }

    void CLAllocLinear::expand() {
        m_curr_offset = 0;
        m_arena_size *= 2;
        m_arena.emplace_back(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, m_arena_size);
        LOG_MSG(Status::Ok, "expand to " << m_arena_size);
    }
    void CLAllocLinear::shrink() {
        if (m_arena.size() > 1) {
            cl::Buffer main_arena = m_arena.back();
            m_arena.clear();
            m_arena.push_back(std::move(main_arena));
            LOG_MSG(Status::Ok, "shrink to " << m_arena_size);
        }
    }

}// namespace spla
