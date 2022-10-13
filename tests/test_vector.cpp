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

#include "test_common.hpp"

#include <spla/spla.hpp>

TEST(vector, get_set_naive) {
    const spla::uint N    = 10;
    const int        X[N] = {1, 2, 3, 4, 5, -3, -3, 5, -8, 1};

    auto ivec = spla::make_vector(N, spla::INT);

    for (spla::uint i = 0; i < N; ++i) {
        ivec->set_int(i, X[i]);
    }

    for (spla::uint i = 0; i < N; ++i) {
        int x;
        ivec->get_int(i, x);
        EXPECT_EQ(x, X[i]);
    }
}

TEST(vector, get_set_reduce_default) {
    const spla::uint N    = 10;
    const int        X[N] = {1, 2, 3, 4, 5, -3, -3, 5, -8, 1};

    auto ivec = spla::make_vector(N, spla::INT);

    for (spla::uint i = 0; i < N; ++i) {
        ivec->set_int(i, X[i]);
        ivec->set_int(i, X[i]);
    }

    for (spla::uint i = 0; i < N; ++i) {
        int x;
        ivec->get_int(i, x);
        EXPECT_EQ(x, X[i]);
    }
}

TEST(vector, reduce_plus) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    const int        X[K] = {1, 2, 3, 4, 5, -3, -3, 5};

    auto ivec   = spla::make_vector(N, spla::INT);
    auto ir     = spla::make_scalar(spla::INT);
    auto istart = spla::make_int(0);
    int  isum   = 0;

    for (int k = 0; k < K; ++k) {
        ivec->set_int(I[k], X[k]);
        isum += X[k];
    }

    spla::execute_immediate(spla::make_sched_v_reduce(ir, istart, ivec, spla::PLUS_INT));

    int result;
    ir->get_int(result);

    EXPECT_EQ(result, isum);
}

TEST(vector, reduce_mult) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    const int        X[K] = {1, 2, 3, 4, 5, -3, -3, 5};

    auto ivec   = spla::make_vector(N, spla::INT);
    auto ir     = spla::make_scalar(spla::INT);
    auto istart = spla::make_int(1);
    int  isum   = 1;

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 1);
    }

    for (int k = 0; k < K; ++k) {
        ivec->set_int(I[k], X[k]);
        isum *= X[k];
    }

    spla::execute_immediate(spla::make_sched_v_reduce(ir, istart, ivec, spla::MULT_INT));

    int result;
    ir->get_int(result);

    EXPECT_EQ(result, isum);
}

TEST(vector, assign_plus) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        S    = -5;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    int              R[N];

    auto ivec  = spla::make_vector(N, spla::INT);
    auto imask = spla::make_vector(N, spla::INT);
    auto ival  = spla::make_int(S);

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 14);
        imask->set_int(k, 0);
        R[k] = 14;
    }

    for (int k = 0; k < K; ++k) {
        imask->set_int(I[k], 1);
        R[I[k]] = R[I[k]] + S;
    }

    spla::execute_immediate(spla::make_sched_v_assign_masked(ivec, imask, ival, spla::PLUS_INT));

    for (int k = 0; k < N; k++) {
        int r;
        ivec->get_int(k, r);
        EXPECT_EQ(R[k], r);
    }
}

TEST(vector, assign_second) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        S    = -5;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    int              R[N];

    auto ivec  = spla::make_vector(N, spla::INT);
    auto imask = spla::make_vector(N, spla::INT);
    auto ival  = spla::make_int(S);

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 14);
        imask->set_int(k, 0);
        R[k] = 14;
    }

    for (int k = 0; k < K; ++k) {
        imask->set_int(I[k], 1);
        R[I[k]] = S;
    }

    spla::execute_immediate(spla::make_sched_v_assign_masked(ivec, imask, ival, spla::SECOND_INT));

    for (int k = 0; k < N; k++) {
        int r;
        ivec->get_int(k, r);
        EXPECT_EQ(R[k], r);
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE