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

TEST(array, build) {
    const auto  N         = 10;
    spla::T_INT values[N] = {0, 1, 2, -1, 40, -1, 0, 0, 1, -100};

    auto array = spla::Array::make(N, spla::INT);

    for (int i = 0; i < N; i++) {
        array->set_int(i, values[i]);
    }

    spla::ref_ptr<spla::MemView> view;

    array->read(view);

    EXPECT_EQ(view->get_size(), sizeof(spla::T_INT) * N);

    for (int i = 0; i < N; i++) {
        EXPECT_EQ(values[i], ((spla::T_INT*) view->get_buffer())[i]);
    }
}


TEST(array, read) {
    const auto  N         = 10;
    spla::T_INT values[N] = {0, 1, 2, -1, 40, -1, 0, 0, 1, -100};

    auto array = spla::Array::make(N, spla::INT);

    array->build(spla::MemView::make(values, N * sizeof(spla::T_INT), false));

    for (int i = 0; i < N; i++) {
        int v;
        array->get_int(i, v);

        EXPECT_EQ(values[i], v);
    }
}

SPLA_GTEST_MAIN