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

    auto [keysResEndFirst, keysResEndSecond, valsResEnd] = spla::MergeByPairKey(
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

    for (auto it = keysRes1.begin(); it < keysResEndFirst; ++it) {
        std::size_t ind = it - keysRes1.begin();
        EXPECT_EQ(it.read(queue), keys1Expected[ind]);
    }

    for (auto it = keysRes2.begin(); it < keysResEndSecond; ++it) {
        std::size_t ind = it - keysRes2.begin();
        EXPECT_EQ(it.read(queue), keys2Expected[ind]);
    }

    for (auto it = valsRes.begin(); it < valsResEnd; ++it) {
        std::size_t ind = it - valsRes.begin();
        EXPECT_EQ(it.read(queue), valsExpected[ind]);
    }
}

TEST(Compute, MergeKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<int> hostKeysA{1, 2, 3, 4, 5};
    std::vector<int> hostKeysB{2, 2, 6, 6, 7};
    compute::vector<int> keysA(hostKeysA, queue);
    compute::vector<int> keysB(hostKeysB, queue);

    compute::vector<int> keysRes(10, ctx);

    auto keysResEnd = spla::MergeKeys(
            keysA.begin(), keysA.end(),
            keysB.begin(), keysB.end(),
            keysRes.begin(),
            queue);

    std::vector<int> keysExpected{1, 2, 2, 2, 3, 4, 5, 6, 6, 7};

    for (auto it = keysRes.begin(); it < keysResEnd; ++it) {
        std::size_t ind = it - keysRes.begin();
        EXPECT_EQ(it.read(queue), keysExpected[ind]);
    }
}

TEST(Compute, MergePairKeys) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::vector<int> hostKeysA1{1, 3, 5, 5, 7, 7, 7, 7, 9};
    std::vector<int> hostKeysA2{2, 5, 4, 6, 3, 3, 4, 6, 0};

    std::vector<int> hostKeysB1{3, 3, 5, 6, 6, 6, 6, 7, 10};
    std::vector<int> hostKeysB2{2, 6, 5, 0, 1, 2, 3, 5, 5};

    compute::vector<int> keysA1(hostKeysA1, queue);
    compute::vector<int> keysA2(hostKeysA2, queue);
    compute::vector<int> keysB1(hostKeysB1, queue);
    compute::vector<int> keysB2(hostKeysB2, queue);

    compute::vector<int> keysRes1(hostKeysA1.size() + hostKeysB1.size(), ctx);
    compute::vector<int> keysRes2(hostKeysA2.size() + hostKeysB2.size(), ctx);

    auto [keysResEndFirst, keysResEndSecond] = spla::MergePairKeys(
            keysA1.begin(), keysA2.begin(), keysA1.end(),
            keysB1.begin(), keysB2.begin(), keysB1.end(),
            keysRes1.begin(), keysRes2.begin(),
            queue);

    std::vector<int> keys1Expected{1, 3, 3, 3, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 9, 10};
    std::vector<int> keys2Expected{2, 2, 5, 6, 4, 5, 6, 0, 1, 2, 3, 3, 3, 4, 5, 6, 0, 5};

    for (auto it = keysRes1.begin(); it < keysResEndFirst; ++it) {
        std::size_t ind = it - keysRes1.begin();
        EXPECT_EQ(it.read(queue), keys1Expected[ind]);
    }

    for (auto it = keysRes2.begin(); it < keysResEndSecond; ++it) {
        std::size_t ind = it - keysRes2.begin();
        EXPECT_EQ(it.read(queue), keys2Expected[ind]);
    }
}

SPLA_GTEST_MAIN