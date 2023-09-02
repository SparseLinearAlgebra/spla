/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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
#include "test_common.hpp"

#include <iostream>
#include <spla.hpp>

TEST(kron, naive) {
    spla::uint M = 3, K = 4;

    auto A = spla::Matrix::make(M, M, spla::INT);
    auto B = spla::Matrix::make(K, K, spla::INT);
    auto R = spla::Matrix::make(M * K, M * K, spla::INT);

    A->set_int(0, 1, 2);
    A->set_int(1, 0, 2);
    A->set_int(1, 2, 2);
    A->set_int(2, 1, 2);

    for (spla::uint i = 0; i < K; i++) {
        for (spla::uint j = 0; j < K; j++) {
            B->set_int(i, j, 3);
        }
    }

    spla::exec_kron(R, A, B, spla::MULT_INT);

    spla::uint x, y;

    x = 0;
    y = 1;
    for (spla::uint i = 0; i < K; i++) {
        for (spla::uint j = 0; j < K; j++) {
            int v;
            R->get_int(x * K + i, y * K + j, v);
            EXPECT_EQ(6, v);
        }
    }

    x = 1;
    y = 0;
    for (spla::uint i = 0; i < K; i++) {
        for (spla::uint j = 0; j < K; j++) {
            int v;
            R->get_int(x * K + i, y * K + j, v);
            EXPECT_EQ(6, v);
        }
    }

    x = 1;
    y = 2;
    for (spla::uint i = 0; i < K; i++) {
        for (spla::uint j = 0; j < K; j++) {
            int v;
            R->get_int(x * K + i, y * K + j, v);
            EXPECT_EQ(6, v);
        }
    }

    x = 2;
    y = 1;
    for (spla::uint i = 0; i < K; i++) {
        for (spla::uint j = 0; j < K; j++) {
            int v;
            R->get_int(x * K + i, y * K + j, v);
            EXPECT_EQ(6, v);
        }
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE_PLATFORM(1)
