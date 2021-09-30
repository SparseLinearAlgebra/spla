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
#include <spla-cpp/Spla.hpp>

TEST(Expression, Empty) {
    using namespace spla;

    Library library;
    RefPtr<Expression> empty = Expression::Make(library);
    library.Submit(empty);

    EXPECT_EQ(empty->GetState(), Expression::State::Evaluated);
}

TEST(Expression, Graph) {
    using namespace spla;

    size_t M = 10000, N = 2000;

    Library library;
    auto type = Type::Make("TEST", 0, library);
    auto mat = Matrix::Make(M, N, type, library);
    auto data = DataMatrix::Make(library);

    auto expr = Expression::Make(library);
    auto node0 = expr->MakeDataWrite(mat, data, nullptr);
    auto node1 = expr->MakeDataWrite(mat, data, nullptr);
    auto node2 = expr->MakeDataWrite(mat, data, nullptr);
    auto node3 = expr->MakeDataWrite(mat, data, nullptr);

    expr->Dependency(node0, node2);
    expr->Dependency(node1, node2);
    expr->Dependency(node2, node3);

    library.Submit(expr);

    EXPECT_EQ(expr->GetState(), Expression::State::Evaluated);
}

SPLA_GTEST_MAIN