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

#ifndef SPLA_TEST_UTILS_RANDOM_HPP
#define SPLA_TEST_UTILS_RANDOM_HPP

#include <random>

namespace testing {

    template<typename T, typename = void>
    class UniformGenerator;

    template<typename T>
    class UniformGenerator<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    public:
        explicit UniformGenerator(std::size_t seed = 0)
            : mEngine(seed) {
        }

        T operator()() {
            return mDist(mEngine);
        }

    private:
        std::default_random_engine mEngine;
        std::uniform_real_distribution<T> mDist;
    };

    template<typename T>
    class UniformGenerator<T, std::enable_if_t<std::is_integral_v<T>>> {
    public:
        explicit UniformGenerator(std::size_t seed = 0,
                                  T min = std::numeric_limits<T>::min(),
                                  T max = std::numeric_limits<T>::max())
            : mEngine(seed),
              mDist(min, max) {}

        T operator()() {
            return mDist(mEngine);
        }

    private:
        std::default_random_engine mEngine;
        std::uniform_int_distribution<T> mDist;
    };

}// namespace testing

#endif//SPLA_TEST_UTILS_RANDOM_HPP
