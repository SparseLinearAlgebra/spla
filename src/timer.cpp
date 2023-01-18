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

#include <spla/timer.hpp>

namespace spla {

    void Timer::start() {
        m_laps.clear();
        m_start = m_prev = m_end = clock::now();
    }
    void Timer::stop() {
        m_prev = m_end = clock::now();
    }
    void Timer::lap_begin() {
        m_prev = clock::now();
    }
    void Timer::lap_end() {
        auto lap_start = m_prev;
        m_prev         = clock::now();
        m_laps.push_back(static_cast<double>(std::chrono::duration_cast<us>(m_prev - lap_start).count()) * 1e-3);
    }

    void Timer::print(std::ostream& out) const {
        for (auto t : m_laps) out << t << ", ";
    }

    double Timer::get_elapsed_ms() const {
        return static_cast<double>(std::chrono::duration_cast<us>(clock::now() - m_start).count()) * 1e-3;
    }
    double Timer::get_elapsed_sec() const {
        return static_cast<double>(std::chrono::duration_cast<us>(clock::now() - m_start).count()) * 1e-6;
    }
    double Timer::get_elapsed_lap_ms() const {
        return static_cast<double>(std::chrono::duration_cast<us>(clock::now() - m_prev).count()) * 1e-3;
    }
    const std::vector<double>& Timer::get_laps_ms() const {
        return m_laps;
    }

    Timer::~Timer() = default;
    Timer::Timer()  = default;


}// namespace spla
