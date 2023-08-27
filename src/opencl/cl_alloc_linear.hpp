/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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

#ifndef SPLA_CL_ALLOC_LINEAR_HPP
#define SPLA_CL_ALLOC_LINEAR_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_alloc.hpp>

#include <svector.hpp>

namespace spla {

    /**
     * @class CLAllocLinear
     * @brief Linear allocator for temporary device local buffer allocations
     *
     * Linear allocator uses linear allocation strategy. It places allocated
     * buffers sequentially in a large pre-allocated arena buffer. If arena is full,
     * new arena allocated. On a free all step, all allocations must be invalid.
     * All allocated arenas cleared, and one new bigger arena allocated.
     */
    class CLAllocLinear : public CLAlloc {
    public:
        static constexpr const uint DEFAULT_SIZE      = 1024 * 1024;// 1 MiB
        static constexpr const uint DEFAULT_ALIGNMENT = 128;

        explicit CLAllocLinear(std::size_t arena_size = DEFAULT_SIZE, std::size_t alignment = DEFAULT_ALIGNMENT);
        ~CLAllocLinear() override = default;

        cl::Buffer alloc(std::size_t size) override;
        void       free(cl::Buffer buffer) override;
        void       free_all() override;

    protected:
        void expand();
        void shrink();

    private:
        ankerl::svector<cl::Buffer, 4> m_arena;
        std::size_t                    m_alignment       = 128;
        std::size_t                    m_curr_offset     = 0;
        std::size_t                    m_total_allocated = 0;
        std::size_t                    m_arena_size      = 0;
        std::size_t                    m_fallback_size   = 0;
    };

}// namespace spla

#endif//SPLA_CL_ALLOC_LINEAR_HPP
