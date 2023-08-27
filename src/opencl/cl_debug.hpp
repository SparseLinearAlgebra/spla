/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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

#ifndef SPLA_CL_DEBUG_HPP
#define SPLA_CL_DEBUG_HPP

#include <profiling/time_profiler.hpp>

#ifndef SPLA_RELEASE
    #define CL_FINISH(queue) queue.finish()

    #define CL_DISPATCH_PROFILED(name, queue, kernel, ...)                                                        \
        do {                                                                                                      \
            CL_FINISH(queue);                                                                                     \
            {                                                                                                     \
                TIME_PROFILE_SUBSCOPE(name);                                                                      \
                cl::Event __cl_event;                                                                             \
                queue.enqueueNDRangeKernel(kernel, __VA_ARGS__, nullptr, &__cl_event);                            \
                __cl_event.wait();                                                                                \
                cl_ulong __queued                   = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(); \
                cl_ulong __start                    = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();  \
                cl_ulong __end                      = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_END>();    \
                TIME_PROFILE_SUBLABEL.queued_nano   = __end - __queued;                                           \
                TIME_PROFILE_SUBLABEL.executed_nano = __end - __start;                                            \
                CL_FINISH(queue);                                                                                 \
            }                                                                                                     \
        } while (false)

    #define CL_READ_PROFILED(name, queue, buffer, ...)                                                            \
        do {                                                                                                      \
            CL_FINISH(queue);                                                                                     \
            {                                                                                                     \
                TIME_PROFILE_SUBSCOPE(name);                                                                      \
                cl::Event __cl_event;                                                                             \
                queue.enqueueReadBuffer(buffer, __VA_ARGS__, nullptr, &__cl_event);                               \
                __cl_event.wait();                                                                                \
                cl_ulong __queued                   = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(); \
                cl_ulong __start                    = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();  \
                cl_ulong __end                      = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_END>();    \
                TIME_PROFILE_SUBLABEL.queued_nano   = __end - __queued;                                           \
                TIME_PROFILE_SUBLABEL.executed_nano = __end - __start;                                            \
                CL_FINISH(queue);                                                                                 \
            }                                                                                                     \
        } while (false)

    #define CL_COUNTER_GET(name, queue, counter, value)                                                           \
        do {                                                                                                      \
            CL_FINISH(queue);                                                                                     \
            {                                                                                                     \
                TIME_PROFILE_SUBSCOPE(name);                                                                      \
                cl::Event __cl_event;                                                                             \
                (value) = (counter).get(queue, &__cl_event);                                                      \
                __cl_event.wait();                                                                                \
                cl_ulong __queued                   = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(); \
                cl_ulong __start                    = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();  \
                cl_ulong __end                      = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_END>();    \
                TIME_PROFILE_SUBLABEL.queued_nano   = __end - __queued;                                           \
                TIME_PROFILE_SUBLABEL.executed_nano = __end - __start;                                            \
                CL_FINISH(queue);                                                                                 \
            }                                                                                                     \
        } while (false)

    #define CL_COUNTER_SET(name, queue, counter, value)                                                           \
        do {                                                                                                      \
            CL_FINISH(queue);                                                                                     \
            {                                                                                                     \
                TIME_PROFILE_SUBSCOPE(name);                                                                      \
                cl::Event __cl_event;                                                                             \
                (counter).set(queue, (value), &__cl_event);                                                       \
                __cl_event.wait();                                                                                \
                cl_ulong __queued                   = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(); \
                cl_ulong __start                    = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();  \
                cl_ulong __end                      = __cl_event.getProfilingInfo<CL_PROFILING_COMMAND_END>();    \
                TIME_PROFILE_SUBLABEL.queued_nano   = __end - __queued;                                           \
                TIME_PROFILE_SUBLABEL.executed_nano = __end - __start;                                            \
                CL_FINISH(queue);                                                                                 \
            }                                                                                                     \
        } while (false)

    #define CL_PROFILE_BEGIN(name, queue) \
        do {                              \
            auto& __prfq = queue;         \
            CL_FINISH(__prfq);            \
            {                             \
                TIME_PROFILE_SUBSCOPE(name);

    #define CL_PROFILE_END() \
        }                    \
        CL_FINISH(__prfq);   \
        }                    \
        while (false)
#else
    #define CL_FINISH(queue)
    #define CL_DISPATCH_PROFILED(name, queue, kernel, ...) queue.enqueueNDRangeKernel(kernel, __VA_ARGS__)
    #define CL_READ_PROFILED(name, queue, buffer, ...)     queue.enqueueReadBuffer(buffer, __VA_ARGS__)
    #define CL_COUNTER_GET(name, queue, counter, value)    (value) = (counter).get(queue)
    #define CL_COUNTER_SET(name, queue, counter, value)    (counter).set(queue, (value))
    #define CL_PROFILE_BEGIN(name, queue)
    #define CL_PROFILE_END()
#endif

#endif//SPLA_CL_DEBUG_HPP
