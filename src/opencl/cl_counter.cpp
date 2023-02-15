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

#include "cl_counter.hpp"

namespace spla {

    CLCounter::CLCounter(uint init) {
        m_buffer = cl::Buffer(get_acc_cl()->get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(uint), &init);
    }
    uint CLCounter::get(cl::CommandQueue& queue, cl::Event* event) {
        uint value;
        queue.enqueueReadBuffer(m_buffer, true, 0, sizeof(uint), &value, nullptr, event);
        return value;
    }
    void CLCounter::set(cl::CommandQueue& queue, uint value, cl::Event* event) {
        queue.enqueueWriteBuffer(m_buffer, true, 0, sizeof(uint), &value, nullptr, event);
    }
    cl::Buffer& CLCounter::buffer() {
        return m_buffer;
    }

    CLCounterWrapper::CLCounterWrapper() {
        m_counter = std::move(get_acc_cl()->get_counter_pool()->allocate());
    }
    CLCounterWrapper::~CLCounterWrapper() {
        get_acc_cl()->get_counter_pool()->release(std::move(m_counter));
    }
    uint CLCounterWrapper::get(cl::CommandQueue& queue, cl::Event* event) {
        return m_counter->get(queue, event);
    }
    void CLCounterWrapper::set(cl::CommandQueue& queue, uint value, cl::Event* event) {
        m_counter->set(queue, value, event);
    }
    cl::Buffer& CLCounterWrapper::buffer() {
        return m_counter->buffer();
    }

    CLCounterPool::CLCounterPool(uint pre_allocate) {
        for (uint i = 0; i < pre_allocate; i++) {
            m_counters.push_back(std::make_shared<CLCounter>());
        }
    }
    std::shared_ptr<CLCounter> CLCounterPool::allocate() {
        if (m_counters.empty()) {
            m_counters.push_back(std::make_shared<CLCounter>());
        }

        auto counter = std::move(m_counters.back());
        m_counters.pop_back();
        return counter;
    }
    void CLCounterPool::release(std::shared_ptr<CLCounter> counter) {
        m_counters.push_back(std::move(counter));
    }

}// namespace spla