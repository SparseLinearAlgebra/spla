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

#ifndef SPLA_CONFIG_HPP
#define SPLA_CONFIG_HPP

#include <cinttypes>
#include <functional>
#include <string>

#ifdef SPLA_MSVC
    #ifdef SPLA_EXPORTS
        #define SPLA_API __declspec(dllexport)
    #else
        #define SPLA_API __declspec(dllimport)
    #endif
#else
    #define SPLA_API
#endif

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @brief Library index and size type
     */
    using uint = std::uint32_t;

    /**
     * @class Status
     * @brief Status of library operation execution
     */
    enum class Status : int {
        /** No error */
        Ok = 0,
        /** Some error occurred */
        Error = 1,
        /** Library has no configured accelerator for computations */
        NoAcceleration = 2,
        /** Accelerator platform not found */
        PlatformNotFound = 3,
        /** Accelerator device not found */
        DeviceNotFound = 4,
        /** Call of the function is not possible for some context */
        InvalidState = 5,
        /** Passed invalid argument for some function */
        InvalidArgument = 6,
        /** No such requested value in matrix, vector or scalar storage */
        NoValue = 7,
        /** Failed to compile GPU/ACC kernel */
        CompilationError = 8,
        /** Some library feature is not implemented */
        NotImplemented = 1024
    };

    /**
     * @class AcceleratorType
     * @brief Types of supported accelerators for computations
     */
    enum class AcceleratorType {
        /** No acceleration to be used */
        None = 0,
        /** OpenCL-based single device acceleration */
        OpenCL = 1
    };

    /**
     * @class FormatMatrix
     * @brief Named storage format for library matrix data objects
     *
     * @warning Do not change order and values
     */
    enum class FormatMatrix {
        /** Matrix list of lists format for fast increment build */
        CpuLil = 0,
        /** Matrix dictionary of keys for fast look-up of values */
        CpuDok = 1,
        /** Matrix coordinates list format */
        CpuCoo = 2,
        /** Matrix compressed sparse rows format */
        CpuCsr = 3,
        /** Matrix compressed sparse columns format */
        CpuCsc = 4,
        /** Matrix acceleration structured coo format */
        AccCoo = 5,
        /** Matrix acceleration structured csr format */
        AccCsr = 6,
        /** Matrix acceleration structured csc format */
        AccCsc = 7,
        /** Total number of supported matrix formats */
        Count = 8
    };

    /**
     * @class FormatVector
     * @brief Named storage format for library vector data objects
     *
     * @warning Do not change order and values
     */
    enum class FormatVector {
        /** Vector dictionary of keys representation */
        CpuDok = 0,
        /** Vector dense array of values representation */
        CpuDense = 1,
        /** Vector list of values for sparse data */
        CpuCoo = 2,
        /** Vector acceleration structured dense format */
        AccDense = 3,
        /** Vector acceleration structured coo format */
        AccCoo = 4,
        /** Total number of supported vector formats */
        Count = 5
    };

    /**
     * @class MessageCallback
     * @brief Callback function called on library message event
     *
     * Message callback function is called on library log.
     * Callback accepts message status, actual textual message with description,
     * file name and function with line location of message dispatch place.
     *
     * Use this message callback to receive library messages (in debug mode especially).
     */
    using MessageCallback = std::function<void(Status             status,
                                               const std::string& msg,
                                               const std::string& file,
                                               const std::string& function,
                                               int                line)>;

    /**
     * @class ScheduleCallback
     * @brief Callback function which can be scheduled in schedule
     *
     * Scheduled callback function allows to schedule your callable
     * object inside any schedule step. It allows user track the progress
     * of schedule execution and allows perform some tasks inside running schedule.
     */
    using ScheduleCallback = std::function<void()>;

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CONFIG_HPP
