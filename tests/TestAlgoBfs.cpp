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

#include <Testing.hpp>

void testCase(spla::Library &library, std::size_t M, std::size_t nvals, std::size_t seed = 0) {
    auto rnd = utils::UniformIntGenerator<spla::Index>(seed, 0, M - 1);
    auto sp_Int32 = spla::Types::Int32(library);
    auto sp_Void = spla::Types::Void(library);
    auto sp_s = rnd();

    utils::Matrix A = utils::Matrix<char>::Generate(M, M, nvals).SortReduceDuplicates();

    auto sp_v = spla::Vector::Make(M, sp_Int32, library);
    auto sp_A = spla::Matrix::Make(M, M, sp_Void, library);

    auto sp_setup = spla::Expression::Make(library);
    sp_setup->MakeDataWrite(sp_A, A.GetDataIndices(library));
    sp_setup->SubmitWait();
    ASSERT_EQ(sp_setup->GetState(), spla::Expression::State::Evaluated);

    spla::Bfs(sp_v, sp_A, sp_s);

    auto host_A = A.ToHostMatrix();
    auto host_v = spla::RefPtr<spla::HostVector>();

    spla::Bfs(host_v, host_A, sp_s);

    auto result = utils::Vector<std::int32_t>::FromHostVector(host_v);
    ASSERT_TRUE(result.Equals(sp_v));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase(library, M, nvals, i);
        }
    });
}

TEST(BFS, Small) {
    std::vector<std::size_t> blockSizes = {100, 1000};
    std::size_t M = 120;
    test(M, M, M, 10, blockSizes);
}

TEST(BFS, Medium) {
    std::vector<std::size_t> blockSizes = {1000, 10000};
    std::size_t M = 1220;
    test(M, M, M, 10, blockSizes);
}

TEST(BFS, Large) {
    std::vector<std::size_t> blockSizes = {10000, 100000};
    std::size_t M = 12400;
    test(M, M, M, 5, blockSizes);
}

SPLA_GTEST_MAIN