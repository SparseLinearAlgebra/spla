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

#ifndef SPLA_TYPETRAITS_HPP
#define SPLA_TYPETRAITS_HPP

#include <cstddef>

namespace utils {

    template<bool IsComparable, typename = void>
    struct Comparable {
        static constexpr bool comparable = IsComparable;
    };

    template<bool DoUseError>
    struct ComparableWithError : Comparable<true> {
        static constexpr bool useError = DoUseError;
    };

    template<typename T, typename = void>
    struct ComparisonHelper : Comparable<false> {
        static constexpr T GetError() {
            // Will not be executed
            return 0;
        }
    };

    template<typename T>
    inline constexpr bool UseError() {
        static_assert(ComparisonHelper<T>::comparable);
        return ComparisonHelper<T>::useError;
    }

    template<typename T>
    inline constexpr T GetError() {
        static_assert(ComparisonHelper<T>::comparable);
        return ComparisonHelper<T>::GetError();
    }

    template<typename T>
    inline constexpr bool EqWithError(T a, T b) {
        if constexpr (UseError<T>()) {
            return std::abs(a - b) < GetError<T>();
        } else {
            return a == b;
        }
    }

    template<typename T>
    struct ComparisonHelper<T, std::enable_if_t<std::is_floating_point_v<T>>> : ComparableWithError<true> {
        static constexpr T GetError() {
            return static_cast<T>(1e-5);
        }
    };

    template<typename T>
    struct ComparisonHelper<T, std::enable_if_t<std::is_integral_v<T>>> : ComparableWithError<false> {
    };

    template<typename T>
    bool EqWithRelativeError(T a, T b, T part = static_cast<T>(0.001)) {
        if (!UseError<T>()) {
            return a == b;
        }
        return (std::abs(a - b) / std::max(a, b)) <= part;
    }

}// namespace utils

#endif//SPLA_TYPETRAITS_HPP
