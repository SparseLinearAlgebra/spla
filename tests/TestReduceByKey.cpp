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
#include <compute/SplaReduceByKey.hpp>

void TestReduceAlignedValuesByPairKey(
        const std::vector<std::uint32_t> &keys1,
        const std::vector<std::uint32_t> &keys2,
        const std::vector<std::uint8_t> &values,

        const std::vector<std::uint32_t> &keys1Expected_2,
        const std::vector<std::uint32_t> &keys2Expected_2,
        const std::vector<std::uint8_t> &valuesExpected_2,

        const std::vector<std::uint32_t> &keys1Expected_1,
        const std::vector<std::uint8_t> &valuesExpected_1) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    compute::vector<std::uint32_t> dKeys1(keys1, queue);
    compute::vector<std::uint32_t> dKeys2(keys2, queue);
    compute::vector<std::uint8_t> dValues(values, queue);

    std::string op = "_ACCESS_A const uchar* a = ((_ACCESS_A const uchar*)vp_a);"
                     "_ACCESS_B const uchar* b = ((_ACCESS_B const uchar*)vp_b);"
                     "_ACCESS_C uchar* c = (_ACCESS_C uchar*)vp_c;"
                     "c[0] = a[0] * b[0];"
                     "c[1] = a[1] + b[1];";

    {
        compute::vector<std::uint32_t> dKeysOut1(ctx);
        compute::vector<std::uint32_t> dKeysOut2(ctx);
        compute::vector<std::uint8_t> dValuesOut(ctx);

        std::size_t reducedSize = spla::ReduceByKey(
                dKeys1, dKeys2, dValues,
                dKeysOut1, dKeysOut2, dValuesOut,
                2, op, queue);

        std::vector<std::uint32_t> keys1Actual(reducedSize);
        std::vector<std::uint32_t> keys2Actual(reducedSize);
        std::vector<std::uint8_t> valuesActual(reducedSize * 2);

        compute::copy(dKeysOut1.begin(), dKeysOut1.end(), keys1Actual.begin(), queue);
        compute::copy(dKeysOut2.begin(), dKeysOut2.end(), keys2Actual.begin(), queue);
        compute::copy(dValuesOut.begin(), dValuesOut.end(), valuesActual.begin(), queue);

        queue.finish();

        EXPECT_EQ(keys1Expected_2, keys1Actual);
        EXPECT_EQ(keys2Expected_2, keys2Actual);
        EXPECT_EQ(valuesExpected_2, valuesActual);
    }

    {
        compute::vector<std::uint32_t> dKeysOut(ctx);
        compute::vector<std::uint8_t> dValuesOut(ctx);

        std::size_t reducedSize = spla::ReduceByKey(
                dKeys1, dValues,
                dKeysOut, dValuesOut,
                2, op, queue);

        std::vector<std::uint32_t> keysActual(reducedSize);
        std::vector<std::uint8_t> valuesActual(reducedSize * 2);

        compute::copy(dKeysOut.begin(), dKeysOut.end(), keysActual.begin(), queue);
        compute::copy(dValuesOut.begin(), dValuesOut.end(), valuesActual.begin(), queue);

        queue.finish();

        EXPECT_EQ(keys1Expected_1, keysActual);
        EXPECT_EQ(valuesExpected_1, valuesActual);
    }
}

TEST(ReduceByKey, ByPairKeyBasic) {
    // clang-format off
    std::vector<std::uint32_t> keys1 = {1,    2,    2,    4,    5,    5,    7,    8,    8,    8};
    std::vector<std::uint32_t> keys2 = {0,    1,    4,    2,    2,    2,    2,    2,    2,    3};
    std::vector<std::uint8_t> values = {0, 1, 6, 2, 6, 0, 2, 5, 2, 2, 6, 7, 9, 5, 7, 8, 3, 3, 7, 5};
    //                                                          *     *           *     *
    //                                  1      2     2     4    5           7     8           8
    //                                  0      1     4     2    2           2     2           3
    //                                  0  1   6  2  6  0  2  5 12 9        9  5  21 11        7 5
    //
    //                                         *     *          *     *           *     *     *
    //                                  1      2           4    5           7     8
    //                                  0  1   36 2        2 5  12 9        9 5   147 16
    // clang-format on

    std::vector<std::uint32_t> keys1Expected_2 = {1, 2, 2, 4, 5, 7, 8, 8};
    std::vector<std::uint32_t> keys2Expected_2 = {0, 1, 4, 2, 2, 2, 2, 3};
    std::vector<std::uint8_t> valuesExpected_2 = {0, 1, 6, 2, 6, 0, 2, 5, 12, 9, 9, 5, 21, 11, 7, 5};

    std::vector<std::uint32_t> keysExpected_1 = {1, 2, 4, 5, 7, 8};
    std::vector<std::uint8_t> valuesExpected_1 = {0, 1, 36, 2, 2, 5, 12, 9, 9, 5, 147, 16};

    TestReduceAlignedValuesByPairKey(
            keys1,
            keys2,
            values,
            keys1Expected_2,
            keys2Expected_2,
            valuesExpected_2,
            keysExpected_1,
            valuesExpected_1);
}

TEST(ReduceByKey, PairKeyEmpty) {
    TestReduceAlignedValuesByPairKey(
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {});
}

TEST(ReduceByKey, PairKeyOne) {
    TestReduceAlignedValuesByPairKey(
            {1},
            {2},
            {1, 2},
            {1},
            {2},
            {1, 2},
            {1},
            {1, 2});
}

TEST(ReduceByKey, PairKeyTwo) {
    TestReduceAlignedValuesByPairKey(
            {1, 1},
            {2, 2},
            {1, 2, 3, 4},
            {1},
            {2},
            {3, 6},
            {1},
            {3, 6});
}

TEST(ReduceByKey, PairKeyTwoNotReduceOneKey) {
    TestReduceAlignedValuesByPairKey(
            {1, 1},
            {2, 0},
            {1, 2, 3, 4},
            {1, 1},
            {2, 0},
            {1, 2, 3, 4},
            {1},
            {3, 6});
}

TEST(ReduceByKey, PairKeySeveralSimilar) {
    TestReduceAlignedValuesByPairKey(
            {1, 1, 1, 1},
            {2, 2, 3, 3},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1},
            {2, 3},
            {1, 2, 1, 2},
            {1},
            {1, 4});
}

SPLA_GTEST_MAIN
