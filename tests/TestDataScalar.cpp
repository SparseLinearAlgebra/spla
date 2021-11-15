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

template<typename T, typename Gen>
void test(std::size_t iterations, std::size_t seed, const spla::RefPtr<spla::Type> &spT, spla::Library &library) {
    Gen generator(seed);

    auto spDataWrite = spla::DataScalar::Make(library);
    auto spDataRead = spla::DataScalar::Make(library);

    ASSERT_EQ(sizeof(T), spT->GetByteSize());

    for (std::size_t k = 0; k < iterations; k++) {
        T value = generator();
        std::vector<T> result(1);

        spDataWrite->SetValue(&value);
        spDataRead->SetValue(result.data());

        auto spScalar = spla::Scalar::Make(spT, library);
        auto spExpr = spla::Expression::Make(library);
        auto spWrite = spExpr->MakeDataWrite(spScalar, spDataWrite);
        auto spRead = spExpr->MakeDataRead(spScalar, spDataRead);
        spExpr->Dependency(spWrite, spRead);
        spExpr->Submit();
        spExpr->Wait();

        ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);
        ASSERT_EQ(value, result.front());
    }
}

TEST(DataScalar, Float) {
    spla::Library library;
    spla::RefPtr<spla::Type> type = spla::Types::Float32(library);
    test<float, utils::UniformRealGenerator<float>>(10, 0, type, library);
}

TEST(DataScalar, Int) {
    spla::Library library;
    spla::RefPtr<spla::Type> type = spla::Types::Int32(library);
    test<std::int32_t, utils::UniformIntGenerator<std::int32_t>>(10, 0, type, library);
}

SPLA_GTEST_MAIN