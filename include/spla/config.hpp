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

#ifndef SPLA_CONFIG_HPP
#define SPLA_CONFIG_HPP

#include <cinttypes>
#include <functional>

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
     * @class StateHint
     * @brief Hint used to explicitly prepare matrix state
     */
    enum class StateHint {
        /** Default state of container (empty) */
        Default = 0,
        /** Prepare for incremental build */
        Incremental = 1,
        /** Commits build data to optimal storage format */
        Compute = 2,
    };

    /**
     * @class FormatHint
     * @brief Hint desired storage format of object
     */
    enum class FormatHint {
        Default    = 0,
        CpuLil     = 1,
        CpuCoo     = 2,
        CpuCsr     = 3,
        CpuCsc     = 4,
        AccDefault = 100,
        AccCoo     = 102,
        AccCsr     = 103,
        AccCsc     = 104
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
     * @brief Convert status value to string
     *
     * @param status Status value
     *
     * @return String value
     */
    static const char* to_string(Status status) {
#define STATUS_MAP(value) \
    case Status::value:   \
        return #value

        switch (status) {
            STATUS_MAP(Ok);
            STATUS_MAP(Error);
            STATUS_MAP(NoAcceleration);
            STATUS_MAP(PlatformNotFound);
            STATUS_MAP(DeviceNotFound);
            STATUS_MAP(InvalidState);
            STATUS_MAP(InvalidArgument);
            STATUS_MAP(NotImplemented);
            default:
                return "none";
        }
#undef STATUS_MAP
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CONFIG_HPP
