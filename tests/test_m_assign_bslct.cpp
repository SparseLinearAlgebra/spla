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
#include <limits>

#include "spla.hpp"

TEST(m_assign_bslct, assign_pair) {
    int32_t n = 3;
    auto    S = spla::Matrix::make(n, n, spla::PAIR);

    S->set_pair(0, 1, spla::T_PAIR(7.0f, 7));
    S->set_pair(1, 0, spla::T_PAIR(3.0f, 1));
    S->set_pair(1, 2, spla::T_PAIR(3.0f, 6));
    S->set_pair(2, 1, spla::T_PAIR(2.0f, 5));
    S->set_pair(2, 2, spla::T_PAIR(8.0f, 9));

    auto mask = spla::Vector::make(n, spla::PAIR);
    mask->set_pair(0, spla::T_PAIR(3.0f, 3));
    mask->set_pair(1, spla::T_PAIR(32.0f, 3));
    mask->set_pair(2, spla::T_PAIR(6.0f, 7));

    auto assign_scalar = spla::Scalar::make(spla::PAIR);
    assign_scalar->set_pair(spla::T_PAIR());
    spla::exec_m_assign_bslct_masked(S, mask, assign_scalar, spla::SECOND_PAIR, spla::EQVERTEX_PAIR);

    spla::T_PAIR expected[] = {spla::T_PAIR(), spla::T_PAIR(), spla::T_PAIR(),
                               spla::T_PAIR(), spla::T_PAIR(), spla::T_PAIR(3.0f, 6),
                               spla::T_PAIR(), spla::T_PAIR(2.0f, 5), spla::T_PAIR()};

    for (int32_t i = 0; i < n; i++) {
        for (int32_t j = 0; j < n; j++) {
            spla::T_PAIR p;
            S->get_pair(i, j, p);
            EXPECT_FLOAT_EQ(p.weight, expected[i * n + j].weight);
            EXPECT_EQ(p.vertex, expected[i * n + j].vertex);
        }
    }
}

TEST(m_assign_bslct, assign_int) {
    int32_t n = 5;
    auto    S = spla::Matrix::make(n, n, spla::INT);

    S->set_int(0, 2, spla::T_INT(1));
    S->set_int(0, 3, spla::T_INT(2));
    S->set_int(1, 1, spla::T_INT(3));
    S->set_int(2, 2, spla::T_INT(4));
    S->set_int(2, 4, spla::T_INT(5));
    S->set_int(3, 0, spla::T_INT(6));
    S->set_int(3, 3, spla::T_INT(7));
    S->set_int(4, 1, spla::T_INT(8));
    S->set_int(4, 4, spla::T_INT(9));

    auto mask = spla::Vector::make(n, spla::INT);
    mask->set_int(0, spla::T_INT(1));
    mask->set_int(1, spla::T_INT(2));
    mask->set_int(2, spla::T_INT(3));
    mask->set_int(3, spla::T_INT(4));
    mask->set_int(4, spla::T_INT(5));

    auto assign_scalar = spla::Scalar::make(spla::INT);
    assign_scalar->set_int(spla::T_INT(10));
    spla::exec_m_assign_bslct_masked(S, mask, assign_scalar, spla::SECOND_INT, spla::EQ_INT);
    spla::T_INT expected[] = {spla::T_INT(), spla::T_INT(), spla::T_INT(1), spla::T_INT(2), spla::T_INT(),
                              spla::T_INT(), spla::T_INT(10), spla::T_INT(), spla::T_INT(), spla::T_INT(),
                              spla::T_INT(), spla::T_INT(), spla::T_INT(10), spla::T_INT(), spla::T_INT(5),
                              spla::T_INT(6), spla::T_INT(), spla::T_INT(), spla::T_INT(10), spla::T_INT(),
                              spla::T_INT(), spla::T_INT(8), spla::T_INT(), spla::T_INT(), spla::T_INT(10)};

    for (int32_t i = 0; i < n; i++) {
        for (int32_t j = 0; j < n; j++) {
            spla::T_INT num;
            S->get_int(i, j, num);
            EXPECT_EQ(num, expected[i * n + j]);
        }
    }
}

