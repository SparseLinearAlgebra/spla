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

#include <test_utils.hpp>

#include <spla/spla.hpp>

TEST(BinaryOp, Invoke) {
    using namespace spla;

    EXPECT_EQ(3 + (-20), binary_op::plus<int>::invoke_host(3, -20));
    EXPECT_EQ(2 * -3, binary_op::times<int>::invoke_host(2, -3));
    EXPECT_EQ(30 - 4, binary_op::minus<int>::invoke_host(30, 4));

    EXPECT_TRUE(std::abs((3.0 + (-20.1)) - binary_op::plus<float>::invoke_host(3.0, -20.1)) <= 0.0005f);
    EXPECT_TRUE(std::abs((2.5 * -3.9) - binary_op::times<float>::invoke_host(2.5, -3.9)) <= 0.0005f);
    EXPECT_TRUE(std::abs((30.1 - 4.5) - binary_op::minus<float>::invoke_host(30.1, 4.5)) <= 0.0005f);

    EXPECT_EQ(333.0f, binary_op::first<float>::invoke_host(333.0f, 123414.0f));
    EXPECT_EQ(666.6f, binary_op::second<float>::invoke_host(0.4f, 666.6f));
}

TEST(BinaryOp, Source) {
    using namespace spla;

    std::cout << binary_op::plus<int>::source() << std::endl;
    std::cout << binary_op::plus<unsigned int>::source() << std::endl;
    std::cout << binary_op::plus<float>::source() << std::endl;

    std::cout << binary_op::div<int>::source() << std::endl;
    std::cout << binary_op::div<unsigned int>::source() << std::endl;
    std::cout << binary_op::div<float>::source() << std::endl;

    std::cout << binary_op::first<int>::source() << std::endl;
    std::cout << binary_op::first<unsigned int>::source() << std::endl;
    std::cout << binary_op::first<float>::source() << std::endl;

    std::cout << binary_op::second<int>::source() << std::endl;
    std::cout << binary_op::second<unsigned int>::source() << std::endl;
    std::cout << binary_op::second<float>::source() << std::endl;

    std::cout << binary_op::rminus<int>::source() << std::endl;
    std::cout << binary_op::rminus<unsigned int>::source() << std::endl;
    std::cout << binary_op::rminus<float>::source() << std::endl;
}

SPLA_GTEST_MAIN