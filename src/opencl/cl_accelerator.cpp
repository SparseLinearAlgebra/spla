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

#include <core/configure.hpp>
#include <core/logger.hpp>
#include <opencl/cl_alloc_general.hpp>
#include <opencl/cl_alloc_linear.hpp>
#include <opencl/cl_counter.hpp>
#include <opencl/cl_program_cache.hpp>

#include <sstream>

namespace spla {

    std::optional<std::pair<int, int>> find_first_device() {
        std::vector<cl::Platform> platforms;
#if defined(CL_HPP_ENABLE_EXCEPTIONS)
        try {
            cl::Platform::get(&platforms);
        } catch (const cl::Error& e) {
            LOG_MSG(Status::PlatformNotFound, "failed to query OpenCL platforms: " << e.what());
            return std::nullopt;
        }
#else
        cl::Platform::get(&platforms);
#endif
        if (platforms.empty()) {
            LOG_MSG(Status::PlatformNotFound, "no OpenCL platforms available");
            return std::nullopt;
        }

        for (size_t p = 0; p < platforms.size(); p++) {
            std::vector<cl::Device> devices;
#if defined(CL_HPP_ENABLE_EXCEPTIONS)
            try {
                platforms[p].getDevices(CL_DEVICE_TYPE_GPU, &devices);
            } catch (const cl::Error& e) {
                LOG_MSG(Status::DeviceNotFound, "failed to query devices of platform " << p << ": " << e.what());
                continue;
            }
#else
            platforms[p].getDevices(CL_DEVICE_TYPE_GPU, &devices);
#endif
            if (!devices.empty()) {
                return std::make_pair(static_cast<int>(p), 0);
            }
        }

        return std::nullopt;
    }

    CLAccelerator::CLAccelerator()  = default;
    CLAccelerator::~CLAccelerator() = default;

    Status CLAccelerator::init() {
        if (!config_initialized) {
            ConfigStatus status = configure(0, nullptr, false);
            if (status != ConfigStatus::Ok) return Status::Error;
        }

        std::string backend = config_final.backend.value_or("opencl_gpu");
        if (backend == "cpu") {
            return Status::NoAcceleration;
        }

        int         platform_index    = 0;
        int         device_index      = 0;
        int         queues_count      = config_final.queues_count.value_or(1);
        bool        profiling         = config_final.profiling.value_or(false);
        std::string allocator_type    = config_final.allocator_type.value_or("general");
        size_t      linear_alloc_size = config_final.linear_alloc_size.value_or(0);

        if (backend == "opencl_gpu") {
            auto found = find_first_device();

            if (!found) {
                LOG_MSG(Status::DeviceNotFound, "no OpenCL device found for backend='" << backend << "'");
                return Status::DeviceNotFound;
            }

            platform_index = found->first;
            device_index   = found->second;
        } else if (backend == "by_index") {
            if (!config_final.platform_index.has_value() || !config_final.device_index.has_value()) {
                LOG_MSG(Status::InvalidArgument, "platform_index and device_index are required for backend='by_index'");
                return Status::InvalidArgument;
            }
            platform_index = *config_final.platform_index;
            device_index   = *config_final.device_index;
        } else {
            LOG_MSG(Status::InvalidArgument, "unknown backend: '" << backend << "'");
            return Status::InvalidArgument;
        }

        LOG_MSG(Status::Ok, "Initializing accelerator...");

        LOG_MSG(Status::Ok, "Configuration parameters:");
        LOG_MSG(Status::Ok, "  backend   = " << backend);
        LOG_MSG(Status::Ok, "  platform  = " << platform_index);
        LOG_MSG(Status::Ok, "  device    = " << device_index);
        LOG_MSG(Status::Ok, "  queues    = " << queues_count);
        LOG_MSG(Status::Ok, "  profiling = " << (profiling ? "true" : "false"));
        LOG_MSG(Status::Ok, "  allocator = " << allocator_type);
        if (allocator_type == "linear") {
            LOG_MSG(Status::Ok, "  alloc_size= " << linear_alloc_size);
        }
        LOG_MSG(Status::Ok, "  verbosity = " << config_final.verbosity.value());

        Status status = set_platform(platform_index);
        if (status != Status::Ok) return status;

        status = set_device(device_index);
        if (status != Status::Ok) return status;

        status = set_profiling(profiling);
        if (status != Status::Ok) return status;

        status = set_queues_count(queues_count);
        if (status != Status::Ok) return status;

        if (allocator_type == "linear") {
            status = set_linear_allocator(linear_alloc_size);
        } else {
            status = set_general_allocator();
        }
        if (status != Status::Ok) return status;

        m_cache = std::make_unique<CLProgramCache>();
        LOG_MSG(Status::Ok, "program cache created");

        LOG_MSG(Status::Ok, "accelerator initialized: " << get_description());

        return Status::Ok;
    }


