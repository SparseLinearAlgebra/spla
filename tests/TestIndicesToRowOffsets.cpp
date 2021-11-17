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
#include <compute/SplaIndicesToRowOffsets.hpp>

void test(std::size_t n,
          const std::vector<unsigned int> &indices,
          const std::vector<unsigned int> &offsets,
          const std::vector<unsigned int> &lengths) {
    using namespace boost;
    auto ctx = compute::system::default_context();
    auto queue = compute::system::default_queue();

    compute::vector<unsigned int> deviceIndices(indices.size(), ctx);
    compute::vector<unsigned int> deviceOffsets(ctx);
    compute::vector<unsigned int> deviceLengths(ctx);

    compute::copy(indices.begin(), indices.end(), deviceIndices.begin(), queue);
    spla::IndicesToRowOffsets(deviceIndices, deviceOffsets, deviceLengths, n, queue);

    queue.finish();

    ASSERT_EQ(offsets.size(), deviceOffsets.size());
    ASSERT_EQ(lengths.size(), deviceLengths.size());

    for (std::size_t i = 0; i < offsets.size(); i++)
        EXPECT_EQ(offsets[i], (deviceOffsets.begin() + i).read(queue));

    for (std::size_t i = 0; i < lengths.size(); i++)
        EXPECT_EQ(lengths[i], (deviceLengths.begin() + i).read(queue));
}

TEST(IndicesToRowOffsets, ZeroDim) {
    test(0, {}, {0}, {0});
}

TEST(IndicesToRowOffsets, Empty) {
    test(10, {}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}

TEST(IndicesToRowOffsets, Sequence) {
    test(10, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0});
}

TEST(IndicesToRowOffsets, Generic) {
    test(5, {0, 0, 1, 2, 3, 4, 4, 4}, {0, 2, 3, 4, 5, 8}, {2, 1, 1, 1, 3, 0});
}

SPLA_GTEST_MAIN