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

#ifndef SPLA_ACCELERATOR_HPP
#define SPLA_ACCELERATOR_HPP

#include <spla/config.hpp>

#include <string>

namespace spla {

    /**
     * @class Accelerator
     * @brief Interface for an computations acceleration backend
     *
     * Accelerator is an optional library computations backend, which
     * may provided customized and efficient implementations of some operations
     * over matrices and vectors.
     *
     * Accelerator can implement additional and custom storage schemas on top of
     * the default schemas in matrices and vectors and optional store any data
     * along with default in order to speed-up computations.
     *
     * Typical accelerator implementation is a GPUs utilization by usage of
     * OpenCL or CUDA API. In this case additional device resident data stored
     * with host data and kernels dispatched in order to perform computations.
     */
    class Accelerator {
    public:
        virtual ~Accelerator() = default;

        virtual Status init() = 0;

        virtual Status set_platform(int index) = 0;
        virtual Status set_device(int index) = 0;
        virtual Status set_queues_count(int count) = 0;

        virtual std::string get_name() = 0;
        virtual std::string get_description() = 0;
    };

}// namespace spla

#endif//SPLA_ACCELERATOR_HPP
