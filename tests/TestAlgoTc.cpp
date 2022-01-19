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
#include <spla-algo/SplaAlgoTc.hpp>

void testCase(spla::Library &library, std::size_t M, std::size_t nvals, bool dir) {
    utils::Matrix A = utils::Matrix<int>::Generate(M, M, nvals).SortReduceDuplicates();
    A.Fill([]() { return 1; });

    if (!dir) {
        A = A.EWiseAdd(A.Transpose(), [](int a, int b) { return a | b; });
    }

    auto sp_Int32 = spla::Types::Int32(library);

    auto sp_B = spla::Matrix::Make(M, M, sp_Int32, library);
    auto sp_A = spla::Matrix::Make(M, M, sp_Int32, library);

    auto sp_setup = spla::Expression::Make(library);
    sp_setup->MakeDataWrite(sp_A, A.GetData(library));
    sp_setup->SubmitWait();

    ASSERT_EQ(sp_setup->GetState(), spla::Expression::State::Evaluated);

    std::int32_t nTrinsSpla = 0;

    SPLA_TIME_BEGIN(tc_spla);
    spla::Tc(nTrinsSpla, sp_B, sp_A, dir);
    SPLA_TIME_END(tc_spla, "spla");

    auto host_A = A.ToHostMatrix();
    auto host_B = spla::RefPtr<spla::HostMatrix>();

    std::int32_t nTrinsCpu = 0;

    SPLA_TIME_BEGIN(sssp_cpu);
    spla::Tc(nTrinsCpu, host_B, host_A, dir);
    SPLA_TIME_END(sssp_cpu, "cpu ");

    auto result = utils::Matrix<int>::FromHostMatrix(host_B);
    EXPECT_TRUE(result.Equals(sp_B));
    EXPECT_EQ(nTrinsSpla, nTrinsCpu);
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase(library, M, nvals, true);
            testCase(library, M, nvals, false);
        }
    });
}

void testCpu(std::size_t n,
             std::vector<spla::Index> from,
             std::vector<spla::Index> to,
             bool dir,
             std::int32_t nTrins,
             const std::vector<std::int32_t> &trinsPerEdge) {
    std::int32_t actualNTrins = 0;

    const std::size_t nVals = from.size();
    assert(to.size() == nVals);

    std::vector<int> values(nVals, 1);
    std::vector<unsigned char> valuesData(nVals * sizeof(std::int32_t));

    std::memcpy(values.data(), valuesData.data(), nVals * sizeof(std::int32_t));

    auto trinsActual = spla::RefPtr<spla::HostMatrix>();
    auto A = spla::RefPtr<spla::HostMatrix>(new spla::HostMatrix(n,
                                                                 n,
                                                                 from,
                                                                 to,
                                                                 valuesData));

    spla::Tc(actualNTrins, trinsActual, A, dir);

    if (trinsActual.IsNull()) {
        EXPECT_EQ(nTrins, 0);
        return;
    }

    std::vector<int> trinsCounts(trinsActual->GetNnvals());
    const std::vector<spla::Index> &actRows = trinsActual->GetRowIndices();
    const std::vector<spla::Index> &actCols = trinsActual->GetColIndices();

    std::memcpy(trinsCounts.data(), trinsActual->GetValues().data(), trinsActual->GetNnvals() * sizeof(std::int32_t));

    EXPECT_EQ(nTrins, actualNTrins);

    for (std::size_t itAns = 0, itAct = 0; itAns < nVals; ++itAns) {
        if (trinsPerEdge[itAns] == 0) {
            continue;
        }
        EXPECT_EQ(trinsPerEdge[itAns], trinsCounts[itAct]);
        EXPECT_EQ(from[itAns], actRows[itAct]);
        EXPECT_EQ(to[itAns], actCols[itAct++]);
    }
}

TEST(TC, CpuShallowDirectedDir) {
    testCpu(3, {1, 1, 2}, {0, 2, 0}, true, 1, {0, 1, 0});
    testCpu(3, {0, 1, 2}, {1, 2, 0}, true, 0, {0, 0, 0});
    testCpu(5,
            {2, 2, 2, 3, 3, 3, 4},
            {0, 1, 3, 0, 1, 4, 1}, true, 3,
            {0, 0, 2, 0, 0, 1, 0});
}

TEST(TC, CpuShallowDirectedNonDir) {
    testCpu(3, {0, 0, 1, 1, 2, 2}, {1, 2, 0, 2, 0, 1}, false, 1, {1, 1, 1});
    testCpu(3, {0, 1, 1, 2}, {1, 0, 2, 1}, false, 0, {0, 0, 0});
    testCpu(6,
            {0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4, 5, 5},
            {1, 2, 0, 2, 3, 0, 1, 1, 4, 5, 3, 5, 3, 4},
            false,
            2,
            {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1});
}

TEST(TC, Small) {
    std::vector<std::size_t> blockSizes = {100, 1000};
    std::size_t M = 120;
    test(M, M, M, 10, blockSizes);
}

TEST(TC, Medium) {
    std::vector<std::size_t> blockSizes = {1000, 10000};
    std::size_t M = 1220;
    test(M, M, M, 10, blockSizes);
}

TEST(TC, Large) {
    std::vector<std::size_t> blockSizes = {10000, 100000};
    std::size_t M = 12400;
    test(M, M, M, 5, blockSizes);
}

SPLA_GTEST_MAIN