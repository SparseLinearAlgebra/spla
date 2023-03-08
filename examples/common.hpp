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

#ifndef SPLA_COMMON_HPP
#define SPLA_COMMON_HPP

#include <spla.hpp>

#include <cmath>
#include <iostream>
#include <vector>

void verify_exact(int a, int b) {
    assert(a == b);

    if (a != b) {
        std::cerr << " VERIFY: expected " << a << " actual " << b << std::endl;
    }
}

void verify_exact(const spla::ref_ptr<spla::Vector>& a, const std::vector<int>& b) {
    const auto N = a->get_n_rows();
    for (spla::uint i = 0; i < N; i++) {
        int expected = b[i];
        int actual;
        a->get_int(i, actual);

        assert(expected == actual);

        if (expected != actual) {
            std::cerr << " VERIFY: " << i << " expected " << expected << " actual " << actual << std::endl;
        }
    }
}

void verify_exact(const spla::ref_ptr<spla::Vector>& a, const std::vector<float>& b, const float error = 0.005f) {
    const auto N = a->get_n_rows();
    for (spla::uint i = 0; i < N; i++) {
        float expected = b[i];
        float actual;
        a->get_float(i, actual);

        bool equals = std::abs(expected - actual) <= error;

        assert(equals);

        if (!equals) {
            std::cerr << " VERIFY: " << i << " expected " << expected << " actual " << actual << std::endl;
        }
    }
}

#endif//SPLA_COMMON_HPP
