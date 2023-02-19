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

#include "test_common.hpp"

#include <spla.hpp>

TEST(vector, get_set_naive) {
    const spla::uint N    = 10;
    const int        X[N] = {1, 2, 3, 4, 5, -3, -3, 5, -8, 1};

    auto ivec = spla::Vector::make(N, spla::INT);

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

    auto ivec = spla::Vector::make(N, spla::INT);

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

TEST(vector, get_set_reduce_plus) {
    const spla::uint N    = 10;
    const int        X[N] = {1, 2, 3, 4, 5, -3, -3, 5, -8, 1};

    auto ivec = spla::Vector::make(N, spla::INT);
    ivec->set_reduce(spla::PLUS_INT);

    for (spla::uint i = 0; i < N; ++i) {
        ivec->set_int(i, X[i]);
        ivec->set_int(i, X[i]);
    }

    for (spla::uint i = 0; i < N; ++i) {
        int x;
        ivec->get_int(i, x);
        EXPECT_EQ(x, X[i] + X[i]);
    }
}

TEST(vector, get_set_reduce_mult) {
    const spla::uint N    = 10;
    const int        X[N] = {1, 2, 3, 4, 5, -3, -3, 5, -8, 1};

    auto ivec = spla::Vector::make(N, spla::INT);

    for (spla::uint i = 0; i < N; ++i) {
        ivec->set_int(i, 4);
    }

    ivec->set_reduce(spla::MULT_INT);

    for (spla::uint i = 0; i < N; ++i) {
        ivec->set_int(i, X[i]);
    }

    for (spla::uint i = 0; i < N; ++i) {
        int x;
        ivec->get_int(i, x);
        EXPECT_EQ(x, 4 * X[i]);
    }
}

TEST(vector, reduce_plus) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    const int        X[K] = {1, 2, 3, 4, 5, -3, -3, 5};

    auto ivec   = spla::Vector::make(N, spla::INT);
    auto ir     = spla::Scalar::make(spla::INT);
    auto istart = spla::Scalar::make_int(0);
    int  isum   = 0;

    for (int k = 0; k < K; ++k) {
        ivec->set_int(I[k], X[k]);
        isum += X[k];
    }

    spla::exec_v_reduce(ir, istart, ivec, spla::PLUS_INT);

    int result;
    ir->get_int(result);

    EXPECT_EQ(result, isum);
}

TEST(vector, reduce_mult) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    const int        X[K] = {1, 2, 3, 4, 5, -3, -3, 5};

    auto ivec   = spla::Vector::make(N, spla::INT);
    auto ir     = spla::Scalar::make(spla::INT);
    auto istart = spla::Scalar::make_int(1);
    int  isum   = 1;

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 1);
    }

    for (int k = 0; k < K; ++k) {
        ivec->set_int(I[k], X[k]);
        isum *= X[k];
    }

    spla::exec_v_reduce(ir, istart, ivec, spla::MULT_INT);

    int result;
    ir->get_int(result);

    EXPECT_EQ(result, isum);
}

TEST(vector, reduce_perf) {
    const int N     = 10000000;
    const int K     = 5000;
    const int NITER = 20;
    int       R     = 0;

    spla::Timer timer;

    auto ivec   = spla::Vector::make(N, spla::INT);
    auto ir     = spla::Scalar::make(spla::INT);
    auto istart = spla::Scalar::make_int(0);

    for (int i = 0; i < N; ++i) {
        if ((i % K) == 0) {
            ivec->set_int(i, 1);
            R += 1;
        }
    }

    for (int i = 0; i < NITER; ++i) {
        timer.lap_begin();
        spla::exec_v_reduce(ir, istart, ivec, spla::PLUS_INT);
        timer.lap_end();
    }

    int r;
    ir->get_int(r);
    EXPECT_EQ(r, R);

    std::cout << "timings (ms): ";
    timer.print();
    std::cout << std::endl;
}

TEST(vector, eadd_fdb_min) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        S    = 5;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    const int        V[K] = {8, 6, 5, 4, 3, 7, 1, 1};
    int              R[N];
    int              F[N];

    auto ir   = spla::Vector::make(N, spla::INT);
    auto iv   = spla::Vector::make(N, spla::INT);
    auto ifdb = spla::Vector::make(N, spla::INT);

    for (int k = 0; k < N; ++k) {
        ir->set_int(k, S);
        R[k] = S;
        F[k] = 0;
    }

    for (int k = 0; k < K; ++k) {
        iv->set_int(I[k], V[k]);
        R[I[k]] = std::min(R[I[k]], V[k]);
        if (R[I[k]] != S) F[I[k]] = R[I[k]];
    }

    spla::exec_v_eadd_fdb(ir, iv, ifdb, spla::MIN_INT);

    for (int k = 0; k < N; k++) {
        int r;
        ir->get_int(k, r);
        EXPECT_EQ(R[k], r);
    }

    for (int k = 0; k < N; k++) {
        int r;
        ifdb->get_int(k, r);
        EXPECT_EQ(F[k], r);
    }
}

