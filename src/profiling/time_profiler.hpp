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

#ifndef SPLA_TIME_PROFILER_HPP
#define SPLA_TIME_PROFILER_HPP

#include <atomic>
#include <chrono>
#include <map>
#include <ostream>
#include <string>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    struct TimeProfilerLabel {
        TimeProfilerLabel(const char* in_name, const char* in_file, const char* in_function);

        const char*          name;
        const char*          file;
        const char*          function;
        std::atomic_uint64_t nano;
    };

    struct TimeProfilerScope {
        explicit TimeProfilerScope(TimeProfilerLabel* in_label);
        ~TimeProfilerScope();

        TimeProfilerLabel*                    label;
        std::chrono::steady_clock::time_point start;
        std::chrono::steady_clock::time_point end;
    };

    /**
     * @class TimeProfiler
     * @brief Scope-based time profiler to measure perf of schedule tasks execution
     */
    class TimeProfiler final {
    public:
        void add_label(TimeProfilerLabel* label);
        void dump(std::ostream& where);
        void reset();

    private:
        std::map<std::string, TimeProfilerLabel*> m_labels;
    };

    /**
     * @}
     */

#ifndef SPLA_RELEASE
    #define TIME_PROFILE_SCOPE(label)                                                     \
        static TimeProfilerLabel __time_prof_static_laber(label, __FILE__, __FUNCTION__); \
        TimeProfilerScope        __time_prof_transient_scope(&__time_prof_static_laber);
#else
    #define TIME_PROFILE_SCOPE(label)
#endif

}// namespace spla

#endif//SPLA_TIME_PROFILER_HPP
