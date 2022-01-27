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
void testCase(spla::Library &library, std::size_t M, std::size_t nvals,
              const spla::RefPtr<spla::Type> &spType, std::size_t seed = 0) {
    utils::Vector v = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    v.Fill(utils::UniformGenerator<Type>());

    auto spV = spla::Vector::Make(M, spType, library);
    auto spI = spla::Scalar::Make(spType, library);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteV = spExpr->MakeDataWrite(spV, v.GetData(library));
    auto spToDense = spExpr->MakeToDense(spV, spV);
    auto spToDense2 = spExpr->MakeToDense(spV, spV);
    spExpr->Dependency(spWriteV, spToDense);
    spExpr->Dependency(spToDense, spToDense2);
    spExpr->SubmitWait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    EXPECT_TRUE(v.Equals(spV));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::int32_t;
        auto spT = spla::Types::Int32(library);

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase<Type>(library, M, nvals, spT, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase<Type>(library, M, nvals, spT, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = float;
        auto spT = spla::Types::Float32(library);

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase<Type>(library, M, nvals, spT, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCase<Type>(library, M, nvals, spT, i);
        }
    });
}

TEST(VectorToDense, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    test(M, M / 2, M / 20, 10, blocksSizes);
}

TEST(VectorToDense, Medium) {
    std::vector<std::size_t> blocksSizes{100, 1000, 10000};
    std::size_t M = 2140;
    test(M, M / 2, M / 20, 10, blocksSizes);
}

TEST(VectorToDense, Large) {
    std::vector<std::size_t> blocksSizes{1000, 10000, 100000};
    std::size_t M = 10300;
    test(M, M / 4, M / 20, 10, blocksSizes);
}

TEST(VectorToDense, MegaLarge) {
    std::vector<std::size_t> blocksSizes{1000000};
    std::size_t M = 990100;
    test(M, M / 10, M / 10, 5, blocksSizes);
}

SPLA_GTEST_MAIN