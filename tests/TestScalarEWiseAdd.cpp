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

template<typename C, typename A, typename B, typename Add>
void test(const spla::RefPtr<spla::Type> &spTA,
          const spla::RefPtr<spla::Type> &spTB,
          const spla::RefPtr<spla::Type> &spTC,
          const spla::RefPtr<spla::FunctionBinary> &spAddFunction,
          A a,
          B b,
          Add add,
          spla::Library &library) {

    auto spDataWriteA = spla::DataScalar::Make(library);
    auto spDataWriteB = spla::DataScalar::Make(library);

    auto spDataReadC = spla::DataScalar::Make(library);

    ASSERT_EQ(sizeof(A), spTA->GetByteSize());
    ASSERT_EQ(sizeof(B), spTB->GetByteSize());
    ASSERT_EQ(sizeof(C), spTC->GetByteSize());

    C cActual = 0;
    C cExpected = add(a, b);

    spDataWriteA->SetValue(&a);
    spDataWriteB->SetValue(&b);

    spDataReadC->SetValue(&cActual);

    auto spScalarA = spla::Scalar::Make(spTA, library);
    auto spScalarB = spla::Scalar::Make(spTB, library);
    auto spScalarC = spla::Scalar::Make(spTC, library);

    auto spExpr = spla::Expression::Make(library);
    auto spWriteA = spExpr->MakeDataWrite(spScalarA, spDataWriteA);
    auto spWriteB = spExpr->MakeDataWrite(spScalarB, spDataWriteB);
    auto spAddAB = spExpr->MakeEWiseAdd(spScalarC, spAddFunction, spScalarA, spScalarB);
    auto spReadC = spExpr->MakeDataRead(spScalarC, spDataReadC);

    spExpr->Dependency(spWriteA, spAddAB);
    spExpr->Dependency(spWriteB, spAddAB);
    spExpr->Dependency(spAddAB, spReadC);
    spExpr->Submit();
    spExpr->Wait();

    ASSERT_EQ(spExpr->GetState(), spla::Expression::State::Evaluated);
    ASSERT_EQ(cActual, cExpected);
}

TEST(ScalarEWiseAdd, SameType) {
    spla::Library library(spla::Library::Config().SetDeviceType(spla::Library::Config::DeviceType::GPU));

    auto testLocal = [&](std::uint32_t a, std::uint32_t b) {
        test<std::uint32_t, std::uint32_t, std::uint32_t>(spla::Types::UInt32(library),
                                                          spla::Types::UInt32(library),
                                                          spla::Types::UInt32(library),
                                                          spla::Functions::PlusUInt32(library),
                                                          a,
                                                          b,
                                                          std::plus<>(),
                                                          library);
    };

    testLocal(42, 13);
    //    testLocal(1289372, 9492818);
    //    testLocal(0, std::numeric_limits<std::uint32_t>::max());
    //    testLocal(std::numeric_limits<std::uint32_t>::max(), 0);
}

TEST(ScalarEWiseAdd, DifferentTypes) {
    spla::Library library(spla::Library::Config().SetDeviceType(spla::Library::Config::DeviceType::GPU));

    auto ta = spla::Types::Int8(library);
    auto tb = spla::Types::Int16(library);
    auto tc = spla::Types::Int32(library);

    std::string concatCode = "int  a = *((_ACCESS_A const char*)vp_a);"
                             "int  b = *((_ACCESS_B const short*)vp_b);"
                             "_ACCESS_C int* c = (_ACCESS_C int*)vp_c;"
                             "*c = a | (b << 8);";

    auto concat = [](std::int8_t a, std::int16_t b) {
        return static_cast<std::uint32_t>(a) | (static_cast<std::uint32_t>(b) << 8);
    };

    auto concatSp = spla::FunctionBinary::Make(ta, tb, tc, concatCode, library);

    auto testLocal = [&](std::int8_t a, std::int16_t b) {
        test<std::int32_t, std::int8_t, std::int16_t>(ta, tb, tc,
                                                      concatSp,
                                                      a, b,
                                                      concat,
                                                      library);
    };

    testLocal(42, 13);
    testLocal(0, 0);
    testLocal(std::numeric_limits<std::int8_t>::max(), 2312);
    testLocal(101, std::numeric_limits<std::int16_t>::max());
    testLocal(std::numeric_limits<std::int8_t>::max(), std::numeric_limits<std::int16_t>::max());
}

SPLA_GTEST_MAIN
