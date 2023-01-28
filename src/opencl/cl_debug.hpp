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

#ifndef SPLA_CL_DEBUG_HPP
#define SPLA_CL_DEBUG_HPP

#include <profiling/time_profiler.hpp>

#ifndef SPLA_RELEASE
    #define CL_FINISH(queue) queue.finish()

    #define CL_DISPATCH_PROFILED(name, queue, kernel, ...)       \
        do {                                                     \
            CL_FINISH(queue);                                    \
            {                                                    \
                TIME_PROFILE_SUBSCOPE(name);                     \
                queue.enqueueNDRangeKernel(kernel, __VA_ARGS__); \
                CL_FINISH(queue);                                \
            }                                                    \
        } while (false)

    #define CL_READ_PROFILED(name, queue, buffer, ...)        \
        do {                                                  \
            CL_FINISH(queue);                                 \
            {                                                 \
                TIME_PROFILE_SUBSCOPE(name);                  \
                queue.enqueueReadBuffer(buffer, __VA_ARGS__); \
                CL_FINISH(queue);                             \
            }                                                 \
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
    #define CL_DISPATCH_PROFILED(name, queue, kernel, ...) queue.enqueueNDRangeKernel(kernel, __VA_ARGS__);
    #define CL_READ_PROFILED(name, queue, buffer, ...)     queue.enqueueReadBuffer(buffer, __VA_ARGS__);
    #define CL_PROFILE_BEGIN(name, queue)
    #define CL_PROFILE_END()
#endif

#endif//SPLA_CL_DEBUG_HPP
