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

TEST(mxm, naive) {
    spla::uint M = 3, N = 4, K = 2;

    // A
    //
    // 1 0 2 0
    // 0 3 0 4
    // 5 0 6 0

    // B
    //
    // 7 1
    // 0 1
    // 0 1
    // 0 1

    // R
    //
    //  7  3
    //  0  7
    // 35 11

    auto R    = spla::Matrix::make(M, K, spla::FLOAT);
    auto A    = spla::Matrix::make(M, N, spla::FLOAT);
    auto B    = spla::Matrix::make(N, K, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0);

    A->set_float(0, 0, 1.0f);
    A->set_float(0, 2, 2.0f);
    A->set_float(1, 1, 3.0f);
    A->set_float(1, 3, 4.0f);
    A->set_float(2, 0, 5.0f);
    A->set_float(2, 2, 6.0f);

    B->set_float(0, 0, 7.0f);
    B->set_float(0, 1, 1.0f);
    B->set_float(1, 1, 1.0f);
    B->set_float(2, 1, 1.0f);
    B->set_float(3, 1, 1.0f);

    spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init);

    float v;

    R->get_float(0, 0, v);
    EXPECT_EQ(v, 7);

    R->get_float(0, 1, v);
    EXPECT_EQ(v, 3);

    R->get_float(1, 0, v);
    EXPECT_EQ(v, 0);

    R->get_float(1, 1, v);
    EXPECT_EQ(v, 7);

    R->get_float(2, 0, v);
    EXPECT_EQ(v, 35);

    R->get_float(2, 1, v);
    EXPECT_EQ(v, 11);
}

TEST(mxm, perf_zero) {
    spla::uint M = 100, N = 1000, K = 200;

    auto R    = spla::Matrix::make(M, K, spla::FLOAT);
    auto A    = spla::Matrix::make(M, N, spla::FLOAT);
    auto B    = spla::Matrix::make(N, K, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0);

    for (spla::uint i = 0; i < M; i++) {
        for (spla::uint j = 0; j < N; j++) {
            A->set_float(i, j, (i + j) % 2 ? 0.0f : 1.0f);
        }
    }
    for (spla::uint i = 0; i < N; i++) {
        for (spla::uint j = 0; j < K; j++) {
            B->set_float(i, j, (i + j) % 2 ? 1.0f : 0.0f);
        }
    }

    spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init);

    for (spla::uint i = 0; i < M; i++) {
        for (spla::uint j = 0; j < K; j++) {
            float tol = 0.0001;
            float v;
            R->get_float(i, j, v);

            if ((i + j) % 2 == 1) {
                EXPECT_TRUE(std::fabs(v - 500.0f) <= tol);
            } else {
                EXPECT_TRUE(std::fabs(v - 0.0f) <= tol);
            }
        }
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE_PLATFORM(1)
