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

#include "cl_accelerator.hpp"

#include <opencl/cl_alloc_general.hpp>
#include <opencl/cl_alloc_linear.hpp>
#include <opencl/cl_counter.hpp>
#include <opencl/cl_program_cache.hpp>

#include <sstream>

namespace spla {

    CLAccelerator::CLAccelerator()  = default;
    CLAccelerator::~CLAccelerator() = default;

    Status CLAccelerator::init() {
        m_description = "no platform or device";

        if (set_platform(0) != Status::Ok)
            return Status::PlatformNotFound;

        if (set_device(0) != Status::Ok)
            return Status::DeviceNotFound;

        if (set_queues_count(1) != Status::Ok)
            return Status::Error;

        m_cache = std::make_unique<CLProgramCache>();

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

        m_counter_pool.reset();
        m_alloc_general.reset();
        m_alloc_linear.reset();
        m_alloc_tmp = nullptr;
        m_device    = cl::Device();
        m_platform  = available_platforms[index];
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

        m_device = available_devices[index];
        LOG_MSG(Status::Ok, "select OpenCL device " << m_device.getInfo<CL_DEVICE_NAME>());

        m_vendor_code.clear();
        m_vendor_name   = m_device.getInfo<CL_DEVICE_VENDOR>();
        m_vendor_id     = m_device.getInfo<CL_DEVICE_VENDOR_ID>();
        m_max_cu        = m_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        m_max_wgs       = m_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        m_max_local_mem = m_device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        m_addr_align    = m_device.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>() / 8;// from bits to bytes

        m_is_nvidia = false;
        m_is_amd    = false;
        m_is_intel  = false;
        m_is_img    = false;

        if (m_vendor_name.find("Intel") != std::string::npos ||
            m_vendor_name.find("intel") != std::string::npos ||
            m_vendor_name.find("INTEL") != std::string::npos ||
            m_vendor_id == 32902) {
            m_vendor_code = VENDOR_CODE_INTEL;
            m_default_wgs = 64;
            m_wave_size   = 8;
            m_is_intel    = true;
        }
        if (m_vendor_name.find("Nvidia") != std::string::npos ||
            m_vendor_name.find("nvidia") != std::string::npos ||
            m_vendor_name.find("NVIDIA") != std::string::npos ||
            m_vendor_id == 4318) {
            m_vendor_code = VENDOR_CODE_NVIDIA;
            m_default_wgs = 64;
            m_wave_size   = 32;
            m_is_nvidia   = true;
        }
        if (m_vendor_name.find("Amd") != std::string::npos ||
            m_vendor_name.find("amd") != std::string::npos ||
            m_vendor_name.find("AMD") != std::string::npos ||
            m_vendor_name.find("Advanced Micro Devices") != std::string::npos ||
            m_vendor_name.find("advanced micro devices") != std::string::npos ||
            m_vendor_name.find("ADVANCED MICRO DEVICES") != std::string::npos) {
            m_vendor_code = VENDOR_CODE_AMD;
            m_default_wgs = 64;
            m_wave_size   = 64;
            m_is_amd      = true;

            // Likely, it is an integrated amd device
            if (m_max_wgs <= 256 || m_max_cu == 1) m_wave_size = 16;
        }
        if (m_vendor_name.find("Imagination Technologies") != std::string::npos ||
            m_vendor_name.find("IMG") != std::string::npos ||
            m_vendor_name.find("img") != std::string::npos ||
            m_vendor_id == 0x1010) {
            m_vendor_code = VENDOR_CODE_IMG;
            m_default_wgs = 32;
            m_wave_size   = 32;
            m_is_img      = true;
        }

        if (m_vendor_code.empty()) {
            LOG_MSG(Status::Error, "failed to match one of the pre-defined vendors");
            m_default_wgs = 64;
            m_wave_size   = 8;
        }


        std::stringstream desc;
        desc << "OpenCL Acc " << m_platform.getInfo<CL_PLATFORM_NAME>()
             << " device: " << m_device.getInfo<CL_DEVICE_NAME>()
             << " vendor:" << m_vendor_code
             << " mcu:" << m_max_cu
             << " wave:" << m_wave_size
             << " mwgs:" << m_max_wgs;

        m_description = desc.str();

        LOG_MSG(Status::Ok, m_description);

        return Status::Ok;
    }
    Status CLAccelerator::set_queues_count(int count) {
        m_context = cl::Context(m_device);
        m_queues.clear();
        m_queues.reserve(count);

        for (int i = 0; i < count; i++) {
            cl_command_queue_properties properties = 0;
#ifndef SPLA_RELEASE
            properties = CL_QUEUE_PROFILING_ENABLE;
#endif
            cl::CommandQueue queue(m_context, properties);
            m_queues.emplace_back(std::move(queue));
        }

        m_counter_pool  = std::make_unique<CLCounterPool>();
        m_alloc_general = std::make_unique<CLAllocGeneral>();
        m_alloc_tmp     = m_alloc_general.get();

        if (!is_nvidia()) {
            m_alloc_linear = std::make_unique<CLAllocLinear>(CLAllocLinear::DEFAULT_SIZE, m_addr_align);
            m_alloc_tmp    = m_alloc_linear.get();
        }

        LOG_MSG(Status::Ok, "configure " << count << " queues for computations");
        return Status::Ok;
    }
    const std::string& CLAccelerator::get_name() {
        return m_name;
    }
    const std::string& CLAccelerator::get_description() {
        return m_description;
    }
    const std::string& CLAccelerator::get_suffix() {
        return m_suffix;
    }

}// namespace spla
