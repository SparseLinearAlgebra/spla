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

static void testCommon(spla::Library &library, size_t M, size_t N, size_t nvals, size_t seed = 0) {
    utils::Matrix source = utils::Matrix<float>::Generate(M, N, nvals, seed);
    source.Fill(utils::UniformRealGenerator<float>());

    auto spT = spla::Type::Make("float", sizeof(float), library);
    auto spM = spla::Matrix::Make(M, N, spT, library);
    auto spDesc = spla::Descriptor::Make(library);

    auto spDataSrc = spla::DataMatrix::Make(library);
    spDataSrc->SetRows(source.GetRows());
    spDataSrc->SetCols(source.GetCols());
    spDataSrc->SetVals(source.GetVals());
    spDataSrc->SetNvals(source.GetNvals());

    auto spExprWrite = spla::Expression::Make(library);
    auto spWrite = spExprWrite->MakeDataWrite(spM, spDataSrc, spDesc);

    library.Submit(spExprWrite);
    EXPECT_EQ(spExprWrite->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix<float> expected = source.SortReduceDuplicates();
    EXPECT_TRUE(expected.Equals(spM));
}

static void testSortedNoDuplicates(spla::Library &library, size_t M, size_t N, size_t nvals, size_t seed = 0) {
    utils::Matrix source = utils::Matrix<float>::Generate(M, N, nvals, seed).SortReduceDuplicates();
    source.Fill(utils::UniformRealGenerator<float>());

    auto spT = spla::Type::Make("float", sizeof(float), library);
    auto spM = spla::Matrix::Make(M, N, spT, library);

    // Specify, that values already in row-col order + no duplicates
    auto spDesc = spla::Descriptor::Make(library);
    spDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    spDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    auto spDataSrc = spla::DataMatrix::Make(library);
    spDataSrc->SetRows(source.GetRows());
    spDataSrc->SetCols(source.GetCols());
    spDataSrc->SetVals(source.GetVals());
    spDataSrc->SetNvals(source.GetNvals());

    auto spExprWrite = spla::Expression::Make(library);
    auto spWrite = spExprWrite->MakeDataWrite(spM, spDataSrc, spDesc);

    library.Submit(spExprWrite);
    EXPECT_EQ(spExprWrite->GetState(), spla::Expression::State::Evaluated);

    utils::Matrix<float> &expected = source;
    EXPECT_TRUE(expected.Equals(spM));
}

static void test(size_t M, size_t N, size_t base, size_t step, size_t iter) {
    spla::Library library;

    for (size_t i = 0; i < iter; i++) {
        size_t nvals = base + i * step;
        testCommon(library, M, N, nvals, i);
    }

    for (size_t i = 0; i < iter; i++) {
        size_t nvals = base + i * step;
        testSortedNoDuplicates(library, M, N, nvals, i);
    }
}

TEST(DataMatrix, Small) {
    size_t M = 100, N = 200;
    test(M, N, M, M, 10);
}

TEST(DataMatrix, Medium) {
    size_t M = 1000, N = 2000;
    test(M, N, M, M, 10);
}

TEST(DataMatrix, Large) {
    size_t M = 10000, N = 20000;
    test(M, N, M, M, 5);
}

SPLA_GTEST_MAIN