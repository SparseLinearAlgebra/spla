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
    auto sp_s = rnd();

    utils::Matrix A = utils::Matrix<std::int32_t>::Generate(M, M, nvals).SortReduceDuplicates();
    A.Fill(utils::UniformIntGenerator<std::int32_t>());

    auto sp_v = spla::Vector::Make(M, sp_Int32, library);
    auto sp_A = spla::Matrix::Make(M, M, sp_Int32, library);

    auto sp_setup = spla::Expression::Make(library);
    sp_setup->MakeDataWrite(sp_A, A.GetData(library));
    sp_setup->SubmitWait();
    ASSERT_EQ(sp_setup->GetState(), spla::Expression::State::Evaluated);

    SPLA_TIME_BEGIN(bfs_spla);
    spla::Bfs(sp_v, sp_A, sp_s);
    SPLA_TIME_END(bfs_spla, "spla");

    auto host_A = A.ToHostMatrix();
    auto host_v = spla::RefPtr<spla::HostVector>();

    SPLA_TIME_BEGIN(bfs_cpu);
    spla::Bfs(host_v, host_A, sp_s);
    SPLA_TIME_END(bfs_cpu, "cpu ");

    auto result = utils::Vector<std::int32_t>::FromHostVector(host_v);
    ASSERT_TRUE(result.Equals(sp_v));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::cout << "iter [" << i << "]\n";
            for (std::size_t k = 0; k < 4; k++) {
                std::size_t nvals = base + i * step;
                testCase(library, M, nvals, i);
            }
        }
    });
}

TEST(BFS, Average) {
    utils::testBlocks({1000000}, [=](spla::Library &library) {
        std::vector<std::size_t> sizes = {120, 1222, 13405};
        std::vector<std::size_t> iters = {5, 2, 2};
        std::vector<std::size_t> steps = {100, 1000, 10000};
        std::vector<std::size_t> bases = {100, 1000, 10000};
        for (std::size_t run = 0; run < sizes.size(); run++) {
            auto M = sizes[run];
            auto base = bases[run];
            auto iter = iters[run];
            auto step = steps[run];
            for (std::size_t i = 0; i < iter; i++) {
                std::cout << "run [" << run << "] iter [" << i << "]\n";
                for (std::size_t k = 0; k < 4; k++) {
                    std::size_t nvals = base + i * step;
                    testCase(library, M, nvals, i);
                }
            }
        }
    });
}

TEST(BFS, Small) {
    std::vector<std::size_t> blockSizes = {1000};
    std::size_t M = 120;
    test(M, M, M, 5, blockSizes);
}

TEST(BFS, Medium) {
    std::vector<std::size_t> blockSizes = {10000};
    std::size_t M = 1220;
    test(M, M, M, 2, blockSizes);
}

TEST(BFS, Large) {
    std::vector<std::size_t> blockSizes = {100000};
    std::size_t M = 12400;
    test(M, M, M, 2, blockSizes);
}

TEST(BFS, MegaLarge) {
    std::vector<std::size_t> blockSizes = {1000000};
    std::size_t M = 990990;
    test(M, 10 * M, M, 1, blockSizes);
}

TEST(BFS, UltraLarge) {
    std::vector<std::size_t> blockSizes = {10000000};
    std::size_t M = 4500000;
    test(M, 10 * M, 10 * M, 2, blockSizes);
}

SPLA_GTEST_MAIN