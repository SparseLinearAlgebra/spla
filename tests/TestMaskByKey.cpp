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

TEST(Compute, MaskByKey) {
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

    compute::copy(mask.begin(), mask.end(), deviceMask.begin(), queue);
    compute::copy(keys.begin(), keys.end(), deviceKeys.begin(), queue);
    compute::copy(values.begin(), values.end(), deviceValues.begin(), queue);

    BOOST_COMPUTE_FUNCTION(bool, equals, (unsigned int a, unsigned int b), {
        return a == b;
    });

    BOOST_COMPUTE_FUNCTION(bool, compare, (unsigned int a, unsigned int b), {
        return a < b;
    });

    auto count = spla::MaskByKey(deviceMask.begin(), deviceMask.end(),
                                 deviceKeys.begin(), deviceKeys.end(),
                                 deviceValues.begin(),
                                 deviceKeys.begin(),
                                 deviceValues.begin(),
                                 compare,
                                 equals,
                                 queue);

    ASSERT_EQ(count, expectedKeys.size());
    ASSERT_EQ(count, expectedValues.size());

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedKeys[i], (deviceKeys.begin() + i).read(queue));

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedValues[i], (deviceValues.begin() + i).read(queue));
}

TEST(Compute, MaskByKey2) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    using Index = boost::tuple<unsigned int, unsigned int>;

    std::vector<Index> mask = {Index(0, 10), Index(1, 0), Index(1, 2), Index(8, 0), Index(10, 0), Index(27, 0)};
    std::vector<Index> keys = {Index(0, 10), Index(0, 20), Index(1, 2), Index(7, 0), Index(10, 0), Index(15, 10), Index(19, 40), Index(27, 0)};
    std::vector<unsigned int> values = {66, 24, 13, 7, 1, 0, 1, 200};

    std::vector<Index> expectedKeys = {Index(0, 10), Index(1, 2), Index(10, 0), Index(27, 0)};
    std::vector<unsigned int> expectedValues = {66, 13, 1, 200};

    compute::vector<Index> deviceMask(mask.size(), ctx);
    compute::vector<Index> deviceKeys(keys.size(), ctx);
    compute::vector<unsigned int> deviceValues(values.size(), ctx);

    compute::copy(mask.begin(), mask.end(), deviceMask.begin(), queue);
    compute::copy(keys.begin(), keys.end(), deviceKeys.begin(), queue);
    compute::copy(values.begin(), values.end(), deviceValues.begin(), queue);

    BOOST_COMPUTE_FUNCTION(bool, equals, (Index a, Index b), {
        uint a1 = boost_tuple_get(a, 0);
        uint a2 = boost_tuple_get(a, 1);
        uint b1 = boost_tuple_get(b, 0);
        uint b2 = boost_tuple_get(b, 1);
        return a1 == b1 && a2 == b2;
    });

    BOOST_COMPUTE_FUNCTION(bool, compare, (Index a, Index b), {
        uint a1 = boost_tuple_get(a, 0);
        uint a2 = boost_tuple_get(a, 1);
        uint b1 = boost_tuple_get(b, 0);
        uint b2 = boost_tuple_get(b, 1);
        return a1 < b1 || (a1 == b1 && a2 < b2);
    });

    auto count = spla::MaskByKey(deviceMask.begin(), deviceMask.end(),
                                 deviceKeys.begin(), deviceKeys.end(),
                                 deviceValues.begin(),
                                 deviceKeys.begin(),
                                 deviceValues.begin(),
                                 compare,
                                 equals,
                                 queue);

    ASSERT_EQ(count, expectedKeys.size());
    ASSERT_EQ(count, expectedValues.size());

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedKeys[i].get<0>(), ((deviceKeys.begin() + i).read(queue)).get<0>());
        EXPECT_EQ(expectedKeys[i].get<1>(), ((deviceKeys.begin() + i).read(queue)).get<1>());
    }

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedValues[i], (deviceValues.begin() + i).read(queue));
        EXPECT_EQ(expectedValues[i], (deviceValues.begin() + i).read(queue));
    }
}

TEST(Compute, MaskByPairKey) {
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

    compute::copy(mask1.begin(), mask1.end(), deviceMask1.begin(), queue);
    compute::copy(mask2.begin(), mask2.end(), deviceMask2.begin(), queue);
    compute::copy(keys1.begin(), keys1.end(), deviceKeys1.begin(), queue);
    compute::copy(keys2.begin(), keys2.end(), deviceKeys2.begin(), queue);
    compute::copy(values.begin(), values.end(), deviceValues.begin(), queue);

    auto count = spla::MaskByPairKey(deviceMask1.begin(), deviceMask1.end(),
                                     deviceMask2.begin(),
                                     deviceKeys1.begin(), deviceKeys1.end(),
                                     deviceKeys2.begin(),
                                     deviceValues.begin(),
                                     deviceKeys1.begin(),
                                     deviceKeys2.begin(),
                                     deviceValues.begin(),
                                     queue);

    ASSERT_EQ(count, expectedKeys1.size());
    ASSERT_EQ(count, expectedKeys2.size());
    ASSERT_EQ(count, expectedValues.size());

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedKeys1[i], (deviceKeys1.begin() + i).read(queue));
        EXPECT_EQ(expectedKeys2[i], (deviceKeys2.begin() + i).read(queue));
    }

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedValues[i], (deviceValues.begin() + i).read(queue));
        EXPECT_EQ(expectedValues[i], (deviceValues.begin() + i).read(queue));
    }
}

SPLA_GTEST_MAIN