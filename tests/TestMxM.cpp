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

template<typename Type, typename MultOp, typename AddOp, typename Random>
void testCommon(spla::Library &library,
                std::size_t M, std::size_t K, std::size_t N, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spMult,
                const spla::RefPtr<spla::FunctionBinary> &spAdd,
                MultOp multOp, AddOp addOp,
                std::size_t indexGenerationSeed,
                Random valueGenerator) {
    utils::Matrix a = utils::Matrix<Type>::Generate(M, K, nvals, indexGenerationSeed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<Type>::Generate(K, N, nvals, indexGenerationSeed + 1).SortReduceDuplicates();

    a.Fill(std::ref(valueGenerator));
    b.Fill(std::ref(valueGenerator));

    auto spA = spla::Matrix::Make(M, K, spT, library);
    auto spB = spla::Matrix::Make(K, N, spT, library);
    auto spW = spla::Matrix::Make(M, N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spMxM = spExpr->MakeMxM(spW, nullptr, spMult, spAdd, spA, spB);
    spExpr->Dependency(spWriteA, spMxM);
    spExpr->Dependency(spWriteB, spMxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix<Type> c = a.template MxM<Type>(b, multOp, addOp);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename MultOp, typename AddOp, typename Random>
void testMasked(spla::Library &library,
                std::size_t M, std::size_t K, std::size_t N, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spMult,
                const spla::RefPtr<spla::FunctionBinary> &spAdd,
                MultOp multOp, AddOp addOp,
                std::size_t indexGenerationSeed,
                Random valueGenerator,
                bool maskComplement) {
    utils::Matrix a = utils::Matrix<Type>::Generate(M, K, nvals, indexGenerationSeed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<Type>::Generate(K, N, nvals, indexGenerationSeed + 2).SortReduceDuplicates();
    utils::Matrix mask = utils::Matrix<unsigned char>::Generate(M, N, nvals, indexGenerationSeed + 3).SortReduceDuplicates();

    a.Fill(std::ref(valueGenerator));
    b.Fill(std::ref(valueGenerator));

    auto spA = spla::Matrix::Make(M, K, spT, library);
    auto spB = spla::Matrix::Make(K, N, spT, library);
    auto spW = spla::Matrix::Make(M, N, spT, library);
    auto spMask = spla::Matrix::Make(M, N, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    // Use complementary mask if required
    auto spOpDesc = spla::Descriptor::Make(library);
    if (maskComplement) {
        spOpDesc->SetParam(spla::Descriptor::Param::MaskComplement);
    }

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spMxM = spExpr->MakeMxM(spW, spMask, spMult, spAdd, spA, spB, spOpDesc);
    spExpr->Dependency(spWriteA, spMxM);
    spExpr->Dependency(spWriteB, spMxM);
    spExpr->Dependency(spWriteMask, spMxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix<Type> c = a.template MxM<Type>(mask, maskComplement, b, multOp, addOp);
    ASSERT_TRUE(c.Equals(spW));
}

void testNoValues(spla::Library &library, std::size_t M, std::size_t K, std::size_t N, std::size_t nvals, std::size_t seed = 0) {
    utils::Matrix a = utils::Matrix<unsigned char>::Generate(M, K, nvals, seed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<unsigned char>::Generate(K, N, nvals, seed + 1).SortReduceDuplicates();
    utils::Matrix mask = utils::Matrix<unsigned char>::Generate(M, N, nvals, seed + 2).SortReduceDuplicates();

    auto spT = spla::Types::Void(library);
    auto spA = spla::Matrix::Make(M, K, spT, library);
    auto spB = spla::Matrix::Make(K, N, spT, library);
    auto spW = spla::Matrix::Make(M, N, spT, library);
    auto spMask = spla::Matrix::Make(M, N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetDataIndices(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetDataIndices(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spMxM = spExpr->MakeMxM(spW, spMask, nullptr, nullptr, spA, spB);
    spExpr->Dependency(spWriteA, spMxM);
    spExpr->Dependency(spWriteB, spMxM);
    spExpr->Dependency(spWriteMask, spMxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    auto dummy = [](unsigned char a, unsigned char b) { return 0; };

    utils::Matrix<unsigned char> c = a.template MxM<unsigned char>(mask, false, b, dummy, dummy);
    ASSERT_TRUE(c.EqualsStructure(spW));
}

void test(std::size_t M, std::size_t K, std::size_t N, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes, utils::UniformIntGenerator<std::int32_t> intGen = utils::UniformIntGenerator<std::int32_t>()) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        auto spT = spla::Types::Float32(library);
        auto spMult = spla::Functions::MultFloat32(library);
        auto spAdd = spla::Functions::PlusFloat32(library);
        auto mult = [](float a, float b) { return a * b; };
        auto add = [](float a, float b) { return a + b; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<float>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, utils::UniformRealGenerator<float>(i));
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<float>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, utils::UniformRealGenerator<float>(i), false);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<float>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, utils::UniformRealGenerator<float>(i), true);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, K, N, nvals, i * i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        auto spT = spla::Types::Int32(library);
        auto spMult = spla::Functions::MultInt32(library);
        auto spAdd = spla::Functions::PlusInt32(library);
        auto mult = [](int a, int b) { return a * b; };
        auto add = [](int a, int b) { return a + b; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<std::int32_t>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, intGen);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<std::int32_t>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, intGen, false);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<std::int32_t>(library, M, K, N, nvals, spT, spMult, spAdd, mult, add, i, intGen, true);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, K, N, nvals, i * i);
        }
    });
}

TEST(MxM, Tiny) {
    spla::Library library;
    testMasked<std::int32_t>(library,
                             3, 3, 3, 5,
                             spla::Types::Int32(library),
                             spla::Functions::MultInt32(library), spla::Functions::PlusInt32(library),
                             std::multiplies<>(), std::plus<>(),
                             0,
                             utils::UniformIntGenerator<std::int32_t>(0, 1, 10),
                             false);
}

TEST(MxM, Small) {
    std::vector<std::size_t> blockSizes = {100, 1000};
    std::size_t M = 80, K = 140, N = 120;
    test(M, K, N, M, M, 10, blockSizes);
}

TEST(MxM, Medium) {
    std::vector<std::size_t> blockSizes = {1000, 10000};
    std::size_t M = 880, K = 1400, N = 1220;
    test(M, K, N, M, M, 10, blockSizes);
}

TEST(MxM, Large) {
    std::vector<std::size_t> blockSizes = {10000, 100000};
    std::size_t M = 8080, K = 14100, N = 12400;
    test(M, K, N, M, M, 5, blockSizes);
}

SPLA_GTEST_MAIN