TEST(vector, eadd_fdb_custom) {
    const spla::uint N = 10000;
    auto             v = spla::Vector::make(N, spla::FLOAT);
    auto             u = spla::Vector::make(N, spla::FLOAT);
    auto             f = spla::Vector::make(N, spla::FLOAT);

    for (spla::uint i = 0; i < N; i++) {
        v->set_float(i, float(i));
        u->set_float(i, float(N) - float(i) * float(i));
    }

    v->set_format(spla::FormatVector::AccDense);
    u->set_format(spla::FormatVector::AccDense);

    auto custom_plus = spla::OpBinary::make_float(
            "custom_plus",
            "(float a, float b) { return 0.25f * a + 0.75f * b; }",
            [](float a, float b) { return 0.25f * a + 0.75f * b; });

    spla::exec_v_eadd_fdb(v, u, f, custom_plus);

    auto ref = [](float a, float b) { return 0.25f * a + 0.75f * b; };

    for (spla::uint i = 0; i < N; i++) {
        const float error = 0.005f;
        float       actual;
        v->get_float(i, actual);
        float expected = ref(float(i), float(N) - float(i) * float(i));
        EXPECT_TRUE(std::fabs(actual - expected) <= error);
    }
}

TEST(vector, assign_plus) {
    const spla::uint N    = 20;
    const spla::uint K    = 8;
    const int        S    = -5;
    const int        I[K] = {0, 2, 3, 5, 10, 12, 15, 16};
    int              R[N];

    auto ivec  = spla::Vector::make(N, spla::INT);
    auto imask = spla::Vector::make(N, spla::INT);
    auto ival  = spla::Scalar::make_int(S);

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 14);
        imask->set_int(k, 0);
        R[k] = 14;
    }

    for (int k = 0; k < K; ++k) {
        imask->set_int(I[k], 1);
        R[I[k]] = R[I[k]] + S;
    }

    spla::exec_v_assign_masked(ivec, imask, ival, spla::PLUS_INT, spla::NQZERO_INT);

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

    auto ivec  = spla::Vector::make(N, spla::INT);
    auto imask = spla::Vector::make(N, spla::INT);
    auto ival  = spla::Scalar::make_int(S);

    for (int k = 0; k < N; ++k) {
        ivec->set_int(k, 14);
        imask->set_int(k, 0);
        R[k] = 14;
    }

    for (int k = 0; k < K; ++k) {
        imask->set_int(I[k], 1);
        R[I[k]] = S;
    }

    spla::exec_v_assign_masked(ivec, imask, ival, spla::SECOND_INT, spla::NQZERO_INT);

    for (int k = 0; k < N; k++) {
        int r;
        ivec->get_int(k, r);
        EXPECT_EQ(R[k], r);
    }
}

TEST(vector, assign_perf) {
    const int N     = 7000000;
    const int K     = 1000;
    const int B     = 10;
    const int V     = 20;
    const int NITER = 10;

    spla::Timer timer;

    auto ivec  = spla::Vector::make(N, spla::INT);
    auto imask = spla::Vector::make(N, spla::INT);
    auto ival  = spla::Scalar::make_int(V);

    for (int i = 0; i < N; ++i) {
        ivec->set_int(i, B);
        if (!(i % K)) imask->set_int(i, V);
    }

    for (int i = 0; i < NITER; ++i) {
        timer.lap_begin();
        spla::exec_v_assign_masked(ivec, imask, ival, spla::SECOND_INT, spla::NQZERO_INT);
        timer.lap_end();
    }

    for (int i = 0; i < N; ++i) {
        int r;
        ivec->get_int(i, r);
        EXPECT_EQ(r, (i % K ? B : V));
    }

    std::cout << "timings (ms): ";
    timer.print();
    std::cout << std::endl;
}

TEST(vector, sorting_naive) {
    const int        F     = 1000;
    const int        W     = 4;
    const int        N     = F * W;
    const int        NITER = 10;
    std::vector<int> setup(N);
    std::vector<int> sorting(N);
    std::vector<int> res;

    spla::Timer timer;

    for (int iter = 0; iter < NITER; ++iter) {
        for (int i = 0; i < N; ++i) {
            setup[i] = (i * i + i * W + F);
        }

        timer.lap_begin();
        std::copy(setup.begin(), setup.end(), sorting.begin());
        std::sort(sorting.begin(), sorting.end());
        int sum = 0;
        for (auto val : sorting) sum += val;
        res.push_back(sum);
        timer.lap_end();
    }

    std::cout << "timings (ms): ";
    timer.print();
    std::cout << std::endl;

    for (auto r : res) std::cout << r << std::endl;
}

TEST(vector, fill_value) {
    const int fill = 13;
    const int set  = 7;
    const int N    = 10000;
    const int S    = 2;

    auto v = spla::Vector::make(N, spla::INT);

    v->set_fill_value(spla::Scalar::make_int(fill));

    for (int i = 0; i < N; i++) {
        if (i % S) v->set_int(i, set);
    }

    for (int i = 0; i < N; i++) {
        int r;
        v->get_int(i, r);
        EXPECT_EQ(r, (i % S ? set : fill));
    }

    v->set_fill_value(spla::Scalar::make_int(set));

    for (int i = 0; i < N; i++) {
        int r;
        v->get_int(i, r);
        EXPECT_EQ(r, set);
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE_PLATFORM(1)