TEST(m_assign_bslct, assign_int_no_match) {
    int32_t n = 3;
    auto    S = spla::Matrix::make(n, n, spla::INT);

    S->set_int(0, 1, spla::T_INT(1));
    S->set_int(1, 0, spla::T_INT(2));
    S->set_int(2, 2, spla::T_INT(3));

    auto mask = spla::Vector::make(n, spla::INT);
    mask->set_int(0, spla::T_INT(100));
    mask->set_int(1, spla::T_INT(200));
    mask->set_int(2, spla::T_INT(300));

    auto assign_scalar = spla::Scalar::make(spla::INT);
    assign_scalar->set_int(spla::T_INT(999));
    spla::exec_m_assign_bslct_masked(S, mask, assign_scalar, spla::SECOND_INT, spla::EQ_INT);

    spla::T_INT expected[] = {
            spla::T_INT(), spla::T_INT(1), spla::T_INT(),
            spla::T_INT(2), spla::T_INT(), spla::T_INT(),
            spla::T_INT(), spla::T_INT(), spla::T_INT(999)};

    for (int32_t i = 0; i < n; i++) {
        for (int32_t j = 0; j < n; j++) {
            spla::T_INT num;
            S->get_int(i, j, num);
            EXPECT_EQ(num, expected[i * n + j]);
        }
    }
}

TEST(m_assign_bslct, assign_int_empty_mask) {
    int32_t n = 3;
    auto    S = spla::Matrix::make(n, n, spla::INT);

    S->set_int(0, 1, spla::T_INT(5));
    S->set_int(1, 2, spla::T_INT(15));
    S->set_int(2, 0, spla::T_INT(25));

    auto mask = spla::Vector::make(n, spla::INT);

    auto assign_scalar = spla::Scalar::make(spla::INT);
    assign_scalar->set_int(spla::T_INT(999));

    spla::exec_m_assign_bslct_masked(S, mask, assign_scalar, spla::SECOND_INT, spla::EQ_INT);

    spla::T_INT expected[] = {
            spla::T_INT(), spla::T_INT(999), spla::T_INT(),
            spla::T_INT(), spla::T_INT(), spla::T_INT(999),
            spla::T_INT(999), spla::T_INT(), spla::T_INT()};

    for (int32_t i = 0; i < n; i++) {
        for (int32_t j = 0; j < n; j++) {
            spla::T_INT num;
            S->get_int(i, j, num);
            EXPECT_EQ(num, expected[i * n + j]);
        }
    }
}

TEST(m_assign_bslct, assign_int_sparse_mask) {
    int32_t n = 3;
    auto    S = spla::Matrix::make(n, n, spla::INT);

    S->set_int(0, 1, spla::T_INT(5));
    S->set_int(1, 2, spla::T_INT(15));
    S->set_int(2, 0, spla::T_INT(25));

    auto mask = spla::Vector::make(n, spla::INT);
    mask->set_int(0, spla::T_INT(3));
    mask->set_int(1, spla::T_INT(3));

    auto assign_scalar = spla::Scalar::make(spla::INT);
    assign_scalar->set_int(spla::T_INT(999));

    spla::exec_m_assign_bslct_masked(S, mask, assign_scalar, spla::SECOND_INT, spla::EQ_INT);

    spla::T_INT expected[] = {
            spla::T_INT(), spla::T_INT(999), spla::T_INT(),
            spla::T_INT(), spla::T_INT(), spla::T_INT(15),
            spla::T_INT(25), spla::T_INT(), spla::T_INT()};

    for (int32_t i = 0; i < n; i++) {
        for (int32_t j = 0; j < n; j++) {
            spla::T_INT num;
            S->get_int(i, j, num);
            EXPECT_EQ(num, expected[i * n + j]);
        }
    }
}

SPLA_GTEST_MAIN
