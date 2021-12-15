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

template<typename Type, typename MultOp, typename AddOp>
void testCommon(spla::Library &library,
                std::size_t M, std::size_t N, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spMult,
                const spla::RefPtr<spla::FunctionBinary> &spAdd,
                MultOp multOp, AddOp addOp,
                std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<Type>::Generate(M, N, nvals, seed + 1).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());
    b.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Matrix::Make(M, N, spT, library);
    auto spW = spla::Vector::Make(N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spVxM = spExpr->MakeVxM(spW, nullptr, spMult, spAdd, spA, spB);
    spExpr->Dependency(spWriteA, spVxM);
    spExpr->Dependency(spWriteB, spVxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = utils::VxM(a, b, multOp, addOp);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename MultOp, typename AddOp>
void testMasked(spla::Library &library,
                std::size_t M, std::size_t N, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spMult,
                const spla::RefPtr<spla::FunctionBinary> &spAdd,
                MultOp multOp, AddOp addOp, bool complement,
                std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<Type>::Generate(M, N, nvals, seed + 1).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned char>::Generate(N, nvals, seed + 2).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());
    b.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Matrix::Make(M, N, spT, library);
    auto spW = spla::Vector::Make(N, spT, library);
    auto spMask = spla::Vector::Make(N, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    // Use complementary mask
    auto spOpDesc = spla::Descriptor::Make(library);
    if (complement)
        spOpDesc->SetParam(spla::Descriptor::Param::MaskComplement);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spVxM = spExpr->MakeVxM(spW, spMask, spMult, spAdd, spA, spB, spOpDesc);
    spExpr->Dependency(spWriteA, spVxM);
    spExpr->Dependency(spWriteB, spVxM);
    spExpr->Dependency(spWriteMask, spVxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = utils::VxM(mask, complement, a, b, multOp, addOp);
    ASSERT_TRUE(c.Equals(spW));
}

void testNoValues(spla::Library &library, std::size_t M, std::size_t N, std::size_t nvals, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<unsigned char>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Matrix b = utils::Matrix<unsigned char>::Generate(M, N, nvals, seed + 1).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned char>::Generate(N, nvals, seed + 2).SortReduceDuplicates();

    auto spT = spla::Types::Void(library);
    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Matrix::Make(M, N, spT, library);
    auto spW = spla::Vector::Make(N, spT, library);
    auto spMask = spla::Vector::Make(N, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetDataIndices(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetDataIndices(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spVxM = spExpr->MakeVxM(spW, spMask, nullptr, nullptr, spA, spB);
    spExpr->Dependency(spWriteA, spVxM);
    spExpr->Dependency(spWriteB, spVxM);
    spExpr->Dependency(spWriteMask, spVxM);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    auto dummy = [](unsigned char a, unsigned char b) { return 0; };
    utils::Vector<unsigned char> c = utils::VxM(mask, false, a, b, dummy, dummy);
    ASSERT_TRUE(c.EqualsStructure(spW));
}

void test(std::size_t M, std::size_t N, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using T = float;
        auto spT = spla::Types::Float32(library);
        auto spMult = spla::Functions::MultFloat32(library);
        auto spAdd = spla::Functions::PlusFloat32(library);
        auto mult = [](T a, T b) { return a * b; };
        auto add = [](T a, T b) { return a + b; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, false, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, true, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using T = std::int32_t;
        auto spT = spla::Types::Int32(library);
        auto spMult = spla::Functions::MultInt32(library);
        auto spAdd = spla::Functions::PlusInt32(library);
        auto mult = [](T a, T b) { return a * b; };
        auto add = [](T a, T b) { return a + b; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, false, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<T>(library, M, N, nvals, spT, spMult, spAdd, mult, add, true, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, N, nvals, i);
        }
    });
}

TEST(VxM, Small) {
    std::vector<std::size_t> blockSizes = {100, 1000};
    std::size_t M = 120, N = 80;
    test(M, N, M, M, 10, blockSizes);
}

TEST(VxM, Medium) {
    std::vector<std::size_t> blockSizes = {1000, 10000};
    std::size_t M = 1220, N = 880;
    test(M, N, M, M, 10, blockSizes);
}

TEST(VxM, Large) {
    std::vector<std::size_t> blockSizes = {10000, 100000};
    std::size_t M = 12400, N = 8080;
    test(M, N, M, M, 5, blockSizes);
}

SPLA_GTEST_MAIN