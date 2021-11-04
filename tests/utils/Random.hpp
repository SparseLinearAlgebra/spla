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

#ifndef SPLA_RANDOM_HPP
#define SPLA_RANDOM_HPP

#include <random>

namespace utils {

    template<typename T>
    class UniformRealGenerator {
    public:
        explicit UniformRealGenerator(std::size_t seed = 0)
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
    class UniformIntGenerator {
    public:
        explicit UniformIntGenerator(std::size_t seed = 0)
            : mEngine(seed) {
        }

        T operator()() {
            return mDist(mEngine);
        }

    private:
        std::default_random_engine mEngine;
        std::uniform_int_distribution<T> mDist;
    };

    template<typename T>
    class UniformGenerator;

    template<>
    class UniformGenerator<float> : public UniformRealGenerator<float> {};

    template<>
    class UniformGenerator<std::int32_t> : public UniformIntGenerator<std::int32_t> {};

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    std::vector<T> GenerateIntVector(const std::size_t size, std::size_t seed = 0) {
        std::vector<T> vec(size);
        std::generate_n(vec.begin(), size, UniformIntGenerator<T>{seed});
        return vec;
    }

    template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    std::vector<T> GenerateFloatVector(const std::size_t size, std::size_t seed = 0) {
        std::vector<T> vec(size);
        std::generate_n(vec.begin(), size, UniformRealGenerator<T>{seed});
        return vec;
    }
}// namespace utils

#endif//SPLA_RANDOM_HPP
