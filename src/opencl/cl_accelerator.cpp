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
#include <opencl/cl_configure.hpp>

#include <sstream>

namespace spla {

    CLAccelerator::CLAccelerator()  = default;
    CLAccelerator::~CLAccelerator() = default;

    Status CLAccelerator::init() {

        int index_platform = config_final.platform.value();
        int index_device = config_final.device.value();
        int queues_count = config_final.queues.value();
        bool profiling = config_final.profiling.value_or(false);
        std::string allocator_type = config_final.allocator.value();
        size_t lin_alloc_size = config_final.allocator_size.value_or(CLAllocLinear::DEFAULT_SIZE);
        int default_wgs = config_final.default_wgs.value_or(64);
        int wave_size = config_final.wave_size.value_or(32);
        int verbosity = config_final.verbosity.value();

        //set_verbosity(verbosity);

        m_cache = std::make_unique<CLProgramCache>();

        std::vector<cl::Platform> available_platforms;
        cl::Platform::get(&available_platforms);

        m_counter_pool.reset();
        m_alloc_general.reset();
        m_alloc_linear.reset();
        m_alloc_tmp = nullptr;
        m_device = cl::Device();
        m_platform = available_platforms[index_platform];
        LOG_MSG(Status::Ok, "select OpenCL platform " << m_platform.getInfo<CL_PLATFORM_NAME>());

        std::vector<cl::Device> available_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &available_devices);

        m_device = available_devices[index_device];
        LOG_MSG(Status::Ok, "select OpenCL device " << m_device.getInfo<CL_DEVICE_NAME>());

        m_vendor_name = m_device.getInfo<CL_DEVICE_VENDOR>();
        m_vendor_id = m_device.getInfo<CL_DEVICE_VENDOR_ID>();
        m_max_cu = m_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        m_max_wgs = m_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        m_max_local_mem = m_device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        m_addr_align = m_device.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>() / 8;

        m_default_wgs = default_wgs;
        m_wave_size = wave_size;

        std::stringstream desc;
        desc << "OpenCL Acc " << m_platform.getInfo<CL_PLATFORM_NAME>()
             << " device: " << m_device.getInfo<CL_DEVICE_NAME>()
             << " mcu:" << m_max_cu
             << " wave:" << m_wave_size
             << " mwgs:" << m_max_wgs;
        m_description = desc.str();
        LOG_MSG(Status::Ok, m_description);

        m_context = cl::Context(m_device);
        m_queues.clear();
        m_queues.reserve(queues_count);

        for (int i = 0; i < queues_count; i++) {
            cl_command_queue_properties properties = 0;
            if (profiling) {
                properties |= CL_QUEUE_PROFILING_ENABLE;
            }
            cl::CommandQueue queue(m_context, properties);
            m_queues.emplace_back(std::move(queue));
        }

        m_counter_pool = std::make_unique<CLCounterPool>();
        m_alloc_general = std::make_unique<CLAllocGeneral>();

        if (allocator_type == "linear") {
            m_alloc_linear = std::make_unique<CLAllocLinear>(lin_alloc_size, m_addr_align);
            m_alloc_tmp = m_alloc_linear.get();
        } else {
            m_alloc_tmp = m_alloc_general.get();
        }

        LOG_MSG(Status::Ok, "configure " << queues_count << " queues for computations");
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