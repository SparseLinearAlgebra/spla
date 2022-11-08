/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#include <spla/spla.hpp>

TEST(matrix, get_set_naive) {
    const spla::uint M = 10, N = 10, K = 8;
    const spla::uint Ai[K] = {0, 0, 1, 2, 4, 7, 8, 8};
    const spla::uint Aj[K] = {0, 1, 8, 9, 7, 3, 4, 0};
    const int        Ax[K] = {-1, 2, 4, 9, -10, 11, 23, 45};

    auto imat = spla::make_matrix(M, N, spla::INT);

    for (spla::uint k = 0; k < K; ++k) {
        imat->set_int(Ai[k], Aj[k], Ax[k]);
    }

    for (spla::uint k = 0; k < K; ++k) {
        int x;
        imat->get_int(Ai[k], Aj[k], x);
        EXPECT_EQ(x, Ax[k]);
    }
}

TEST(matrix, get_set_reduce_default) {
    const spla::uint M = 10, N = 10, K = 8;
    const spla::uint Ai[K] = {0, 0, 1, 2, 4, 7, 8, 8};
    const spla::uint Aj[K] = {0, 1, 8, 9, 7, 3, 4, 0};
    const int        Ax[K] = {-1, 2, 4, 9, -10, 11, 23, 45};

    auto imat = spla::make_matrix(M, N, spla::INT);

    for (spla::uint k = 0; k < K; ++k) {
        imat->set_int(Ai[k], Aj[k], Ax[k]);
        imat->set_int(Ai[k], Aj[k], Ax[k]);
    }

    for (spla::uint k = 0; k < K; ++k) {
        int x;
        imat->get_int(Ai[k], Aj[k], x);
        EXPECT_EQ(x, Ax[k]);
    }
}

TEST(matrix, get_set_reduce_plus) {
    const spla::uint M = 10, N = 10, K = 8;
    const spla::uint Ai[K] = {0, 0, 1, 2, 4, 7, 8, 8};
    const spla::uint Aj[K] = {0, 1, 8, 9, 7, 3, 4, 0};
    const int        Ax[K] = {-1, 2, 4, 9, -10, 11, 23, 45};

    auto imat = spla::make_matrix(M, N, spla::INT);
    imat->set_reduce(spla::PLUS_INT);

    for (spla::uint k = 0; k < K; ++k) {
        imat->set_int(Ai[k], Aj[k], Ax[k]);
        imat->set_int(Ai[k], Aj[k], Ax[k]);
    }

    for (spla::uint k = 0; k < K; ++k) {
        int x;
        imat->get_int(Ai[k], Aj[k], x);
        EXPECT_EQ(x, Ax[k] + Ax[k]);
    }
}

TEST(matrix, get_set_reduce_mult) {
    const spla::uint M = 10, N = 10, K = 8;
    const spla::uint Ai[K] = {0, 0, 1, 2, 4, 7, 8, 8};
    const spla::uint Aj[K] = {0, 1, 8, 9, 7, 3, 4, 0};
    const int        Ax[K] = {-1, 2, 4, 9, -10, 11, 23, 45};

    auto imat = spla::make_matrix(M, N, spla::INT);

    for (spla::uint k = 0; k < K; ++k) {
        imat->set_int(Ai[k], Aj[k], 4);
    }

    imat->set_reduce(spla::MULT_INT);

    for (spla::uint k = 0; k < K; ++k) {
        imat->set_int(Ai[k], Aj[k], Ax[k]);
    }

    for (spla::uint k = 0; k < K; ++k) {
        int x;
        imat->get_int(Ai[k], Aj[k], x);
        EXPECT_EQ(x, 4 * Ax[k]);
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE