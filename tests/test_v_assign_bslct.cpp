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

TEST(v_assign_bslct, assign_pair) {
    int32_t n = 3;

    auto vector = spla::Vector::make(n, spla::PAIR);
    vector->set_pair(0, spla::T_PAIR(7.0f, 1));
    vector->set_pair(1, spla::T_PAIR(4.0f, 4));
    vector->set_pair(2, spla::T_PAIR(7.0f, 0));

    auto mask = spla::Vector::make(n, spla::PAIR);
    mask->set_pair(0, spla::T_PAIR(5.0f, 4));
    mask->set_pair(1, spla::T_PAIR(4.0f, 1));
    mask->set_pair(2, spla::T_PAIR(3.0f, 5));

    auto         assign_scalar = spla::Scalar::make(spla::PAIR);
    spla::T_PAIR assign_val;
    assign_scalar->set_pair(assign_val);
    auto mask_scalar = spla::Scalar::make(spla::PAIR);
    mask_scalar->set_pair(spla::T_PAIR(8.0f, 1));

    spla::exec_v_assign_bslct_masked(vector, mask, mask_scalar, assign_scalar, spla::SECOND_PAIR, spla::EQVERTEX_PAIR);

    spla::T_PAIR expected[] = {spla::T_PAIR(7.0f, 1), spla::T_PAIR(),
                               spla::T_PAIR(7.0f, 0)};
    for (int32_t i = 0; i < n; i++) {
        spla::T_PAIR p;
        vector->get_pair(i, p);
        EXPECT_FLOAT_EQ(p.weight, expected[i].weight);
        EXPECT_EQ(p.vertex, expected[i].vertex);
    }
}

TEST(v_assign_bslct, assign_int) {
    int32_t n = 4;

    auto vector = spla::Vector::make(n, spla::INT);
    vector->set_int(0, spla::T_INT(1));
    vector->set_int(1, spla::T_INT(4));
    vector->set_int(2, spla::T_INT(5));
    vector->set_int(3, spla::T_INT(9));

    auto mask = spla::Vector::make(n, spla::INT);
    mask->set_int(0, spla::T_INT(3));
    mask->set_int(1, spla::T_INT(6));
    mask->set_int(2, spla::T_INT(3));
    mask->set_int(3, spla::T_INT(2));

    auto assign_scalar = spla::Scalar::make(spla::INT);
    assign_scalar->set_int(spla::T_INT(100));
    auto mask_scalar = spla::Scalar::make(spla::INT);
    mask_scalar->set_int(spla::T_INT(3));

    spla::exec_v_assign_bslct_masked(vector, mask, mask_scalar, assign_scalar, spla::SECOND_INT, spla::EQ_INT);

    spla::T_INT expected[] = {spla::T_INT(100), spla::T_INT(4),
                              spla::T_INT(100), spla::T_INT(9)};
    for (int32_t i = 0; i < n; i++) {
        spla::T_INT num;
        vector->get_int(i, num);
        EXPECT_EQ(num, expected[i]);
    }
}

SPLA_GTEST_MAIN
