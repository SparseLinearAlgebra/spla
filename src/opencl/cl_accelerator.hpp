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

#ifndef SPLA_CL_ACCELERATOR_HPP
#define SPLA_CL_ACCELERATOR_HPP

#include <core/accelerator.hpp>
#include <core/logger.hpp>

#include <string>
#include <vector>

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/opencl.hpp>

namespace spla {

    class CLAccelerator final : public Accelerator {
    public:
        ~CLAccelerator() override = default;

        // Accelerator API
        Status init() override;
        Status set_platform(int index) override;
        Status set_device(int index) override;
        Status set_queues_count(int count) override;
        std::string get_name() override;
        std::string get_description() override;

        // OpenCL API
        cl::Platform &get_platform() { return m_platform; }
        cl::Device &get_device() { return m_device; }
        cl::Context &get_context() { return m_context; }
        std::vector<cl::CommandQueue> &get_queues() { return m_queues; }

    private:
        cl::Platform m_platform;
        cl::Device m_device;
        cl::Context m_context;
        std::vector<cl::CommandQueue> m_queues;
    };

}// namespace spla

#endif//SPLA_CL_ACCELERATOR_HPP