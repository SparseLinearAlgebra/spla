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

TEST(v_gather, gather_pair) {
    int32_t n      = 5;
    auto    source = spla::Vector::make(n, spla::PAIR);
    source->set_pair(0, spla::T_PAIR(10.0f, 0));
    source->set_pair(1, spla::T_PAIR(20.0f, 1));
    source->set_pair(2, spla::T_PAIR(30.0f, 2));
    source->set_pair(3, spla::T_PAIR(40.0f, 3));
    source->set_pair(4, spla::T_PAIR(50.0f, 4));

    auto indices = spla::Vector::make(n, spla::UINT);
    indices->set_uint(0, 2);
    indices->set_uint(1, 0);
    indices->set_uint(2, 4);
    indices->set_uint(3, 1);
    indices->set_uint(4, 3);

    auto r = spla::Vector::make(n, spla::PAIR);

    spla::exec_v_gather(r, source, indices);

    spla::T_PAIR expected[] = {spla::T_PAIR(30.0f, 2), spla::T_PAIR(10.0f, 0),
                               spla::T_PAIR(50.0f, 4), spla::T_PAIR(20.0f, 1),
                               spla::T_PAIR(40.0f, 3)};

    for (int32_t i = 0; i < n; i++) {
        spla::T_PAIR p;
        r->get_pair(i, p);
        EXPECT_FLOAT_EQ(p.weight, expected[i].weight);
        EXPECT_EQ(p.vertex, expected[i].vertex);
    }
}

TEST(v_gather, gather_int) {
    int32_t n      = 4;
    auto    source = spla::Vector::make(n, spla::INT);
    source->set_int(0, 100);
    source->set_int(1, 200);
    source->set_int(2, 300);
    source->set_int(3, 400);

    auto indices = spla::Vector::make(n, spla::UINT);
    indices->set_uint(0, 3);
    indices->set_uint(1, 1);
    indices->set_uint(2, 0);
    indices->set_uint(3, 2);

    auto r = spla::Vector::make(n, spla::INT);

    spla::exec_v_gather(r, source, indices);

    spla::T_INT expected[] = {400, 200, 100, 300};

    for (int32_t i = 0; i < n; i++) {
        spla::T_INT v;
        r->get_int(i, v);
        EXPECT_EQ(v, expected[i]);
    }
}

TEST(v_gather, gather_self_reference) {
    int32_t n            = 4;
    auto    replacements = spla::Vector::make(n, spla::UINT);
    replacements->set_uint(0, 0);
    replacements->set_uint(1, 2);
    replacements->set_uint(2, 3);
    replacements->set_uint(3, 3);

    auto temp = spla::Vector::make(n, spla::UINT);

    spla::exec_v_gather(temp, replacements, replacements);

    spla::T_UINT expected[] = {0, 3, 3, 3};

    for (int32_t i = 0; i < n; i++) {
        spla::T_UINT v;
        temp->get_uint(i, v);
        EXPECT_EQ(v, expected[i]);
    }
}

TEST(v_gather, identity_indices) {
    int32_t n      = 4;
    auto    source = spla::Vector::make(n, spla::INT);
    source->set_int(0, 10);
    source->set_int(1, 20);
    source->set_int(2, 30);
    source->set_int(3, 40);

    auto indices = spla::Vector::make(n, spla::UINT);
    indices->set_uint(0, 0);
    indices->set_uint(1, 1);
    indices->set_uint(2, 2);
    indices->set_uint(3, 3);

    auto r = spla::Vector::make(n, spla::INT);

    spla::exec_v_gather(r, source, indices);

    spla::T_INT expected[] = {10, 20, 30, 40};

    for (int32_t i = 0; i < n; i++) {
        spla::T_INT v;
        r->get_int(i, v);
        EXPECT_EQ(v, expected[i]);
    }
}

TEST(v_gather, same_index_repeated) {
    int32_t n      = 4;
    auto    source = spla::Vector::make(n, spla::INT);
    source->set_int(0, 100);
    source->set_int(1, 200);
    source->set_int(2, 300);
    source->set_int(3, 400);

    auto indices = spla::Vector::make(n, spla::UINT);
    indices->set_uint(0, 2);
    indices->set_uint(1, 2);
    indices->set_uint(2, 2);
    indices->set_uint(3, 2);

    auto r = spla::Vector::make(n, spla::INT);

    spla::exec_v_gather(r, source, indices);

    spla::T_INT expected[] = {300, 300, 300, 300};

    for (int32_t i = 0; i < n; i++) {
        spla::T_INT v;
        r->get_int(i, v);
        EXPECT_EQ(v, expected[i]);
    }
}

TEST(v_gather, single_element) {
    int32_t n      = 1;
    auto    source = spla::Vector::make(n, spla::INT);
    source->set_int(0, 42);

    auto indices = spla::Vector::make(n, spla::UINT);
    indices->set_uint(0, 0);

    auto r = spla::Vector::make(n, spla::INT);

    spla::exec_v_gather(r, source, indices);

    spla::T_INT v;
    r->get_int(0, v);
    EXPECT_EQ(v, 42);
}

TEST(v_gather, large_vector) {
    int32_t n       = 1000;
    auto    source  = spla::Vector::make(n, spla::INT);
    auto    indices = spla::Vector::make(n, spla::UINT);

    for (int32_t i = 0; i < n; i++) {
        source->set_int(i, i * 2);
        indices->set_uint(i, n - 1 - i);
    }

    auto r = spla::Vector::make(n, spla::INT);

    spla::exec_v_gather(r, source, indices);

    for (int32_t i = 0; i < n; i++) {
        spla::T_INT v;
        r->get_int(i, v);
        EXPECT_EQ(v, (n - 1 - i) * 2);
    }
}

TEST(v_gather, chain_of_replacements) {
    int32_t n            = 6;
    auto    replacements = spla::Vector::make(n, spla::UINT);
    replacements->set_uint(0, 1);
    replacements->set_uint(1, 2);
    replacements->set_uint(2, 3);
    replacements->set_uint(3, 4);
    replacements->set_uint(4, 5);
    replacements->set_uint(5, 5);

    auto temp = spla::Vector::make(n, spla::UINT);

    spla::exec_v_gather(temp, replacements, replacements);

    spla::T_UINT expected[] = {2, 3, 4, 5, 5, 5};

    for (int32_t i = 0; i < n; i++) {
        spla::T_UINT v;
        temp->get_uint(i, v);
        EXPECT_EQ(v, expected[i]);
    }
}

SPLA_GTEST_MAIN