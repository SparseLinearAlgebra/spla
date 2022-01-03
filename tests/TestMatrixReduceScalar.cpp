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

enum class MaskStatus {
    Absent,
    Existent,
    ExistentComplement,
};

template<typename Type>
void testReduceScalar(spla::Library &library,
                      std::size_t M, std::size_t N, std::size_t nvals,
                      const spla::RefPtr<spla::Type> &spType,
                      const spla::RefPtr<spla::FunctionBinary> &spReduce,
                      MaskStatus maskStatus,
                      const std::optional<std::pair<spla::RefPtr<spla::FunctionBinary>, std::function<Type(Type, Type)>>> &spApply,
                      const std::function<Type(Type, Type)> &reduce,
                      std::size_t seed) {

    Type scalar = utils::UniformGenerator<Type>()();
    const Type scalarInitialValue = scalar;
    utils::Matrix matrix = utils::Matrix<Type>::Generate(M, N, nvals, seed).SortReduceDuplicates();
    utils::Matrix mask = utils::Matrix<unsigned short>::Generate(M, N, nvals, seed * 12 + 3).SortReduceDuplicates();

    utils::UniformGenerator<Type> generator;

    matrix.Fill(std::ref(generator));
    matrix.Fill(std::ref(generator));

    Type reducedActual{};

    auto spMatrix = spla::Matrix::Make(M, N, spType, library);
    auto spMask = maskStatus == MaskStatus::Absent
                          ? nullptr
                          : spla::Matrix::Make(M, N, spla::Types::Void(library), library);
    auto spScalar = spla::Scalar::Make(spType, library);
    auto spReadScalarData = spla::DataScalar::Make(library);
    spReadScalarData->SetValue(&reducedActual);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);
    if (maskStatus == MaskStatus::ExistentComplement) {
        spDesc->SetParam(spla::Descriptor::Param::MaskComplement);
    }

    auto spExpr = spla::Expression::Make(library);
    auto spWriteScalar = spExpr->MakeDataWrite(spScalar, utils::GetData(scalar, library));
    auto spWriteMatrix = spExpr->MakeDataWrite(spMatrix, matrix.GetData(library), spDesc);
    auto spWriteMask = maskStatus == MaskStatus::Absent
                               ? nullptr
                               : spExpr->MakeDataWrite(spMask, mask.GetDataIndices(library), spDesc);

    auto spReduceNode = spExpr->MakeReduceScalar(spScalar,
                                                 spMask,
                                                 spApply.has_value()
                                                         ? spApply.value().first
                                                         : nullptr,
                                                 spReduce,
                                                 spMatrix,
                                                 spDesc);
    auto spReadScalar = spExpr->MakeDataRead(spScalar, spReadScalarData);
    spExpr->Dependency(spWriteScalar, spReduceNode);
    spExpr->Dependency(spWriteMatrix, spReduceNode);
    if (maskStatus != MaskStatus::Absent) {
        spExpr->Dependency(spWriteMask, spReduceNode);
    }
    spExpr->Dependency(spReduceNode, spReadScalar);
    spExpr->SubmitWait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    Type reducedExpected{};

    if (maskStatus == MaskStatus::Absent) {
        reducedExpected = matrix.Reduce(reduce);
    } else {
        auto matrixMasked = matrix.Mask(mask, maskStatus == MaskStatus::ExistentComplement);
        if (matrixMasked.IsEmpty()) {
            ASSERT_FALSE(spScalar->HasValue());
            return;
        }
        reducedExpected = matrixMasked.Reduce(reduce);
    }

    ASSERT_TRUE(spScalar->HasValue());

    if (spApply.has_value()) {
        reducedExpected = spApply.value().second(scalarInitialValue, reducedExpected);
    }

    if (!utils::EqWithRelativeError(reducedExpected, reducedActual)) {
        std::cout << std::setprecision(10)
                  << "Reduced values are not equal: "
                  << "expected: " << reducedExpected << ' '
                  << "actual: " << reducedActual << std::endl;
        ASSERT_EQ(reducedExpected, reducedActual);
    }
}

void test(std::size_t M, std::size_t N, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::uint32_t;
        auto spT = spla::Types::UInt32(library);
        auto spReduce = spla::Functions::PlusUInt32(library);
        auto reduce = [](Type x, Type y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testReduceScalar<Type>(library,
                                   M, N, nvals,
                                   spT,
                                   spReduce,
                                   MaskStatus::Absent,
                                   std::nullopt,
                                   reduce,
                                   i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::uint32_t;
        auto spT = spla::Types::UInt32(library);
        auto spReduce = spla::Functions::PlusUInt32(library);
        auto reduce = [](Type x, Type y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testReduceScalar<Type>(library,
                                   M, N, nvals,
                                   spT,
                                   spReduce,
                                   MaskStatus::Existent,
                                   std::nullopt,
                                   reduce,
                                   i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::uint32_t;
        auto spT = spla::Types::UInt32(library);
        auto spReduce = spla::Functions::PlusUInt32(library);
        auto reduce = [](Type x, Type y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testReduceScalar<Type>(library,
                                   M, N, nvals,
                                   spT,
                                   spReduce,
                                   MaskStatus::ExistentComplement,
                                   std::nullopt,
                                   reduce,
                                   i);
        }
    });

    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::uint32_t;
        auto spT = spla::Types::UInt32(library);
        auto spReduce = spla::Functions::PlusUInt32(library);
        auto spApply = spla::Functions::MultUInt32(library);
        auto reduce = [](Type x, Type y) { return x + y; };
        auto apply = [](Type x, Type y) { return x * y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testReduceScalar<Type>(library,
                                   M, N, nvals,
                                   spT,
                                   spReduce,
                                   MaskStatus::ExistentComplement,
                                   {{spApply, std::function<Type(Type, Type)>(apply)}},
                                   reduce,
                                   i);
        }
    });
}

TEST(MatrixReduceScalar, TinyDense) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 25;
    std::size_t N = 15;
    test(M, N, M * N / 2, 0, 200, blocksSizes);
}

TEST(MatrixReduceScalar, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    std::size_t N = 89;
    test(M, N, M / 2, M / 10, 10, blocksSizes);
}

TEST(MatrixReduceScalar, Medium) {
    std::vector<std::size_t> blocksSizes{100, 1000, 10000};
    std::size_t M = 2140;
    std::size_t N = 3127;
    test(M, N, M / 2, M / 10, 10, blocksSizes);
}

TEST(MatrixReduceScalar, Large) {
    std::vector<std::size_t> blocksSizes{1000, 10000, 100000};
    std::size_t M = 10209;
    std::size_t N = 19303;
    test(M, N, M / 4, M / 20, 10, blocksSizes);
}

TEST(MatrixReduceScalar, MegaLarge) {
    std::vector<std::size_t> blocksSizes{1000000};
    std::size_t M = 990116;
    std::size_t N = 897156;
    test(M, N, M / 10, M / 10, 5, blocksSizes);
}

SPLA_GTEST_MAIN
