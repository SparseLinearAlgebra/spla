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

#include "cl_accelerator.hpp"

namespace spla {

    Status CLAccelerator::init() {
        if (set_platform(0) != Status::Ok)
            return Status::PlatformNotFound;

        if (set_device(0) != Status::Ok)
            return Status::DeviceNotFound;

        // Output handy info
        LOG_MSG(Status::Ok, "Initialize accelerator: " << get_description());

        return Status::Ok;
    }
    Status CLAccelerator::set_platform(int index) {
        std::vector<cl::Platform> available_platforms;
        cl::Platform::get(&available_platforms);

        if (available_platforms.empty()) {
            LOG_MSG(Status::PlatformNotFound, "no platform to select for OpenCL acceleration, check your system runtime");
            return Status::PlatformNotFound;
        }
        if (available_platforms.size() <= index) {
            LOG_MSG(Status::InvalidArgument, "index out of list of available platforms");
            return Status::InvalidArgument;
        }

        m_device = cl::Device();
        m_platform = available_platforms[index];
        LOG_MSG(Status::Ok, "select OpenCL platform " << m_platform.getInfo<CL_PLATFORM_NAME>());

        return Status::Ok;
    }
    Status CLAccelerator::set_device(int index) {
        std::vector<cl::Device> available_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_ALL, &available_devices);

        if (available_devices.empty()) {
            LOG_MSG(Status::DeviceNotFound, "no device in selected platform, check your OpenCL runtime");
            return Status::DeviceNotFound;
        }
        if (available_devices.size() <= index) {
            LOG_MSG(Status::DeviceNotFound, "index out of list of available devices");
            return Status::DeviceNotFound;
        }

        m_device = available_devices[0];
        LOG_MSG(Status::Ok, "select OpenCL device " << m_device.getInfo<CL_DEVICE_NAME>());

        return Status::Ok;
    }
    Status CLAccelerator::set_queues_count(int count) {
        return Status::Ok;
    }
    std::string CLAccelerator::get_name() {
        return "OpenCL";
    }
    std::string CLAccelerator::get_description() {
        return "Single-device OpenCL-based acceleration";
    }
}// namespace spla