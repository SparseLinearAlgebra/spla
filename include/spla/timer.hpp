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

#ifndef SPLA_TIMER_HPP
#define SPLA_TIMER_HPP

#include "config.hpp"

#include <chrono>
#include <iostream>
#include <ostream>
#include <vector>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Timer
     * @brief Simple timer to measure intervals of time on CPU-side
     */
    class Timer {
    public:
        SPLA_API Timer();
        SPLA_API ~Timer();

        SPLA_API void                 start();
        SPLA_API void                 stop();
        SPLA_API void                 lap_begin();
        SPLA_API void                 lap_end();
        SPLA_API void                 print(std::ostream& out = std::cout) const;
        [[nodiscard]] SPLA_API double get_elapsed_ms() const;
        [[nodiscard]] SPLA_API double get_elapsed_sec() const;
        [[nodiscard]] SPLA_API double get_elapsed_lap_ms() const;
        [[nodiscard]] SPLA_API const std::vector<double>& get_laps_ms() const;

    private:
        using clock = std::chrono::steady_clock;
        using us    = std::chrono::microseconds;
        using point = clock::time_point;

        std::vector<double> m_laps;
        point               m_start;
        point               m_prev;
        point               m_end;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TIMER_HPP
