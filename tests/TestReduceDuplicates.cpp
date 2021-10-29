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
#include <compute/SplaReduceDuplicates.hpp>

TEST(Compute, ReduceDuplicates_KKV) {
    namespace compute = boost::compute;

    compute::device gpu = compute::system::default_device();
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> indices1 = {0, 0, 1, 2, 3, 3, 4, 5, 5};
    std::vector<unsigned int> indices2 = {0, 1, 1, 3, 3, 3, 4, 5, 5};
    std::vector<int> values = {-1, 2, 4, 9, 0, -1, 4, 10, 20};

    std::vector<unsigned int> resultIndices1 = {0, 0, 1, 2, 3, 4, 5};
    std::vector<unsigned int> resultIndices2 = {0, 1, 1, 3, 3, 4, 5};
    std::vector<int> resultValues = {-1, 2, 4, 9, -1, 4, 30};

    auto count = indices1.size();
    auto size = sizeof(int);

    compute::vector<unsigned int> diI1(count, ctx);
    compute::vector<unsigned int> diI2(count, ctx);
    compute::vector<unsigned char> diV(count * size, ctx);

    compute::copy_n(indices1.begin(), count, diI1.begin(), queue);
    compute::copy_n(indices2.begin(), count, diI2.begin(), queue);
    compute::copy_n((unsigned char *) values.data(), count * size, diV.begin(), queue);

    compute::vector<unsigned int> drI1(count, ctx);
    compute::vector<unsigned int> drI2(count, ctx);
    compute::vector<unsigned char> drV(count * size, ctx);

    std::string op = "int a = *((const int*)vp_a);"
                     "int b = *((const int*)vp_b);"
                     "int* c = (int*)vp_c;"
                     "*c = a + b;";

    auto nnz = spla::ReduceDuplicates(diI1, diI2, diV,
                                      drI1, drI2, drV,
                                      size,
                                      op,
                                      queue);

    ASSERT_EQ(nnz, resultIndices1.size());
    ASSERT_EQ(nnz, resultIndices2.size());
    ASSERT_EQ(nnz, resultValues.size());

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices1[i], drI1[i]);
    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices2[i], drI2[i]);

    std::vector<int> result(nnz);
    compute::copy(drV.begin(), drV.end(), (unsigned char *) result.data(), queue);

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultValues[i], result[i]);
}

TEST(Compute, ReduceDuplicates_KK) {
    namespace compute = boost::compute;

    compute::device gpu = compute::system::default_device();
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> indices1 = {0, 0, 1, 2, 3, 3, 4, 5, 5};
    std::vector<unsigned int> indices2 = {0, 1, 1, 3, 3, 3, 4, 5, 5};

    std::vector<unsigned int> resultIndices1 = {0, 0, 1, 2, 3, 4, 5};
    std::vector<unsigned int> resultIndices2 = {0, 1, 1, 3, 3, 4, 5};

    auto count = indices1.size();
    auto size = 0;

    compute::vector<unsigned int> diI1(count, ctx);
    compute::vector<unsigned int> diI2(count, ctx);

    compute::copy_n(indices1.begin(), count, diI1.begin(), queue);
    compute::copy_n(indices2.begin(), count, diI2.begin(), queue);

    compute::vector<unsigned int> drI1(count, ctx);
    compute::vector<unsigned int> drI2(count, ctx);

    auto nnz = spla::ReduceDuplicates(diI1, diI2,
                                      drI1, drI2,
                                      size,
                                      "",
                                      queue);

    ASSERT_EQ(nnz, resultIndices1.size());
    ASSERT_EQ(nnz, resultIndices2.size());

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices1[i], drI1[i]);
    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices2[i], drI2[i]);
}

TEST(Compute, ReduceDuplicates_KV) {
    namespace compute = boost::compute;

    compute::device gpu = compute::system::default_device();
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> indices = {0, 0, 1, 2, 3, 3, 4, 5, 5};
    std::vector<int> values = {-1, 2, 4, 9, 0, -1, 4, 10, 20};

    std::vector<unsigned int> resultIndices = {0, 1, 2, 3, 4, 5};
    std::vector<int> resultValues = {1, 4, 9, -1, 4, 30};

    auto count = indices.size();
    auto size = sizeof(int);

    compute::vector<unsigned int> diI(count, ctx);
    compute::vector<unsigned char> diV(count * size, ctx);

    compute::copy_n(indices.begin(), count, diI.begin(), queue);
    compute::copy_n((unsigned char *) values.data(), count * size, diV.begin(), queue);

    compute::vector<unsigned int> drI(count, ctx);
    compute::vector<unsigned char> drV(count * size, ctx);

    std::string op = "int a = *((const int*)vp_a);"
                     "int b = *((const int*)vp_b);"
                     "int* c = (int*)vp_c;"
                     "*c = a + b;";

    auto nnz = spla::ReduceDuplicates(diI, diV,
                                      drI, drV,
                                      size,
                                      op,
                                      queue);

    ASSERT_EQ(nnz, resultIndices.size());
    ASSERT_EQ(nnz, resultValues.size());

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices[i], drI[i]);

    std::vector<int> result(nnz);
    compute::copy(drV.begin(), drV.end(), (unsigned char *) result.data(), queue);

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultValues[i], result[i]);
}

TEST(Compute, ReduceDuplicates_K) {
    namespace compute = boost::compute;

    compute::device gpu = compute::system::default_device();
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> indices = {0, 0, 1, 2, 3, 3, 4, 5, 5};
    std::vector<unsigned int> resultIndices = {0, 1, 2, 3, 4, 5};

    auto count = indices.size();
    auto size = 0;

    compute::vector<unsigned int> diI(count, ctx);
    compute::copy_n(indices.begin(), count, diI.begin(), queue);

    compute::vector<unsigned int> drI(count, ctx);

    auto nnz = spla::ReduceDuplicates(diI,
                                      drI,
                                      size,
                                      "",
                                      queue);

    ASSERT_EQ(nnz, resultIndices.size());

    for (std::size_t i = 0; i < nnz; i++)
        EXPECT_EQ(resultIndices[i], drI[i]);
}

SPLA_GTEST_MAIN