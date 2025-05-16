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

#ifndef SPLA_CL_ACCELERATOR_HPP
#define SPLA_CL_ACCELERATOR_HPP

#include <core/accelerator.hpp>
#include <core/common.hpp>
#include <core/logger.hpp>
#include <spla/library.hpp>

#include <string>
#include <vector>

#include <svector.hpp>

#ifndef SPLA_RELEASE
    #define CL_HPP_ENABLE_EXCEPTIONS
#endif

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION  120
#include <CL/opencl.hpp>

#define VENDOR_CODE_NVIDIA "nvidia"
#define VENDOR_CODE_INTEL  "intel"
#define VENDOR_CODE_AMD    "amd"
#define VENDOR_CODE_IMG    "img"

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CLAccelerator
     * @brief Single-device OpenCL acceleration implementation
     */
    class CLAccelerator final : public Accelerator {
    public:
        CLAccelerator();
        ~CLAccelerator() override;

        Status             init() override;
        Status             set_platform(int index) override;
        Status             set_device(int index) override;
        Status             set_queues_count(int count) override;
        const std::string& get_name() override;
        const std::string& get_description() override;
        const std::string& get_suffix() override;

        cl::Platform&         get_platform() { return m_platform; }
        cl::Device&           get_device() { return m_device; }
        cl::Context&          get_context() { return m_context; }
        cl::CommandQueue&     get_queue_default() { return m_queues.front(); }
        class CLProgramCache* get_cache() { return m_cache.get(); }
        class CLCounterPool*  get_counter_pool() { return m_counter_pool.get(); }
        class CLAllocGeneral* get_alloc_general() { return m_alloc_general.get(); }
        class CLAlloc*        get_alloc_tmp() { return m_alloc_tmp; }

        [[nodiscard]] const std::string& get_vendor_name() const { return m_vendor_name; }
        [[nodiscard]] const std::string& get_vendor_code() const { return m_vendor_code; }
        [[nodiscard]] uint               get_vendor_id() const { return m_vendor_id; }
        [[nodiscard]] uint               get_max_cu() const { return m_max_cu; }
        [[nodiscard]] uint               get_max_wgs() const { return m_max_wgs; }
        [[nodiscard]] uint               get_max_local_mem() const { return m_max_local_mem; }
        [[nodiscard]] uint               get_addr_align() const { return m_addr_align; }
        [[nodiscard]] uint               get_default_wgs() const { return m_default_wgs; }
        [[nodiscard]] uint               get_wave_size() const { return m_wave_size; }
        [[nodiscard]] uint               get_num_of_mem_banks() const { return m_num_of_mem_banks; }
        [[nodiscard]] bool               is_nvidia() const { return m_is_nvidia; }
        [[nodiscard]] bool               is_amd() const { return m_is_amd; }
        [[nodiscard]] bool               is_intel() const { return m_is_intel; }
        [[nodiscard]] bool               is_img() const { return m_is_img; }

    private:
        cl::Platform                          m_platform;
        cl::Device                            m_device;
        cl::Context                           m_context;
        std::unique_ptr<class CLProgramCache> m_cache;
        std::unique_ptr<class CLCounterPool>  m_counter_pool;
        std::unique_ptr<class CLAllocLinear>  m_alloc_linear;
        std::unique_ptr<class CLAllocGeneral> m_alloc_general;
        class CLAlloc*                        m_alloc_tmp = nullptr;

        std::string m_name = "OpenCL";
        std::string m_description;
        std::string m_suffix = "__cl";
        std::string m_vendor_name;
        std::string m_vendor_code;
        uint        m_vendor_id        = 0;
        uint        m_max_cu           = 0;
        uint        m_max_wgs          = 0;
        uint        m_max_local_mem    = 0;
        uint        m_addr_align       = 128;
        uint        m_default_wgs      = 64;
        uint        m_wave_size        = 32;
        uint        m_num_of_mem_banks = 32;
        bool        m_is_nvidia        = false;
        bool        m_is_amd           = false;
        bool        m_is_intel         = false;
        bool        m_is_img           = false;

        ankerl::svector<cl::CommandQueue, 2> m_queues;
    };

    /**
     * @brief Returns opencl library accelerator
     *
     * @return OpenCL accelerator if present
     */
    static inline CLAccelerator* get_acc_cl() {
        return dynamic_cast<CLAccelerator*>(Library::get()->get_accelerator());
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_ACCELERATOR_HPP
