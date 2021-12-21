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
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaScalarValue.hpp>

template<typename Type, typename BinaryOp>
void testSimple(spla::Library &library, std::size_t M, std::size_t nvals,
                const spla::RefPtr<spla::Type> &spTypeVec,
                const spla::RefPtr<spla::Type> &spTypeRes,
                const spla::RefPtr<spla::FunctionBinary> &spReduce,
                BinaryOp reduce, std::size_t seed = 0) {
    Type s = utils::UniformGenerator<Type>()();
    utils::Vector v = utils::Vector<Type>::Generate(M, nvals, seed).SortReduceDuplicates();

    v.Fill(utils::UniformGenerator<Type>());

    auto spV = spla::Vector::Make(M, spTypeVec, library);
    auto spS = spla::Scalar::Make(spTypeRes, library);

    // Specify, that values already in row order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spOpDesc = spla::Descriptor::Make(library);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteS = spExpr->MakeDataWrite(spS, utils::GetData(s, library));
    auto spWriteVector = spExpr->MakeDataWrite(spV, v.GetData(library), spDesc);
    auto spReduceNode = spExpr->MakeReduce(spS, spReduce, spV, spOpDesc);
    spExpr->Dependency(spWriteS, spReduceNode);
    spExpr->Dependency(spWriteVector, spReduceNode);

    spExpr->Submit();
    spExpr->Wait();
    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);
    ASSERT_TRUE(spS->HasValue());
    auto &reducedActual = spS->GetStorage()->GetValue()->GetVal();
    auto reducedExpected = v.Reduce(reduce);

    std::vector<unsigned char> bytesActual(reducedActual.size());
    std::vector<unsigned char> bytesExpected(sizeof(reducedExpected));

    std::copy(reducedActual.begin(), reducedActual.end(), bytesActual.begin());
    std::memcpy(bytesExpected.data(), &reducedExpected, sizeof(reducedExpected));

    ASSERT_TRUE(utils::ValuesEqual<Type>(bytesActual, bytesExpected));
}

void test(std::size_t M, std::size_t base, std::size_t step, std::size_t iter, const std::vector<std::size_t> &blocksSizes) {
    utils::testBlocks(blocksSizes, [=](spla::Library &library) {
        using Type = std::int32_t;
        auto spT = spla::Types::Int32(library);
        auto spAccum = spla::Functions::PlusInt32(library);
        auto accum = [](Type x, Type y) { return x + y; };

        for (std::size_t i = 0; i < iter; i++) {
            std::size_t nvals = base + i * step;
            testSimple<Type>(library, M, nvals, spT, spT, spAccum, accum, i);
        }
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
