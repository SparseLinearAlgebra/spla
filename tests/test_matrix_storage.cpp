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

#include <test_utils.hpp>

#include <spla/spla.hpp>

TEST(Matrix, Setup) {
    spla::Matrix<float> m(100, 120);

    std::vector<spla::Index> rows = {2, 2, 5, 7, 9, 98, 0, 0, 0};
    std::vector<spla::Index> cols = {1, 1, 119, 3, 4, 1, 90, 80, 80};
    std::vector<float> values = {50, 50, 0, 4, -0.2, -0.1, 10, -0.05, -0.05};

    spla::Expression expression;
    expression.build(m, spla::binary_op::plus<float>(), rows, cols, values)
            ->precede(expression.read(m, [&](auto &r, auto &c, auto &v) { rows=std::move(r); cols=std::move(c); values=std::move(v); }));

    auto submission = expression.submit();
    submission.wait();
}

SPLA_GTEST_MAIN