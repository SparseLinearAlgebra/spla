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

    Status CLAccelerator::init(const Config& cfg) {

        ConfigStatus config_status;
        config_status = validate(cfg);
        if (config_status != ConfigStatus::Ok) {
            std::exit(1);
        }

        std::cout << "Configuration parametrs:" << std::endl;
        std::cout << "OpenCL platform index: " << cfg.platform.value() << std::endl;
        std::cout << "OpenCL device index: " << cfg.device.value() << std::endl;
        std::cout << "Queues number: " << cfg.queues.value() << std::endl;
        std::cout << "Profiling: " << cfg.profiling.value() << std::endl;
        std::cout << "Allocator: " << cfg.allocator.value() << std::endl;
        if (cfg.allocator == "linear") std::cout << "Linear allocator size: " << cfg.allocator_size.value() << std::endl;
        std::cout << "Wave size: " << cfg.wave_size.value() << std::endl;
        std::cout << "Default wgs: " << cfg.default_wgs.value() << std::endl;
        std::cout << "Num of mem banks: " << cfg.num_of_mem_banks.value() << std::endl;
        std::cout << "Verbosity: " << cfg.verbosity.value() << std::endl;

        m_description = "no platform or device";

        if (set_platform(cfg.platform.value()) != Status::Ok)
            return Status::PlatformNotFound;

        if (set_device(cfg.device.value()) != Status::Ok)
            return Status::DeviceNotFound;

        if (set_profiling(cfg.profiling.value()) != Status::Ok)
            return Status::Error;

        if (set_queues_count(cfg.queues.value()) != Status::Ok)
            return Status::Error;

        if (cfg.allocator.value() == "linear") {
            if (set_linear_allocator(cfg.allocator_size.value()) != Status::Ok)
                return Status::Error;
        } else {
            if (set_general_allocator() != Status::Ok)
                return Status::Error;
        }

        if (set_default_wgs(cfg.default_wgs.value()) != Status::Ok)
            return Status::Error;

        if (set_wave_size(cfg.wave_size.value()) != Status::Ok)
            return Status::Error;

        if (set_num_of_mem_banks(cfg.num_of_mem_banks.value()) != Status::Ok)
            return Status::Error;

        m_cache = std::make_unique<CLProgramCache>();

        LOG_MSG(Status::Ok, "Initialize accelerator: " << get_description());

        return Status::Ok;
    }


    Status CLAccelerator::set_platform(int index) {
        std::vector<cl::Platform> available_platforms;
        cl::Platform::get(&available_platforms);

        if (available_platforms.empty()) {
            LOG_MSG(Status::PlatformNotFound, "no platform to select for OpenCL acceleration");
            return Status::PlatformNotFound;
        }

        if (index < 0) {
            LOG_MSG(Status::InvalidArgument, "platform index must be >= 0 (got " << index << ")");
            return Status::InvalidArgument;
        }

        if (available_platforms.size() <= static_cast<size_t>(index)) {
            LOG_MSG(Status::InvalidArgument, "platform index out of range (got " << index << ", max " << available_platforms.size() - 1 << ")");
            return Status::InvalidArgument;
        }

        m_counter_pool.reset();
        m_alloc_general.reset();
        m_alloc_linear.reset();
        m_alloc_tmp = nullptr;
        m_device    = cl::Device();

        m_platform = available_platforms[index];
        LOG_MSG(Status::Ok, "select OpenCL platform " << m_platform.getInfo<CL_PLATFORM_NAME>());

        return Status::Ok;
    }


    Status CLAccelerator::set_device(int index) {
        m_vendor_code.clear();

        std::vector<cl::Device> available_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &available_devices);

        if (available_devices.empty()) {
            LOG_MSG(Status::DeviceNotFound, "no device to select for OpenCL acceleration");
            return Status::DeviceNotFound;
        }

        if (index < 0) {
            LOG_MSG(Status::InvalidArgument, "device index must be >= 0 (got " << index << ")");
            return Status::InvalidArgument;
        }

        if (available_devices.size() <= static_cast<size_t>(index)) {
            LOG_MSG(Status::InvalidArgument, "platform index out of range (got " << index << ", max " << available_devices.size() - 1 << ")");
            return Status::InvalidArgument;
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
            m_is_intel    = true;
        }
        if (m_vendor_name.find("Nvidia") != std::string::npos ||
            m_vendor_name.find("nvidia") != std::string::npos ||
            m_vendor_name.find("NVIDIA") != std::string::npos ||
            m_vendor_id == 4318) {
            m_vendor_code = VENDOR_CODE_NVIDIA;
            m_is_nvidia   = true;
        }
        if (m_vendor_name.find("Amd") != std::string::npos ||
            m_vendor_name.find("amd") != std::string::npos ||
            m_vendor_name.find("AMD") != std::string::npos ||
            m_vendor_name.find("Advanced Micro Devices") != std::string::npos ||
            m_vendor_name.find("advanced micro devices") != std::string::npos ||
            m_vendor_name.find("ADVANCED MICRO DEVICES") != std::string::npos) {
            m_vendor_code = VENDOR_CODE_AMD;
            m_is_amd      = true;
        }
        if (m_vendor_name.find("Imagination Technologies") != std::string::npos ||
            m_vendor_name.find("IMG") != std::string::npos ||
            m_vendor_name.find("img") != std::string::npos ||
            m_vendor_id == 0x1010) {
            m_vendor_code = VENDOR_CODE_IMG;
            m_is_img      = true;
        }
        if (m_vendor_code.empty()) {
            LOG_MSG(Status::Error, "failed to match one of the pre-defined vendors");
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

    Status CLAccelerator::set_profiling(bool enabled) {
        m_profiling_enabled = enabled;
        LOG_MSG(Status::Ok, "set profiling " << (enabled ? "enabled" : "disabled"));
        return Status::Ok;
    }


    Status CLAccelerator::set_queues_count(int count) {
        if (count <= 0) {
            LOG_MSG(Status::InvalidArgument, "queues count must be > 0 (got " << count << ")");
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

        m_counter_pool  = std::make_unique<CLCounterPool>();
        m_alloc_general = std::make_unique<CLAllocGeneral>();
        m_alloc_tmp     = m_alloc_general.get();

        LOG_MSG(Status::Ok, "configure " << count << " queues for computations"
                                         << " (profiling: " << (m_profiling_enabled ? "ON" : "OFF") << ")");
        return Status::Ok;
    }


    Status CLAccelerator::set_linear_allocator(size_t size) {
        if (size == 0) {
            LOG_MSG(Status::InvalidArgument, "allocator_size must be > 0 for linear allocator (got " << size << ")");
            return Status::InvalidArgument;
        }
        m_alloc_linear = std::make_unique<CLAllocLinear>(size, m_addr_align);
        m_alloc_tmp    = m_alloc_linear.get();
        LOG_MSG(Status::Ok, "set linear allocator (size: " << size << " bytes)");
        return Status::Ok;
    }


    Status CLAccelerator::set_general_allocator() {
        m_alloc_tmp = m_alloc_general.get();
        LOG_MSG(Status::Ok, "set general allocator");
        return Status::Ok;
    }


    Status CLAccelerator::set_default_wgs(int wgs) {
        if (wgs <= 0) {
            LOG_MSG(Status::InvalidArgument, "default_wgs must be > 0 (got " << wgs << ")");
            return Status::InvalidArgument;
        }
        m_default_wgs = wgs;
        LOG_MSG(Status::Ok, "set default work group size: " << wgs);
        return Status::Ok;
    }


    Status CLAccelerator::set_wave_size(int size) {
        if (size <= 0) {
            LOG_MSG(Status::InvalidArgument, "wave_size must be > 0 (got " << size << ")");
            return Status::InvalidArgument;
        }
        m_wave_size = size;
        LOG_MSG(Status::Ok, "set wave size: " << size);
        return Status::Ok;
    }


    Status CLAccelerator::set_num_of_mem_banks(int banks) {
        if (banks <= 0) {
            LOG_MSG(Status::InvalidArgument, "num_of_mem_banks must be > 0 (got " << banks << ")");
            return Status::InvalidArgument;
        }
        m_num_of_mem_banks = banks;
        LOG_MSG(Status::Ok, "set num of mem banks: " << banks);
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