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
void testSimple(spla::Library &library, std::size_t M, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spType,
                const spla::RefPtr<spla::FunctionBinary> &spReduce,
                BinaryOp reduce, std::size_t seed = 0) {
    Type s = utils::UniformGenerator<Type>()();
    utils::Vector v = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();

    v.Fill(utils::UniformGenerator<Type>());

    Type reducedActual{};

    auto spV = spla::Vector::Make(M, spType, library);
    auto spS = spla::Scalar::Make(spType, library);
    auto spReadScalarData = spla::DataScalar::Make(library);
    spReadScalarData->SetValue(&reducedActual);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteS = spExpr->MakeDataWrite(spS, utils::GetData(s, library));
    auto spWriteVector = spExpr->MakeDataWrite(spV, v.GetData(library), spDesc);
    auto spReduceNode = spExpr->MakeReduce(spS, spReduce, spV, spOpDesc);
    auto spReadScalar = spExpr->MakeDataRead(spS, spReadScalarData);
    spExpr->Dependency(spWriteS, spReduceNode);
    spExpr->Dependency(spWriteVector, spReduceNode);
    spExpr->Dependency(spReduceNode, spReadScalar);

    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);
    ASSERT_TRUE(spS->HasValue());

    Type reducedExpected = v.Reduce(reduce);

    if (!utils::EqWithError(reducedExpected, reducedActual)) {
        std::cout << "Reduced values are not equal: "
                  << "expected: " << reducedExpected << ' '
                  << "actual: " << reducedActual << std::endl;
        ASSERT_TRUE(utils::EqWithError(reducedExpected, reducedActual));
    }
}

template<typename Type>
void testEmpty(spla::Library &library,
               const spla::RefPtr<spla::Type> &spType,
               const spla::RefPtr<spla::FunctionBinary> &spReduce) {
    Type s = utils::UniformGenerator<Type>()();

    Type reducedActual{};

    auto spV = spla::Vector::Make(0, spType, library);
    auto spS = spla::Scalar::Make(spType, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);

    auto spExpr = spla::Expression::Make(library);
    auto spReduceNode = spExpr->MakeReduce(spS, spReduce, spV, spOpDesc);

    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);

    ASSERT_FALSE(spS->HasValue());
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::int32_t;
        auto spT = spla::Types::Int32(library);
        auto spAccum = spla::Functions::PlusInt32(library);
        auto accum = [](Type x, Type y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testSimple<Type>(library, M, nvals, spT, spAccum, accum, i);
        }
        testEmpty<Type>(library, spT, spAccum);
    });
}

TEST(VectorReduce, Small) {
    std::vector<std::size_t> blocksSizes{100, 1000};
    std::size_t M = 100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorReduce, Medium) {
    std::vector<std::size_t> blocksSizes{100, 1000, 10000};
    std::size_t M = 1100;
    test(M, M / 2, M / 10, 10, blocksSizes);
}

TEST(VectorReduce, Large) {
    std::vector<std::size_t> blocksSizes{1000, 10000, 100000};
    std::size_t M = 10300;
    test(M, M / 2, M / 10, 5, blocksSizes);
}

SPLA_GTEST_MAIN
