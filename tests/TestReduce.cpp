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
#include <compute/SplaReduce.hpp>

void TestReduceAlignedValues(
        const std::vector<std::uint8_t> &values,
        const std::vector<std::uint8_t> &valuesExpected) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    compute::vector<std::uint8_t> dValues(values, queue);

    std::string op = "_ACCESS_A const uchar* a = ((_ACCESS_A const uchar*)vp_a);"
                     "_ACCESS_B const uchar* b = ((_ACCESS_B const uchar*)vp_b);"
                     "_ACCESS_C uchar* c = (_ACCESS_C uchar*)vp_c;"
                     "c[0] = a[0] * b[0];"
                     "c[1] = a[1] + b[1];";

    compute::vector<std::uint8_t> dValuesOut(ctx);
    compute::vector<std::uint8_t> reduced = spla::Reduce(values, 2, op, queue);
    std::vector<std::uint8_t> reducedActual(2);
    compute::copy(reduced.begin(), reduced.end(), reducedActual.begin(), queue);

    queue.finish();

    EXPECT_EQ(valuesExpected, reducedActual);
}

TEST(ReduceByKey, Basic) {
    TestReduceAlignedValues({2, 0, 2, 1, 3, 3, 1, 2}, {12, 6});
}

TEST(ReduceByKey, Singleton) {
    TestReduceAlignedValues({2, 0}, {2, 0});
}

TEST(ReduceByKey, Binary) {
    TestReduceAlignedValues({2, 0, 4, 2}, {8, 2});
}

TEST(ReduceByKey, Empty) {
    TestReduceAlignedValues({}, {0, 0});
}

SPLA_GTEST_MAIN
