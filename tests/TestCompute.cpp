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
#include <compute/SplaMergeByKey.hpp>

TEST(Compute, MergeByKey) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<int> hostKeysA{1, 2, 3, 4, 5};
    std::vector<char> hostValsA{'o', 't', 't', 'f', 'f'};

    std::vector<int> hostKeysB{2, 2, 6, 6, 7};
    std::vector<char> hostValsB{'t', 't', 's', 's', 's'};

    compute::vector<int> keysA(hostKeysA, queue);
    compute::vector<char> valsA(hostValsA, queue);
    compute::vector<int> keysB(hostKeysB, queue);
    compute::vector<char> valsB(hostValsB, queue);

    compute::vector<int> keysRes(10, ctx);
    compute::vector<char> valsRes(10, ctx);

    auto [keysResEnd, valsResEnd] = spla::MergeByKey(
            keysA.begin(), keysA.end(),
            valsA.begin(),
            keysB.begin(), keysB.end(),
            valsB.begin(),
            keysRes.begin(),
            valsRes.begin(),
            queue);

    std::vector<int> keysExpected{1, 2, 2, 2, 3, 4, 5, 6, 6, 7};
    std::vector<char> valsExpected{'o', 't', 't', 't', 't', 'f', 'f', 's', 's', 's'};

    for (auto it = keysRes.begin(); it < keysResEnd; ++it) {
        std::size_t ind = it - keysRes.begin();
        EXPECT_EQ(it.read(queue), keysExpected[ind]);
    }

    for (auto it = valsRes.begin(); it < valsResEnd; ++it) {
        std::size_t ind = it - valsRes.begin();
        EXPECT_EQ(it.read(queue), valsExpected[ind]);
    }
}

TEST(Compute, MergeByPairKey) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<int> hostKeysA1{1, 3, 5, 5};
    std::vector<int> hostKeysA2{2, 5, 4, 6};
    std::vector<char> hostValsA{'o', 't', 'f', 'f'};

    std::vector<int> hostKeysB1{3, 3, 5, 6};
    std::vector<int> hostKeysB2{2, 6, 5, 0};
    std::vector<char> hostValsB{'t', 't', 'f', 's'};

    compute::vector<int> keysA1(hostKeysA1, queue);
    compute::vector<int> keysA2(hostKeysA2, queue);
    compute::vector<char> valsA(hostValsA, queue);
    compute::vector<int> keysB1(hostKeysB1, queue);
    compute::vector<int> keysB2(hostKeysB2, queue);
    compute::vector<char> valsB(hostValsB, queue);

    compute::vector<int> keysRes1(8, ctx);
    compute::vector<int> keysRes2(8, ctx);
    compute::vector<char> valsRes(8, ctx);

    auto [keysResEnd, valsResEnd] = spla::MergeByPairKey(
            keysA1.begin(), keysA2.begin(), keysA1.end(),
            valsA.begin(),
            keysB1.begin(), keysB2.begin(), keysB1.end(),
            valsB.begin(),
            keysRes1.begin(), keysRes2.begin(),
            valsRes.begin(),
            queue);

    std::vector<int> keys1Expected{1, 3, 3, 3, 5, 5, 5, 6};
    std::vector<int> keys2Expected{2, 2, 5, 6, 4, 5, 6, 0};
    std::vector<char> valsExpected{'o', 't', 't', 't', 'f', 'f', 'f', 's'};

    for (auto it = keysRes1.begin(); it < keysResEnd.first; ++it) {
        std::size_t ind = it - keysRes1.begin();
        EXPECT_EQ(it.read(queue), keys1Expected[ind]);
    }

    for (auto it = keysRes2.begin(); it < keysResEnd.second; ++it) {
        std::size_t ind = it - keysRes2.begin();
        EXPECT_EQ(it.read(queue), keys2Expected[ind]);
    }

    for (auto it = valsRes.begin(); it < valsResEnd; ++it) {
        std::size_t ind = it - valsRes.begin();
        EXPECT_EQ(it.read(queue), valsExpected[ind]);
    }
}

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
        EXPECT_EQ(expectedKeys[i], deviceKeys[i]);

    for (std::size_t i = 0; i < count; i++)
        EXPECT_EQ(expectedValues[i], deviceValues[i]);
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
        EXPECT_EQ(expectedKeys[i].get<0>(), ((Index) deviceKeys[i]).get<0>());
        EXPECT_EQ(expectedKeys[i].get<1>(), ((Index) deviceKeys[i]).get<1>());
    }

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedValues[i], deviceValues[i]);
        EXPECT_EQ(expectedValues[i], deviceValues[i]);
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
        EXPECT_EQ(expectedKeys1[i], deviceKeys1[i]);
        EXPECT_EQ(expectedKeys2[i], deviceKeys2[i]);
    }

    for (std::size_t i = 0; i < count; i++) {
        EXPECT_EQ(expectedValues[i], deviceValues[i]);
        EXPECT_EQ(expectedValues[i], deviceValues[i]);
    }
}

SPLA_GTEST_MAIN