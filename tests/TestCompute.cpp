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
#include <compute/SplaReduceAlignedValuesByKey.hpp>

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

TEST(Compute, ReduceAlignedValuesByPairKey) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    // clang-format off
    std::vector<std::uint32_t> keys1 = {1,    2,    2,    4,    5,    5,    7,    8,    8,    8};
    std::vector<std::uint32_t> keys2 = {0,    1,    4,    2,    2,    2,    2,    2,    2,    3};
    std::vector<std::uint8_t> values = {0, 1, 6, 2, 6, 0, 2, 5, 2, 2, 6, 7, 9, 5, 7, 8, 3, 3, 7, 5};
    //                                                          *     *           *     *
    //                                  1      2     2     4    5           7     8           8
    //                                  0      1     4     2    2           2     2           3
    //                                  0  1   6  2  6  0  2  5 12 20       9  5  21 30       7 5
    // clang-format on

    compute::vector<std::uint32_t> dKeys1(keys1, queue);
    compute::vector<std::uint32_t> dKeys2(keys2, queue);
    compute::vector<std::uint8_t> dValues(values, queue);

    using Key = std::pair<std::uint32_t, std::uint32_t>;

    BOOST_COMPUTE_FUNCTION(bool, CompareKeys, (Key a, Key b),
                           { return (a.first == b.first) && (a.second == b.second); });

    auto ReduceValues =
            boost::compute::make_function_from_source<void(std::uint32_t *, std::uint32_t *, std::uint32_t *)>(
                    "ReduceValues",
                    "void ReduceValues(__global uchar * ac, __global uchar *cur, uchar *res) { res[0] = ac[0] * cur[0]; res[1] = ac[1] * 3 + cur[1] * 2; }");

    compute::vector<std::uint32_t> dKeysOut1(keys1.size(), ctx);
    compute::vector<std::uint32_t> dKeysOut2(keys2.size(), ctx);
    compute::vector<std::uint8_t> dValuesOut(values.size(), ctx);

    auto [keys1EndIt, keys2EndIt, valuesEndIt] = spla::ReduceAlignedValuesByPairKey(
            dKeys1.begin(), dKeys1.end(), dKeys2.begin(),
            dValues.begin(), 2,
            dKeysOut1.begin(), dKeysOut2.begin(),
            dValuesOut.begin(),
            ReduceValues,
            CompareKeys,
            queue);

    std::vector<std::uint32_t> keys1Expected = {1, 2, 2, 4, 5, 7, 8, 8};
    std::vector<std::uint32_t> keys2Expected = {0, 1, 4, 2, 2, 2, 2, 3};
    std::vector<std::uint8_t> valuesExpected = {0, 1, 6, 2, 6, 0, 2, 5, 12, 20, 9, 5, 21, 30, 7, 5};

    std::vector<std::uint32_t> keys1Actual(std::distance(dKeysOut1.begin(), keys1EndIt));
    std::vector<std::uint32_t> keys2Actual(std::distance(dKeysOut2.begin(), keys2EndIt));
    std::vector<std::uint8_t> valuesActual(std::distance(dValuesOut.begin(), valuesEndIt));

    compute::copy(dKeysOut1.begin(), keys1EndIt, keys1Actual.begin(), queue);
    compute::copy(dKeysOut2.begin(), keys2EndIt, keys2Actual.begin(), queue);
    compute::copy(dValuesOut.begin(), valuesEndIt, valuesActual.begin(), queue);

    EXPECT_EQ(keys1Expected, keys1Actual);
    EXPECT_EQ(keys2Expected, keys2Actual);
    EXPECT_EQ(valuesExpected, valuesActual);
}

SPLA_GTEST_MAIN