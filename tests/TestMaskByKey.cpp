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
#include <compute/SplaMaskByKey.hpp>

TEST(Compute, MaskByKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> mask = {0, 1, 3, 8, 10, 27};
    std::vector<unsigned int> keys = {0, 2, 3, 4, 7, 9, 10, 15, 19, 27};
    std::vector<unsigned int> values = {66, 24, 13, 0, 7, 119, 1, 0, 1, 200};

    std::vector<unsigned int> expectedKeys = {0, 3, 10, 27};
    std::vector<unsigned int> expectedValues = {66, 13, 1, 200};

    compute::vector<unsigned int> deviceMask(mask.size(), ctx);
    compute::vector<unsigned int> deviceKeys(keys.size(), ctx);
    compute::vector<unsigned int> deviceValues(values.size(), ctx);

    compute::vector<unsigned int> resultKeys(ctx);
    compute::vector<unsigned int> resultValues(ctx);

    compute::copy(mask.begin(), mask.end(), deviceMask.begin(), queue);
    compute::copy(keys.begin(), keys.end(), deviceKeys.begin(), queue);
    compute::copy(values.begin(), values.end(), deviceValues.begin(), queue);

    auto count = spla::MaskByKeys(deviceMask,
                                  deviceKeys,
                                  deviceValues,
                                  resultKeys,
                                  resultValues,
                                  queue);

    ASSERT_EQ(count, expectedKeys.size());
    ASSERT_EQ(count, expectedValues.size());

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys[i], (resultKeys.begin() + i).read(queue));

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedValues[i], (resultValues.begin() + i).read(queue));
}

TEST(Compute, MaskKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> mask = {0, 1, 3, 8, 10, 27};
    std::vector<unsigned int> keys = {0, 2, 3, 4, 7, 9, 10, 15, 19, 27};

    std::vector<unsigned int> expectedKeys = {0, 3, 10, 27};

    compute::vector<unsigned int> deviceMask(mask.size(), ctx);
    compute::vector<unsigned int> deviceKeys(keys.size(), ctx);

    compute::vector<unsigned int> resultKeys(ctx);
    compute::vector<unsigned int> resultValues(ctx);

    compute::copy(mask.begin(), mask.end(), deviceMask.begin(), queue);
    compute::copy(keys.begin(), keys.end(), deviceKeys.begin(), queue);

    auto count = spla::MaskKeys(deviceMask,
                                deviceKeys,
                                resultKeys,
                                queue);

    ASSERT_EQ(count, expectedKeys.size());

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys[i], (resultKeys.begin() + i).read(queue));
}

TEST(Compute, MaskByPairKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> mask1 = {0, 1, 1, 8, 10, 27};
    std::vector<unsigned int> mask2 = {10, 0, 2, 0, 0, 100};
    std::vector<unsigned int> keys1 = {0, 0, 1, 7, 10, 15, 19, 27};
    std::vector<unsigned int> keys2 = {10, 20, 2, 0, 0, 10, 40, 100};
    std::vector<unsigned int> values = {66, 24, 13, 7, 1, 0, 1, 200};

    std::vector<unsigned int> expectedKeys1 = {0, 1, 10, 27};
    std::vector<unsigned int> expectedKeys2 = {10, 2, 0, 100};
    std::vector<unsigned int> expectedValues = {66, 13, 1, 200};

    compute::vector<unsigned int> deviceMask1(mask1.size(), ctx);
    compute::vector<unsigned int> deviceMask2(mask2.size(), ctx);
    compute::vector<unsigned int> deviceKeys1(keys1.size(), ctx);
    compute::vector<unsigned int> deviceKeys2(keys2.size(), ctx);
    compute::vector<unsigned int> deviceValues(values.size(), ctx);

    compute::vector<unsigned int> resultKeys1(ctx);
    compute::vector<unsigned int> resultKeys2(ctx);
    compute::vector<unsigned int> resultValues(ctx);

    compute::copy(mask1.begin(), mask1.end(), deviceMask1.begin(), queue);
    compute::copy(mask2.begin(), mask2.end(), deviceMask2.begin(), queue);
    compute::copy(keys1.begin(), keys1.end(), deviceKeys1.begin(), queue);
    compute::copy(keys2.begin(), keys2.end(), deviceKeys2.begin(), queue);
    compute::copy(values.begin(), values.end(), deviceValues.begin(), queue);

    auto count = spla::MaskByPairKeys(deviceMask1, deviceMask2,
                                      deviceKeys1, deviceKeys2, deviceValues,
                                      resultKeys1, resultKeys2, resultValues,
                                      queue);

    ASSERT_EQ(count, expectedKeys1.size());
    ASSERT_EQ(count, expectedKeys2.size());
    ASSERT_EQ(count, expectedValues.size());

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys1[i], (resultKeys1.begin() + i).read(queue));

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys2[i], (resultKeys2.begin() + i).read(queue));

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedValues[i], (resultValues.begin() + i).read(queue));
}

TEST(Compute, MaskPairKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<unsigned int> mask1 = {0, 1, 1, 8, 10, 27};
    std::vector<unsigned int> mask2 = {10, 0, 2, 0, 0, 100};
    std::vector<unsigned int> keys1 = {0, 0, 1, 7, 10, 15, 19, 27};
    std::vector<unsigned int> keys2 = {10, 20, 2, 0, 0, 10, 40, 100};

    std::vector<unsigned int> expectedKeys1 = {0, 1, 10, 27};
    std::vector<unsigned int> expectedKeys2 = {10, 2, 0, 100};

    compute::vector<unsigned int> deviceMask1(mask1.size(), ctx);
    compute::vector<unsigned int> deviceMask2(mask2.size(), ctx);
    compute::vector<unsigned int> deviceKeys1(keys1.size(), ctx);
    compute::vector<unsigned int> deviceKeys2(keys2.size(), ctx);

    compute::vector<unsigned int> resultKeys1(ctx);
    compute::vector<unsigned int> resultKeys2(ctx);

    compute::copy(mask1.begin(), mask1.end(), deviceMask1.begin(), queue);
    compute::copy(mask2.begin(), mask2.end(), deviceMask2.begin(), queue);
    compute::copy(keys1.begin(), keys1.end(), deviceKeys1.begin(), queue);
    compute::copy(keys2.begin(), keys2.end(), deviceKeys2.begin(), queue);

    auto count = spla::MaskPairKeys(deviceMask1, deviceMask2,
                                    deviceKeys1, deviceKeys2,
                                    resultKeys1, resultKeys2,
                                    queue);

    ASSERT_EQ(count, expectedKeys1.size());
    ASSERT_EQ(count, expectedKeys2.size());

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys1[i], (resultKeys1.begin() + i).read(queue));

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys2[i], (resultKeys2.begin() + i).read(queue));
}

SPLA_GTEST_MAIN