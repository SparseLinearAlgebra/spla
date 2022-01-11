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

#define SPLA_TEST_REDUCE
#include <compute/SplaReduce.hpp>
#include <compute/SplaReduce2.hpp>

void TestReduceAlignedValues(
        const std::vector<unsigned char> &values,
        const std::vector<unsigned char> &valuesExpected) {
    namespace compute = boost::compute;

    compute::context ctx(compute::system::devices()[0]);
    compute::command_queue queue(ctx, ctx.get_device());

    compute::vector<std::uint8_t> dValues(values, queue);

    std::string op = "_ACCESS_A const uchar* a = ((_ACCESS_A const uchar*)vp_a);"
                     "_ACCESS_B const uchar* b = ((_ACCESS_B const uchar*)vp_b);"
                     "_ACCESS_C uchar* c = (_ACCESS_C uchar*)vp_c;"
                     "c[0] = a[0] * b[0];"
                     "c[1] = a[1] + b[1];";

    compute::vector<std::uint8_t> dValuesOut(ctx);
    compute::vector<std::uint8_t> reduced = spla::Reduce(dValues, 2, op, queue);
    std::vector<std::uint8_t> reducedActual(2);
    compute::copy(reduced.begin(), reduced.end(), reducedActual.begin(), queue);

    queue.finish();

    EXPECT_EQ(valuesExpected, reducedActual);
}

TEST(Reduce, Basic) {
    TestReduceAlignedValues({2, 0, 2, 1, 3, 3, 1, 2}, {12, 6});
}

TEST(Reduce, Singleton) {
    TestReduceAlignedValues({2, 0}, {2, 0});
}

TEST(Reduce, Binary) {
    TestReduceAlignedValues({2, 0, 4, 2}, {8, 2});
}

TEST(Reduce, Empty) {
    TestReduceAlignedValues({}, {0, 0});
}

template<typename T, std::size_t ValueSize = sizeof(T), typename Reduce>
std::array<unsigned char, ValueSize> ReduceCpu(const std::vector<unsigned char> &values, Reduce f) {
    std::array<unsigned char, ValueSize> result{};
    if (values.empty()) {
        std::fill(result.begin(), result.end(), 0);
        return result;
    }
    std::memcpy(result.data(), values.data(), ValueSize);
    std::size_t nVals = values.size() / ValueSize;
    for (std::size_t i = 1; i < nVals; ++i) {
        T left, right;
        std::memcpy(&left, result.data(), ValueSize);
        std::memcpy(&right, values.data() + i * ValueSize, ValueSize);
        T ap = f(left, right);
        std::memcpy(result.data(), &ap, ValueSize);
    }
    return result;
}

template<typename ComputeReduce>
void ReduceStress(std::size_t n, std::size_t seed, ComputeReduce computeReduce) {
    using T = std::int32_t;
    constexpr std::size_t valueByteSize = sizeof(T);

    namespace compute = boost::compute;

    compute::context ctx(compute::system::devices()[0]);
    compute::command_queue queue(ctx, ctx.get_device());

    std::default_random_engine rand(seed);

    std::vector<unsigned char> values(sizeof(T) * n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < sizeof(T); ++j) {
            values[i * sizeof(T) + j] = rand();
        }
    }

    std::string op = "_ACCESS_A const uint* a = ((_ACCESS_A const uint*)vp_a);"
                     "_ACCESS_B const uint* b = ((_ACCESS_B const uint*)vp_b);"
                     "_ACCESS_C uint* c = (_ACCESS_C uint*)vp_c;"
                     "*c = *a + *b;";

    compute::vector<std::uint8_t> dValues(values, queue);

    compute::vector<std::uint8_t> reduced = computeReduce(dValues, sizeof(T), op, queue);
    queue.finish();

    std::array<unsigned char, sizeof(T)> reducedActual{};
    compute::copy(reduced.begin(), reduced.end(), reducedActual.begin(), queue);
    std::array<unsigned char, sizeof(T)> reducedExpected = ReduceCpu<T>(values, [](T a, T b) { return a + b; });

    T reducedNumberExpected = *reducedExpected.data();
    T reducedNumberActual = *reducedActual.data();

    EXPECT_EQ(reducedNumberExpected, reducedNumberActual);
    EXPECT_EQ(reducedExpected, reducedActual);
}

template<typename ComputeReduce>
void ReduceStressSeries(std::size_t iterations, std::size_t n, ComputeReduce reduce) {
    for (std::size_t i = 0; i < iterations; ++i) {
        ReduceStress(n, i, reduce);
    }
}

TEST(ReduceStress, Small) {
    ReduceStressSeries(60, 500, spla::Reduce);
}

TEST(ReduceStress, Medium) {
    ReduceStressSeries(50, 10000, spla::Reduce);
}

TEST(ReduceStress, Large) {
    ReduceStressSeries(10, 100000, spla::Reduce);
}

TEST(Reduce2Stress, Small) {
    ReduceStressSeries(60, 500, spla::Reduce2);
}

TEST(Reduce2Stress, Medium) {
    ReduceStressSeries(50, 10000, spla::Reduce2);
}

TEST(Reduce2Stress, Large) {
    ReduceStressSeries(10, 100000, spla::Reduce2);
}

SPLA_GTEST_MAIN