    Status CLAccelerator::set_platform(int index) {
        std::vector<cl::Platform> available_platforms;
        cl::Platform::get(&available_platforms);

        if (index < 0 || static_cast<size_t>(index) >= available_platforms.size()) {
            LOG_MSG(Status::PlatformNotFound, "no OpenCL platform with index " << index
                                                                               << " (available: " << available_platforms.size() << ")");
            return Status::PlatformNotFound;
        }

        m_counter_pool.reset();
        m_alloc_general.reset();
        m_alloc_linear.reset();
        m_alloc_tmp         = nullptr;
        m_device            = cl::Device();
        m_profiling_enabled = false;

        m_platform = available_platforms[index];
        LOG_MSG(Status::Ok, "select OpenCL platform: "
                                    << m_platform.getInfo<CL_PLATFORM_NAME>());

        return Status::Ok;
    }


    Status CLAccelerator::set_device(int index) {
        std::vector<cl::Device> available_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &available_devices);

        if (index < 0 || static_cast<size_t>(index) >= available_devices.size()) {
            LOG_MSG(Status::DeviceNotFound, "no OpenCL device with index " << index
                                                                           << " (available: " << available_devices.size() << ")");
            return Status::DeviceNotFound;
        }

        m_device = available_devices[index];
        LOG_MSG(Status::Ok, "select OpenCL device: "
                                    << m_device.getInfo<CL_DEVICE_NAME>());

        m_vendor_code.clear();
        m_vendor_name   = m_device.getInfo<CL_DEVICE_VENDOR>();
        m_vendor_id     = m_device.getInfo<CL_DEVICE_VENDOR_ID>();
        m_max_cu        = m_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        m_max_wgs       = m_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        m_max_local_mem = m_device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        m_addr_align    = m_device.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>() / 8;

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
            LOG_MSG(Status::InvalidArgument, "unknown vendor: " << m_vendor_name
                                                                << ", using default parameters");
            m_default_wgs = 64;
            m_wave_size   = 8;
        }

        std::stringstream desc;
        desc << "OpenCL Acc "
             << m_platform.getInfo<CL_PLATFORM_NAME>()
             << " | device: " << m_device.getInfo<CL_DEVICE_NAME>()
             << " | vendor: " << m_vendor_code
             << " | mcu: " << m_max_cu
             << " | wave: " << m_wave_size
             << " | mwgs: " << m_max_wgs;

        m_description = desc.str();

        return Status::Ok;
    }


    Status CLAccelerator::set_profiling(bool enabled) {
        m_profiling_enabled = enabled;
        LOG_MSG(Status::Ok, "set profiling: " << (enabled ? "enabled" : "disabled"));
        return Status::Ok;
    }


    Status CLAccelerator::set_queues_count(int count) {
        if (count < 1) {
            LOG_MSG(Status::InvalidArgument, "queues count must be >= 1 (got " << count << ")");
            return Status::InvalidArgument;
        }

        m_context = cl::Context(m_device);
        m_queues.clear();
        m_queues.reserve(count);

        for (int i = 0; i < count; i++) {
            cl_command_queue_properties properties = 0;
            if (m_profiling_enabled) {
                properties |= CL_QUEUE_PROFILING_ENABLE;
            }
            cl::CommandQueue queue(m_context, properties);
            m_queues.emplace_back(std::move(queue));
        }

        m_counter_pool = std::make_unique<CLCounterPool>();

        LOG_MSG(Status::Ok, "configure " << count << " queues"
                                         << " (profiling: " << (m_profiling_enabled ? "ON" : "OFF") << ")");
        return Status::Ok;
    }


    Status CLAccelerator::set_linear_allocator(size_t size) {
        if (size == 0) {
            LOG_MSG(Status::InvalidArgument, "linear allocator size must be > 0 (got " << size << ")");
            return Status::InvalidArgument;
        }

        m_alloc_linear = std::make_unique<CLAllocLinear>(size, m_addr_align);
        m_alloc_tmp    = m_alloc_linear.get();
        LOG_MSG(Status::Ok, "set linear allocator (size: " << size << " bytes)");
        return Status::Ok;
    }


    Status CLAccelerator::set_general_allocator() {
        m_alloc_general = std::make_unique<CLAllocGeneral>();
        m_alloc_tmp     = m_alloc_general.get();
        LOG_MSG(Status::Ok, "set general allocator");
        return Status::Ok;
    }

    const std::string& CLAccelerator::get_name() { return m_name; }
    const std::string& CLAccelerator::get_description() { return m_description; }
    const std::string& CLAccelerator::get_suffix() { return m_suffix; }

}// namespace spla