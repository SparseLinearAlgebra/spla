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

inline void test(std::size_t N, std::size_t nvals, std::size_t runs, std::size_t seed = 0) {
    auto t_rnd = testing::UniformGenerator<spla::Index>(seed, 0, N - 1);
    auto t_A = testing::Matrix<int>::generate(N, N, nvals, seed, int{1});
    auto t_source = t_rnd();

    spla::Matrix<int> A(N, N);
    spla::Vector<int> v(N);
    spla::Vector<int> v_naive(N);

    spla::Expression create_graph;
    create_graph.build(A, spla::NullOp(), t_A.rows, t_A.cols, t_A.values);
    create_graph.submit().wait();

    for (std::size_t run = 0; run < runs; run++) {
        spla::bfs(v, A, t_source);
        spla::bfs_naive(v_naive, A, t_source);

        EXPECT_TRUE(testing::equals(v, v_naive));
    }
}

TEST(Bfs, Small) {
    std::size_t N = 120;

    for (std::size_t i = 0; i < 10; i++) {
        test(N, N * (i + 1), 10, i);
    }
}

TEST(Bfs, Medium) {
    std::size_t N = 1220;

    for (std::size_t i = 0; i < 10; i++) {
        test(N, N * (i + 1), 10, i);
    }
}

TEST(Bfs, Large) {
    std::size_t N = 12420;

    for (std::size_t i = 0; i < 10; i++) {
        test(N, N * (i + 1), 10, i);
    }
}

SPLA_GTEST_MAIN