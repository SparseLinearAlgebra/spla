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

#include <iostream>
#include <spla.hpp>

TEST(mxmT_masked, naive) {
    spla::uint M = 3, N = 4, K = 2;

    // R
    //
    // 23 0
    //  0 0
    // 83 0

    // mask
    //
    // 1 1
    // 0 0
    // 1 1

    // A
    //
    // 1 0 2 0
    // 0 3 0 4
    // 5 0 6 0

    // B
    //
    // 7  0 8  0
    // 0  9 0  10

    auto R    = spla::Matrix::make(M, K, spla::FLOAT);
    auto mask = spla::Matrix::make(M, K, spla::FLOAT);
    auto A    = spla::Matrix::make(M, N, spla::FLOAT);
    auto B    = spla::Matrix::make(K, N, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0);

    mask->set_float(0, 0, 1.0f);
    mask->set_float(0, 1, 1.0f);
    mask->set_float(2, 0, 1.0f);
    mask->set_float(2, 1, 1.0f);

    A->set_float(0, 0, 1.0f);
    A->set_float(0, 2, 2.0f);
    A->set_float(1, 1, 3.0f);
    A->set_float(1, 3, 4.0f);
    A->set_float(2, 0, 5.0f);
    A->set_float(2, 2, 6.0f);

    B->set_float(0, 0, 7.0f);
    B->set_float(0, 2, 8.0f);
    B->set_float(1, 1, 9.0f);
    B->set_float(1, 3, 10.0f);

    spla::exec_mxmT_masked(R, mask, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, spla::GTZERO_FLOAT, init);

    float v;

    R->get_float(0, 0, v);
    EXPECT_EQ(v, 23);

    R->get_float(0, 1, v);
    EXPECT_EQ(v, 0);

    R->get_float(1, 0, v);
    EXPECT_EQ(v, 0);

    R->get_float(1, 1, v);
    EXPECT_EQ(v, 0);

    R->get_float(2, 0, v);
    EXPECT_EQ(v, 83);

    R->get_float(2, 1, v);
    EXPECT_EQ(v, 0);
}

SPLA_GTEST_MAIN_WITH_FINALIZE_PLATFORM(1)