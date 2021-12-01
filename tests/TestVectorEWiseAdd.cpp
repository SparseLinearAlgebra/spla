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

template<typename Type, typename BinaryOp>
void testCommon(spla::Library &library, std::size_t M, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spOp,
                BinaryOp op, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector b = utils::Vector<Type>::Generate(M, nvals, seed + 1).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());
    b.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Vector::Make(M, spT, library);
    auto spW = spla::Vector::Make(M, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spEAddAB = spExpr->MakeEWiseAdd(spW, nullptr, spOp, spA, spB);
    spExpr->Dependency(spWriteA, spEAddAB);
    spExpr->Dependency(spWriteB, spEAddAB);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = a.EWiseAdd(b, op);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename BinaryOp>
void testMasked(spla::Library &library, std::size_t M, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spOp,
                BinaryOp op, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector b = utils::Vector<Type>::Generate(M, nvals, seed + 1).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned short>::Generate(M, nvals, seed + 2).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());
    b.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Vector::Make(M, spT, library);
    auto spW = spla::Vector::Make(M, spT, library);
    auto spMask = spla::Vector::Make(M, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spEAddAB = spExpr->MakeEWiseAdd(spW, spMask, spOp, spA, spB);
    spExpr->Dependency(spWriteA, spEAddAB);
    spExpr->Dependency(spWriteB, spEAddAB);
    spExpr->Dependency(spWriteMask, spEAddAB);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = a.EWiseAdd(mask, false, b, op);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename BinaryOp>
void testMaskedComplement(spla::Library &library, std::size_t M, std::size_t nvals,
                          const spla::RefPtr<spla::Type> &spT,
                          const spla::RefPtr<spla::FunctionBinary> &spOp,
                          BinaryOp op, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector b = utils::Vector<Type>::Generate(M, nvals, seed + 1).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned short>::Generate(M, nvals, seed + 2).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());
    b.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Vector::Make(M, spT, library);
    auto spW = spla::Vector::Make(M, spT, library);
    auto spMask = spla::Vector::Make(M, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);
    spOpDesc->SetParam(spla::Descriptor::Param::MaskComplement);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetData(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spEAddAB = spExpr->MakeEWiseAdd(spW, spMask, spOp, spA, spB, spOpDesc);
    spExpr->Dependency(spWriteA, spEAddAB);
    spExpr->Dependency(spWriteB, spEAddAB);
    spExpr->Dependency(spWriteMask, spEAddAB);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = a.EWiseAdd(mask, true, b, op);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename BinaryOp>
void testOneIsEmpty(spla::Library &library, std::size_t M, std::size_t nvals,
                    const spla::RefPtr<spla::Type> &spT,
                    const spla::RefPtr<spla::FunctionBinary> &spOp,
                    BinaryOp op, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector b = utils::Vector<Type>::Empty(M);
    utils::Vector mask = utils::Vector<unsigned short>::Generate(M, nvals, seed + 1).SortReduceDuplicates();

    a.Fill(utils::UniformGenerator<Type>());

    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Vector::Make(M, spT, library);
    auto spW1 = spla::Vector::Make(M, spT, library);
    auto spW2 = spla::Vector::Make(M, spT, library);
    auto spMask = spla::Vector::Make(M, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetData(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spEAddAB1 = spExpr->MakeEWiseAdd(spW1, spMask, spOp, spA, spB);
    auto spEAddAB2 = spExpr->MakeEWiseAdd(spW2, spMask, spOp, spB, spA);
    spExpr->Dependency(spWriteA, spEAddAB1);
    spExpr->Dependency(spWriteMask, spEAddAB1);
    spExpr->Dependency(spWriteA, spEAddAB2);
    spExpr->Dependency(spWriteMask, spEAddAB2);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = a.EWiseAdd(mask, false, b, op);
    ASSERT_TRUE(c.Equals(spW1));
    ASSERT_TRUE(c.Equals(spW2));
}

void testNoValues(spla::Library &library, std::size_t M, std::size_t nvals, std::size_t seed = 0) {
    utils::Vector a = utils::Vector<unsigned char>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector b = utils::Vector<unsigned char>::Generate(M, nvals, seed + 1).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned char>::Generate(M, nvals, seed + 2).SortReduceDuplicates();

    auto spT = spla::Types::Void(library);
    auto spA = spla::Vector::Make(M, spT, library);
    auto spB = spla::Vector::Make(M, spT, library);
    auto spW = spla::Vector::Make(M, spT, library);
    auto spMask = spla::Vector::Make(M, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spA, a.GetDataIndices(library), spDesc);
    auto spWriteB = spExpr->MakeDataWrite(spB, b.GetDataIndices(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spEAddAB = spExpr->MakeEWiseAdd(spW, spMask, nullptr, spA, spB);
    spExpr->Dependency(spWriteA, spEAddAB);
    spExpr->Dependency(spWriteB, spEAddAB);
    spExpr->Dependency(spWriteMask, spEAddAB);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<unsigned char> c = a.EWiseAdd(mask, false, b, [](unsigned char a, unsigned char b) { return 0; });
    ASSERT_TRUE(c.EqualsStructure(spW));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        auto spT = spla::Types::Float32(library);
        auto spOp = spla::Functions::PlusFloat32(library);
        auto op = [](float x, float y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<float>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<float>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<float>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testOneIsEmpty<float>(library, M, nvals, spT, spOp, op, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        auto spT = spla::Types::Int32(library);
        auto spOp = spla::Functions::PlusInt32(library);
        auto op = [](std::int32_t x, std::int32_t y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testCommon<std::int32_t>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<std::int32_t>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<std::int32_t>(library, M, nvals, spT, spOp, op, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testOneIsEmpty<std::int32_t>(library, M, nvals, spT, spOp, op, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, nvals, i);
        }
    });
}

TEST(VectorEWiseAdd, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorEWiseAdd, Medium) {
    std::vector<std::size_t> blocksSizes{100, 1000, 10000};
    std::size_t M = 1100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorEWiseAdd, Large) {
    std::vector<std::size_t> blocksSizes{1000, 10000, 100000};
    std::size_t M = 10300;
    test(M, M / 2, M / 10, 5, blocksSizes);
}

SPLA_GTEST_MAIN