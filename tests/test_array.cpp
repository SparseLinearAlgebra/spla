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

#include <spla.hpp>

TEST(array, create) {
    auto array = spla::Array::make(10, spla::INT);

    EXPECT_EQ(array->get_n_values(), 10);
}

TEST(array, resize) {
    auto array = spla::Array::make(10, spla::INT);

    EXPECT_EQ(array->get_n_values(), 10);
    EXPECT_EQ(array->resize(100), spla::Status::Ok);
    EXPECT_EQ(array->get_n_values(), 100);
}

TEST(array, get_set) {
    const auto        N         = 10;
    const spla::T_INT values[N] = {0, 1, 2, -1, 40, -1, 0, 0, 1, -100};

    auto array = spla::Array::make(N, spla::INT);

    for (int i = 0; i < N; i++) {
        array->set_int(i, values[i]);
    }

    for (int i = 0; i < N; i++) {
        int v;
        array->get_int(i, v);

        EXPECT_EQ(values[i], v);
    }
}

TEST(array, vector) {
    const auto         N         = 10;
    const spla::T_UINT keys[N]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const spla::T_INT  values[N] = {0, 1, 2, -1, 40, -1, 0, 0, 1, -100};

    auto k = spla::Array::make(N, spla::UINT);
    auto v = spla::Array::make(N, spla::INT);

    for (int i = 0; i < N; i++) {
        k->set_uint(i, keys[i]);
    }
    for (int i = 0; i < N; i++) {
        v->set_int(i, values[i]);
    }

    auto x = spla::Vector::make(N, spla::INT);

    EXPECT_EQ(x->build(k, v), spla::Status::Ok);

    auto r_k = spla::Array::make(0, spla::UINT);
    auto r_v = spla::Array::make(0, spla::INT);

    EXPECT_EQ(x->read(r_k, r_v), spla::Status::Ok);

    std::unordered_map<spla::T_UINT, spla::T_INT> pairs;

    for (int i = 0; i < N; i++) {
        spla::uint key;
        r_k->get_uint(i, key);

        int value;
        r_v->get_int(i, value);

        pairs[key] = value;
    }

    for (int i = 0; i < N; i++) {
        EXPECT_TRUE(pairs.find(keys[i]) != pairs.end());

        if (pairs.find(keys[i]) != pairs.end()) {
            EXPECT_EQ(pairs[keys[i]], values[i]);
        }
    }
}

TEST(array, matrix) {
    const auto         D         = 10;
    const auto         N         = 10;
    const spla::T_UINT keys1[N]  = {0, 1, 1, 3, 4, 5, 6, 6, 6, 9};
    const spla::T_UINT keys2[N]  = {0, 1, 4, 3, 4, 5, 1, 4, 7, 9};
    const spla::T_INT  values[N] = {0, 1, 2, -1, 40, -1, 0, 0, 1, -100};

    auto k1 = spla::Array::make(N, spla::UINT);
    auto k2 = spla::Array::make(N, spla::UINT);
    auto v  = spla::Array::make(N, spla::INT);

    for (int i = 0; i < N; i++) {
        k1->set_uint(i, keys1[i]);
    }
    for (int i = 0; i < N; i++) {
        k2->set_uint(i, keys2[i]);
    }
    for (int i = 0; i < N; i++) {
        v->set_int(i, values[i]);
    }

    auto M = spla::Matrix::make(D, D, spla::INT);

    EXPECT_EQ(M->build(k1, k2, v), spla::Status::Ok);

    auto r_k1 = spla::Array::make(0, spla::UINT);
    auto r_k2 = spla::Array::make(0, spla::UINT);
    auto r_v  = spla::Array::make(0, spla::INT);

    EXPECT_EQ(M->read(r_k1, r_k2, r_v), spla::Status::Ok);

    std::unordered_map<std::pair<spla::T_UINT, spla::T_UINT>, spla::T_INT, pair_hash> pairs;

    for (int i = 0; i < N; i++) {
        spla::uint key1;
        r_k1->get_uint(i, key1);

        spla::uint key2;
        r_k2->get_uint(i, key2);

        int value;
        r_v->get_int(i, value);

        pairs[{key1, key2}] = value;
    }

    for (int i = 0; i < N; i++) {
        EXPECT_TRUE(pairs.find({keys1[i], keys2[i]}) != pairs.end());

        if (pairs.find({keys1[i], keys2[i]}) != pairs.end()) {
            auto pv = pairs[{keys1[i], keys2[i]}];
            EXPECT_EQ(pv, values[i]);
        }
    }
}

SPLA_GTEST_MAIN