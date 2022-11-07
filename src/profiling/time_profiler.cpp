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

#include "time_profiler.hpp"

#include <spla/library.hpp>

namespace spla {

    TimeProfilerLabel::TimeProfilerLabel(const char* in_name, const char* in_file, const char* in_function) {
        name     = in_name;
        file     = in_file;
        function = in_function;

        get_library()->get_time_profiler()->add_label(this);
    }

    TimeProfilerScope::TimeProfilerScope(TimeProfilerLabel* in_label) {
        label = in_label;
        start = end = std::chrono::steady_clock::now();
    }

    TimeProfilerScope::~TimeProfilerScope() {
        end          = std::chrono::steady_clock::now();
        auto elapsed = end - start;
        label->nano.store(std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count());
    }

    void TimeProfiler::add_label(TimeProfilerLabel* label) {
        m_labels[label->name] = label;
    }

    void TimeProfiler::dump(std::ostream& where) {
        for (const auto& entry : m_labels) {
            const auto& name  = entry.first;
            const auto& label = entry.second;
            where << "  - " << name << " " << static_cast<double>(label->nano.load()) * 1e-6 << " ms\n";
        }
    }

}// namespace spla