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

#ifndef SPLA_COMMON_HPP
#define SPLA_COMMON_HPP

#include <spla/config.hpp>

#include <cmath>

namespace spla {

    static inline uint clamp(uint x, uint left, uint right) {
        return std::min(std::max(x, left), right);
    }

    static inline uint div_up(uint what, uint by) {
        return what / by + (what % by ? 1 : 0);
    }

    static inline uint div_up_clamp(uint what, uint by, uint left, uint right) {
        return clamp(div_up(what, by), left, right);
    }

    static inline uint align(uint what, uint alignment) {
        return what + (what % alignment ? alignment - (what % alignment) : 0);
    }

    static inline uint ceil_to_pow2(uint n) {
        uint r = 1;
        while (r < n) r *= 2u;
        return r;
    }

    static inline uint floor_to_pow2(uint n) {
        uint r = 1;
        while (r <= n) {
            r *= 2;
        }
        return r / 2;
    }

}// namespace spla

#endif//SPLA_COMMON_HPP
