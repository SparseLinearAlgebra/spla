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

#ifndef SPLA_CL_COUNTER_HPP
#define SPLA_CL_COUNTER_HPP

#include <opencl/cl_accelerator.hpp>

#include <memory>
#include <vector>

namespace spla {

    /**
     * @class CLCounter
     * @brief Unsigned integer reusable counter for operations
     */
    class CLCounter {
    public:
        explicit CLCounter(uint init = 0);

        uint        get(cl::CommandQueue& queue, cl::Event* event = nullptr);
        void        set(cl::CommandQueue& queue, uint value, cl::Event* event = nullptr);
        cl::Buffer& buffer();

    private:
        cl::Buffer m_buffer;
    };

    /**
     * @brief CLCounterWrapper
     * @class Automates allocation and release operations of counters
     */
    class CLCounterWrapper {
    public:
        CLCounterWrapper();
        ~CLCounterWrapper();

        uint        get(cl::CommandQueue& queue, cl::Event* event = nullptr);
        void        set(cl::CommandQueue& queue, uint value, cl::Event* event = nullptr);
        cl::Buffer& buffer();

    private:
        std::shared_ptr<CLCounter> m_counter;
    };

    /**
     * @class CLCounterPool
     * @brief Global pool with pre-allocated counters
     */
    class CLCounterPool {
    public:
        explicit CLCounterPool(uint pre_allocate = 16);

        std::shared_ptr<CLCounter> allocate();
        void                       release(std::shared_ptr<CLCounter> counter);

    private:
        std::vector<std::shared_ptr<CLCounter>> m_counters;
    };

}// namespace spla

#endif//SPLA_CL_COUNTER_HPP
