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

template<typename Type>
void testCommon(spla::Library &library, std::size_t M, std::size_t N, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT, std::size_t seed = 0) {
    utils::Matrix a = utils::Matrix<Type>::Generate(M, N, nvals, seed).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Matrix::Make(M, N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spTria = spExpr->MakeTril(spA, spA);
    spExpr->Dependency(spWriteA, spTria);
    spExpr->SubmitWait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix result = a.Tril();
    EXPECT_TRUE(result.Equals(spA));
}

void testNoValues(spla::Library &library, std::size_t M, std::size_t N, std::size_t nvals, std::size_t seed = 0) {
    using Type = char;

    utils::Matrix a = utils::Matrix<Type>::Generate(M, N, nvals, seed).SortReduceDuplicates();

    auto spT = spla::Types::Void(library);
    auto spA = spla::Matrix::Make(M, N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetDataIndices(library), spDesc);
    auto spTria = spExpr->MakeTril(spA, spA);
    spExpr->Dependency(spWriteA, spTria);
    spExpr->SubmitWait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix result = a.Tril();
    EXPECT_TRUE(result.EqualsStructure(spA));
}

void test(std::size_t M, std::size_t N, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using T = float;
        auto spT = spla::Types::Float32(library);

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<T>(library, M, N, nvals, spT, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using T = std::int32_t;
        auto spT = spla::Types::Int32(library);

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<T>(library, M, N, nvals, spT, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, N, nvals, i);
        }
    });
}

TEST(Tril, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    std::size_t N = 120;
    test(M, N, M, M, 10, blocksSizes);
}

TEST(Tril, Medium) {
    std::vector<std::size_t> blocksSizes{1000, 10000};
    std::size_t M = 900;
    std::size_t N = 1200;
    test(M, N, M, M, 10, blocksSizes);
}

TEST(Tril, Large) {
    std::vector<std::size_t> blocksSizes{10000, 100000};
    std::size_t M = 8800;
    std::size_t N = 12700;
    test(M, N, M, M, 5, blocksSizes);
}

TEST(Tril, MegaLarge) {
    std::vector<std::size_t> blocksSizes{1000000};
    std::size_t M = 900100;
    std::size_t N = 970500;
    test(M, N, M, M, 2, blocksSizes);
}

SPLA_GTEST_MAIN