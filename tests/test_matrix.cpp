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

TEST(matrix, get_set_naive) {
    const spla::uint M = 10, N = 10, K = 8;
    const spla::uint Ai[K] = {0, 0, 1, 2, 4, 7, 8, 8};
    const spla::uint Aj[K] = {0, 1, 8, 9, 7, 3, 4, 0};
    const int        Ax[K] = {-1, 2, 4, 9, -10, 11, 23, 45};

    auto imat = spla::Matrix::make(M, N, spla::INT);

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

    auto imat = spla::Matrix::make(M, N, spla::INT);

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

    auto imat = spla::Matrix::make(M, N, spla::INT);
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

    auto imat = spla::Matrix::make(M, N, spla::INT);

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

TEST(matrix, reduce_by_row) {
    const spla::uint M = 10000, N = 20000, K = 8;

    auto imat  = spla::Matrix::make(M, N, spla::INT);
    auto ivec  = spla::Vector::make(M, spla::INT);
    auto iinit = spla::Scalar::make_int(0);

    std::vector<int> ref(M, 0);

    for (spla::uint i = 0; i < M; i += 1) {
        for (spla::uint k = 0; k < K; k++) {
            spla::uint j = (i * K + k) % N;
            imat->set_int(i, j, k + 1);
            ref[i] += k + 1;
        }
    }

    spla::exec_m_reduce_by_row(ivec, imat, spla::PLUS_INT, iinit);

    for (spla::uint i = 0; i < M; i += 1) {
        int expected = ref[i];
        int actual;
        ivec->get_int(i, actual);
        EXPECT_EQ(expected, actual);
    }
}

TEST(matrix, reduce_by_column) {
    const spla::uint M = 10000, N = 20000, K = 8;

    auto imat  = spla::Matrix::make(N, M, spla::INT);
    auto ivec  = spla::Vector::make(N, spla::INT);
    auto iinit = spla::Scalar::make_int(0);

    std::vector<int> ref(M, 0);

    for (spla::uint i = 0; i < M; i += 1) {
        for (spla::uint k = 0; k < K; k++) {
            spla::uint j = (i * K + k) % N;
            imat->set_int(j, i, k + 1);
            ref[i] += k + 1;
        }
    }

    spla::exec_m_reduce_by_column(ivec, imat, spla::PLUS_INT, iinit);

    for (spla::uint i = 0; i < M; i += 1) {
        int expected = ref[i];
        int actual;
        ivec->get_int(i, actual);
        EXPECT_EQ(expected, actual);
    }
}


TEST(matrix, reduce) {
    const spla::uint M = 10000, N = 20000, K = 8;

    auto ir    = spla::Scalar::make(spla::INT);
    auto iinit = spla::Scalar::make_int(0);
    auto imat  = spla::Matrix::make(M, N, spla::INT);

    for (spla::uint i = 0; i < M; i += 1) {
        for (spla::uint k = 0; k < K; k++) {
            spla::uint j = (i * K + k) % N;
            imat->set_int(i, j, 2);
        }
    }

    spla::exec_m_reduce(ir, iinit, imat, spla::PLUS_INT);

    EXPECT_EQ(ir->as_int(), M * K * 2);
}

TEST(matrix, transpose) {
    const spla::uint M = 100, N = 200, K = 8;

    auto iM = spla::Matrix::make(M, N, spla::INT);
    auto iR = spla::Matrix::make(N, M, spla::INT);

    for (spla::uint i = 0; i < M; i += 1) {
        for (spla::uint j = 0; j < N; j += 1) {
            if ((i + j) % 2) iM->set_int(i, j, i * 10 + j);
        }
    }

    spla::exec_m_transpose(iR, iM, spla::AINV_INT);

    for (spla::uint i = 0; i < M; i += 1) {
        for (spla::uint j = 0; j < N; j += 1) {
            int v;
            iR->get_int(j, i, v);
            if ((i + j) % 2) {
                EXPECT_EQ(-(i * 10 + j), v);
            } else {
                EXPECT_EQ(0, v);
            }
        }
    }
}

SPLA_GTEST_MAIN_WITH_FINALIZE