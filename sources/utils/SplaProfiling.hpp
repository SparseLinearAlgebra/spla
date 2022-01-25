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

#ifndef SPLA_SPLAPROFILING_H
#define SPLA_SPLAPROFILING_H

#ifdef SPLA_PROFILING
    #include <spla-cpp/SplaUtils.hpp>

    #define PF_SCOPE(tm, header)     \
        std::string tm##_h = header; \
        CpuTimer tm;                 \
        tm.Start();

    #define PF_SCOPE_SHOW(tm, msg) \
        std::cout << tm##_h << " (" << tm.GetElapsedMs() << ") " << msg << std::endl;

    #define PF_SCOPE_MARK(tm, msg)                                                                                 \
        queue.finish();                                                                                            \
        tm.Stop();                                                                                                 \
        std::cout << tm##_h << " (" << tm.GetElapsedMs() << ") " << msg << " " << tm.GetDurationMs() << std::endl; \
        tm.Start();

    #define PF_MARKER_START(tm, header) \
        std::string tm##_h = header;    \
        CpuTimer tm;                    \
        tm.Start();

    #define PF_MARKER_STOR(tm, msg) \
        queue.finish();             \
        tm.Stop();                  \
        std::cout << tm##_h << msg << " " << tm.GetElapsedMs() << std::endl;
#else
    #define PF_SCOPE(tm, header)
    #define PF_SCOPE_SHOW(tm, msg)
    #define PF_SCOPE_MARK(tm, msg)
    #define PF_MARKER_START(tm, header)
    #define PF_MARKER_STOR(tm, msg)
#endif

#endif//SPLA_SPLAPROFILING_H
