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
void testMasked(spla::Library &library, std::size_t M, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spT,
                const spla::RefPtr<spla::FunctionBinary> &spAccum,
                BinaryOp accum, std::size_t seed = 0) {
    auto applyAccum = spAccum.IsNotNull();
    Type s = utils::UniformGenerator<Type>()();
    utils::Vector w = applyAccum ? utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates() : utils::Vector<Type>::Empty(M);
    utils::Vector mask = utils::Vector<unsigned char>::Generate(M, nvals, seed + 1).SortReduceDuplicates();

    w.Fill(utils::UniformGenerator<Type>());

    auto spW = spla::Vector::Make(M, spT, library);
    auto spS = spla::Scalar::Make(spT, library);
    auto spMask = spla::Vector::Make(M, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);
    if (applyAccum) spOpDesc->SetParam(spla::Descriptor::Param::AccumResult);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteS = spExpr->MakeDataWrite(spS, utils::GetData(s, library));
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spAssign = spExpr->MakeAssign(spW, spMask, spAccum, spS, spOpDesc);
    spExpr->Dependency(spWriteS, spAssign);
    spExpr->Dependency(spWriteMask, spAssign);

    if (applyAccum) {
        auto spWriteW = spExpr->MakeDataWrite(spW, w.GetData(library), spDesc);
        spExpr->Dependency(spWriteW, spAssign);
    }

    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = w.Assign(mask, false, s, accum);
    ASSERT_TRUE(c.Equals(spW));
}

template<typename Type, typename BinaryOp>
void testMaskedComplement(spla::Library &library, std::size_t M, std::size_t nvals,
                          const spla::RefPtr<spla::Type> &spT,
                          const spla::RefPtr<spla::FunctionBinary> &spAccum,
                          BinaryOp accum, std::size_t seed = 0) {
    auto applyAccum = spAccum.IsNotNull();
    Type s = utils::UniformGenerator<Type>()();
    utils::Vector w = applyAccum ? utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates() : utils::Vector<Type>::Empty(M);
    utils::Vector mask = utils::Vector<unsigned char>::Generate(M, nvals, seed + 1).SortReduceDuplicates();

    w.Fill(utils::UniformGenerator<Type>());

    auto spW = spla::Vector::Make(M, spT, library);
    auto spS = spla::Scalar::Make(spT, library);
    auto spMask = spla::Vector::Make(M, spla::Types::Void(library), library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);
    spOpDesc->SetParam(spla::Descriptor::Param::MaskComplement);
    if (applyAccum) spOpDesc->SetParam(spla::Descriptor::Param::AccumResult);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteS = spExpr->MakeDataWrite(spS, utils::GetData(s, library));
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spAssign = spExpr->MakeAssign(spW, spMask, spAccum, spS, spOpDesc);
    spExpr->Dependency(spWriteS, spAssign);
    spExpr->Dependency(spWriteMask, spAssign);

    if (applyAccum) {
        auto spWriteW = spExpr->MakeDataWrite(spW, w.GetData(library), spDesc);
        spExpr->Dependency(spWriteW, spAssign);
    }

    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<Type> c = w.Assign(mask, true, s, accum);
    ASSERT_TRUE(c.Equals(spW));
}

void testNoValues(spla::Library &library, std::size_t M, std::size_t nvals, std::size_t seed = 0) {
    utils::Vector w = utils::Vector<unsigned char>::Generate(M, nvals, seed).SortReduceDuplicates();
    utils::Vector mask = utils::Vector<unsigned char>::Generate(M, nvals, seed + 1).SortReduceDuplicates();

    auto spT = spla::Types::Void(library);
    auto spW = spla::Vector::Make(M, spT, library);
    auto spMask = spla::Vector::Make(M, spT, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);
    spOpDesc->SetParam(spla::Descriptor::Param::AccumResult);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteW = spExpr->MakeDataWrite(spW, w.GetDataIndices(library), spDesc);
    auto spWriteMask = spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);
    auto spAssign = spExpr->MakeAssign(spW, spMask, nullptr, nullptr, spOpDesc);
    spExpr->Dependency(spWriteW, spAssign);
    spExpr->Dependency(spWriteMask, spAssign);
    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    utils::Vector<unsigned char> c = w.Assign(mask, false, 0, [](unsigned char, unsigned char) { return 0; });
    ASSERT_TRUE(c.EqualsStructure(spW));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = float;
        auto spT = spla::Types::Float32(library);
        auto spAccum = spla::Functions::PlusFloat32(library);
        auto accum = [](Type x, Type y) { return x + y; };
        auto accumNull = [](Type, Type y) { return y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<Type>(library, M, nvals, spT, spAccum, accum, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<Type>(library, M, nvals, spT, nullptr, accumNull, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<Type>(library, M, nvals, spT, spAccum, accum, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<Type>(library, M, nvals, spT, nullptr, accumNull, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::int32_t;
        auto spT = spla::Types::Int32(library);
        auto spAccum = spla::Functions::PlusInt32(library);
        auto accum = [](Type x, Type y) { return x + y; };
        auto accumNull = [](Type, Type y) { return y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<Type>(library, M, nvals, spT, spAccum, accum, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMasked<Type>(library, M, nvals, spT, nullptr, accumNull, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<Type>(library, M, nvals, spT, spAccum, accum, i);
        }

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testMaskedComplement<Type>(library, M, nvals, spT, nullptr, accumNull, i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testNoValues(library, M, nvals, i);
        }
    });
}

TEST(VectorAssign, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorAssign, Medium) {
    std::vector<std::size_t> blocksSizes{100, 1000, 10000};
    std::size_t M = 1100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorAssign, Large) {
    std::vector<std::size_t> blocksSizes{1000, 10000, 100000};
    std::size_t M = 10300;
    test(M, M / 2, M / 10, 5, blocksSizes);
}

SPLA_GTEST_MAIN