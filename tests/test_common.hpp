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

#ifndef SPLA_TEST_COMMON_HPP
#define SPLA_TEST_COMMON_HPP

#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

struct pair_hash {
public:
    template<typename T, typename U>
    std::size_t operator()(const std::pair<T, U>& x) const {
        return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
    }
};

#define SPLA_GTEST_MAIN                                  \
    int main(int argc, char* argv[]) {                   \
        ::testing::GTEST_FLAG(catch_exceptions) = false; \
        ::testing::InitGoogleTest(&argc, argv);          \
        auto lib = spla::Library::get(argc, argv);       \
        int __ret = RUN_ALL_TESTS();                     \
        lib->finalize();                                 \
        return __ret;                                    \
    }

// Put in the end of the unit test file
#define SPLA_GTEST_MAIN_NO_SPLA                          \
    int main(int argc, char* argv[]) {                   \
        ::testing::GTEST_FLAG(catch_exceptions) = false; \
        ::testing::InitGoogleTest(&argc, argv);          \
        return RUN_ALL_TESTS();                          \
    }

#endif//SPLA_TEST_COMMON_HPP
