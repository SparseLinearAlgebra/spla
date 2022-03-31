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

#ifndef SPLA_REFERENCE_BACKEND_HPP
#define SPLA_REFERENCE_BACKEND_HPP

#include <spla/config.hpp>

namespace spla::backend {

    /**
     * @addtogroup reference
     * @{
     */

    inline std::size_t &device_count_ref() {
        static std::size_t s_device_count = 1;
        return s_device_count;
    }

    inline std::size_t device_count() {
        return device_count_ref();
    }

    inline std::size_t device_for_task(std::size_t task_id) {
        return task_id % device_count();
    }

    /**
     * @brief Initialize reference backend
     * @param config
     */
    inline void initialize(const Config &config) {
        if (config.get_workers_count().has_value())
            device_count_ref() = config.get_workers_count().value();
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_REFERENCE_BACKEND_HPP
