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

    std::ptrdiff_t mergedSize = spla::MergeByKey(
            keysA.begin(), keysA.end(),
            valsA.begin(),
            keysB.begin(), keysB.end(),
            valsB.begin(),
            keysRes.begin(),
            valsRes.begin(),
            queue);

    std::vector<int> keysExpected{1, 2, 2, 2, 3, 4, 5, 6, 6, 7};
    std::vector<char> valsExpected{'o', 't', 't', 't', 't', 'f', 'f', 's', 's', 's'};

    for (auto it = keysRes.begin(); it < keysRes.begin() + mergedSize; ++it) {
        std::size_t ind = it - keysRes.begin();
        EXPECT_EQ(it.read(queue), keysExpected[ind]);
    }

    for (auto it = valsRes.begin(); it < valsRes.begin() + mergedSize; ++it) {
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

    std::vector<int> hostKeysB1{1, 3, 3, 5, 6};
    std::vector<int> hostKeysB2{2, 2, 6, 5, 0};
    std::vector<char> hostValsB{'k', 't', 't', 'f', 's'};

    compute::vector<int> keysA1(hostKeysA1, queue);
    compute::vector<int> keysA2(hostKeysA2, queue);
    compute::vector<char> valsA(hostValsA, queue);
    compute::vector<int> keysB1(hostKeysB1, queue);
    compute::vector<int> keysB2(hostKeysB2, queue);
    compute::vector<char> valsB(hostValsB, queue);

    auto resCount = hostKeysA1.size() + hostKeysB1.size();

    compute::vector<int> keysRes1(resCount, ctx);
    compute::vector<int> keysRes2(resCount, ctx);
    compute::vector<char> valsRes(resCount, ctx);

    std::ptrdiff_t mergedSize = spla::MergeByPairKey(
            keysA1.begin(), keysA1.end(), keysA2.begin(),
            valsA.begin(),
            keysB1.begin(), keysB1.end(), keysB2.begin(),
            valsB.begin(),
            keysRes1.begin(), keysRes2.begin(),
            valsRes.begin(),
            queue);

    std::vector<int> keys1Expected{1, 1, 3, 3, 3, 5, 5, 5, 6};
    std::vector<int> keys2Expected{2, 2, 2, 5, 6, 4, 5, 6, 0};
    std::vector<char> valsExpected{'o', 'k', 't', 't', 't', 'f', 'f', 'f', 's'};

    for (auto it = keysRes1.begin(); it < keysRes1.begin() + mergedSize; ++it) {
        std::size_t ind = it - keysRes1.begin();
        EXPECT_EQ(it.read(queue), keys1Expected[ind]);
    }

    for (auto it = keysRes2.begin(); it < keysRes2.begin() + mergedSize; ++it) {
        std::size_t ind = it - keysRes2.begin();
        EXPECT_EQ(it.read(queue), keys2Expected[ind]);
    }

    for (auto it = valsRes.begin(); it < valsRes.begin() + mergedSize; ++it) {
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

    std::ptrdiff_t mergedSize = spla::MergeKeys(
            keysA.begin(), keysA.end(),
            keysB.begin(), keysB.end(),
            keysRes.begin(),
            queue);

    std::vector<int> keysExpected{1, 2, 2, 2, 3, 4, 5, 6, 6, 7};

    for (auto it = keysRes.begin(); it < keysRes.begin() + mergedSize; ++it) {
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

    std::vector<int> hostKeysB1{1, 3, 3, 5, 6, 6, 6, 6, 7, 10};
    std::vector<int> hostKeysB2{2, 2, 6, 5, 0, 1, 2, 3, 5, 5};

    compute::vector<int> keysA1(hostKeysA1, queue);
    compute::vector<int> keysA2(hostKeysA2, queue);
    compute::vector<int> keysB1(hostKeysB1, queue);
    compute::vector<int> keysB2(hostKeysB2, queue);

    compute::vector<int> keysRes1(hostKeysA1.size() + hostKeysB1.size(), ctx);
    compute::vector<int> keysRes2(hostKeysA2.size() + hostKeysB2.size(), ctx);

    std::ptrdiff_t mergedSize = spla::MergePairKeys(
            keysA1.begin(), keysA1.end(), keysA2.begin(),
            keysB1.begin(), keysB1.end(), keysB2.begin(),
            keysRes1.begin(), keysRes2.begin(),
            queue);

    std::vector<int> keys1Expected{1, 1, 3, 3, 3, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 9, 10};
    std::vector<int> keys2Expected{2, 2, 2, 5, 6, 4, 5, 6, 0, 1, 2, 3, 3, 3, 4, 5, 6, 0, 5};

    for (auto it = keysRes1.begin(); it < keysRes1.begin() + mergedSize; ++it) {
        std::size_t ind = it - keysRes1.begin();
        EXPECT_EQ(it.read(queue), keys1Expected[ind]);
    }

    for (auto it = keysRes2.begin(); it < keysRes2.begin() + mergedSize; ++it) {
        std::size_t ind = it - keysRes2.begin();
        EXPECT_EQ(it.read(queue), keys2Expected[ind]);
    }
}

std::pair<std::vector<int>, std::vector<int>> MergeByKeyCpu(
        const std::vector<int> &k1,
        const std::vector<int> &v1,
        const std::vector<int> &k2,
        const std::vector<int> &v2) {
    EXPECT_EQ(k1.size(), v1.size());
    EXPECT_EQ(k2.size(), v2.size());

    const std::size_t mergedSize = k1.size() + k2.size();

    std::vector<int> kRes(mergedSize), vRes(mergedSize);

    std::vector<std::pair<int, int>> zipped1(k1.size());
    for (std::size_t i = 0; i < k1.size(); ++i) {
        zipped1[i] = {k1[i], v1[i]};
    }

    std::vector<std::pair<int, int>> zipped2(k2.size());
    for (std::size_t i = 0; i < k2.size(); ++i) {
        zipped2[i] = {k2[i], v2[i]};
    }

    std::vector<std::pair<int, int>> zippedR(mergedSize);
    std::merge(zipped1.begin(), zipped1.end(), zipped2.begin(), zipped2.end(), zippedR.begin());

    for (std::size_t i = 0; i < mergedSize; ++i) {
        kRes[i] = zippedR[i].first;
        vRes[i] = zippedR[i].second;
    }
    return {kRes, vRes};
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> MergeByKeyPairCpu(
        const std::vector<int> &k1f,
        const std::vector<int> &k1s,
        const std::vector<int> &v1,
        const std::vector<int> &k2f,
        const std::vector<int> &k2s,
        const std::vector<int> &v2) {
    EXPECT_EQ(k1f.size(), v1.size());
    EXPECT_EQ(k1s.size(), v1.size());
    EXPECT_EQ(k2f.size(), v2.size());
    EXPECT_EQ(k2s.size(), v2.size());

    const std::size_t mergedSize = k1f.size() + k2f.size();

    std::vector<int> kfRes(mergedSize), ksRes(mergedSize), vRes(mergedSize);

    std::vector<std::tuple<int, int, int>> zipped1(k1f.size());
    for (std::size_t i = 0; i < k1f.size(); ++i) {
        zipped1[i] = {k1f[i], k1s[i], v1[i]};
    }

    std::vector<std::tuple<int, int, int>> zipped2(k2f.size());
    for (std::size_t i = 0; i < k2f.size(); ++i) {
        zipped2[i] = {k2f[i], k2s[i], v2[i]};
    }

    std::vector<std::tuple<int, int, int>> zippedR(mergedSize);
    std::merge(zipped1.begin(), zipped1.end(), zipped2.begin(), zipped2.end(), zippedR.begin());

    for (std::size_t i = 0; i < mergedSize; ++i) {
        kfRes[i] = std::get<0>(zippedR[i]);
        ksRes[i] = std::get<1>(zippedR[i]);
        vRes[i] = std::get<2>(zippedR[i]);
    }
    return {kfRes, ksRes, vRes};
}

void MergeByKeyStress(
        std::size_t iterations,
        std::size_t aSize,
        std::size_t bSize) {
    namespace compute = boost::compute;

    for (std::size_t testIt = 0; testIt < iterations; ++testIt) {
        // get the default compute device
        compute::device gpu = compute::system::default_device();

        // create a compute context and command queue
        compute::context ctx(gpu);
        compute::command_queue queue(ctx, gpu);

        std::vector<int> k1 = utils::GenerateIntVector<int>(aSize, testIt * 4);
        std::vector<int> v1 = utils::GenerateIntVector<int>(aSize, testIt * 4 + 1);
        std::vector<int> k2 = utils::GenerateIntVector<int>(bSize, testIt * 4 + 2);
        std::vector<int> v2 = utils::GenerateIntVector<int>(bSize, testIt * 4 + 3);

        std::sort(k1.begin(), k1.end());
        std::sort(k2.begin(), k2.end());

        auto [expectedKeyRes, expectedValRes] = MergeByKeyCpu(k1, v1, k2, v2);

        compute::vector<int> keysA(k1, queue);
        compute::vector<int> valsA(v1, queue);
        compute::vector<int> keysB(k2, queue);
        compute::vector<int> valsB(v2, queue);

        const std::size_t sizeSum = aSize + bSize;
        compute::vector<int> keysRes(sizeSum, ctx);
        compute::vector<int> valsRes(sizeSum, ctx);

        const std::ptrdiff_t mergedSize = spla::MergeByKey(
                keysA.begin(), keysA.end(),
                valsA.begin(),
                keysB.begin(), keysB.end(),
                valsB.begin(),
                keysRes.begin(),
                valsRes.begin(),
                queue);

        for (auto it = keysRes.begin(); it < keysRes.begin() + mergedSize; ++it) {
            std::size_t ind = it - keysRes.begin();
            EXPECT_EQ(it.read(queue), expectedKeyRes[ind]);
        }

        for (auto it = valsRes.begin(); it < valsRes.begin() + mergedSize; ++it) {
            std::size_t ind = it - valsRes.begin();
            EXPECT_EQ(it.read(queue), expectedValRes[ind]);
        }
    }
}

void MergeByKeyPairStress(
        std::size_t iterations,
        std::size_t aSize,
        std::size_t bSize) {
    namespace compute = boost::compute;

    for (std::size_t testIt = 0; testIt < iterations; ++testIt) {
        // get the default compute device
        compute::device gpu = compute::system::default_device();

        // create a compute context and command queue
        compute::context ctx(gpu);
        compute::command_queue queue(ctx, gpu);

        std::vector<int> k1f = utils::GenerateIntVector<int>(aSize, testIt * 6);
        std::vector<int> k1s = utils::GenerateIntVector<int>(aSize, testIt * 6 + 1);
        std::vector<int> v1 = utils::GenerateIntVector<int>(aSize, testIt * 6 + 2);
        std::vector<int> k2f = utils::GenerateIntVector<int>(bSize, testIt * 6 + 3);
        std::vector<int> k2s = utils::GenerateIntVector<int>(bSize, testIt * 6 + 4);
        std::vector<int> v2 = utils::GenerateIntVector<int>(bSize, testIt * 6 + 5);

        {
            std::vector<std::pair<int, int>> k1(aSize);
            std::vector<std::pair<int, int>> k2(bSize);

            for (std::size_t i = 0; i < aSize; ++i) {
                k1[i] = {k1f[i], k1s[i]};
            }
            for (std::size_t i = 0; i < bSize; ++i) {
                k2[i] = {k2f[i], k2s[i]};
            }

            std::sort(k1.begin(), k1.end());
            std::sort(k2.begin(), k2.end());

            for (std::size_t i = 0; i < aSize; ++i) {
                std::tie(k1f[i], k1s[i]) = k1[i];
            }
            for (std::size_t i = 0; i < bSize; ++i) {
                std::tie(k2f[i], k2s[i]) = k2[i];
            }
        }

        auto [expectedKeyFRes, expectedKeySRes, expectedValRes] = MergeByKeyPairCpu(k1f, k1s, v1, k2f, k2s, v2);

        compute::vector<int> keysAf(k1f, queue);
        compute::vector<int> keysAs(k1s, queue);
        compute::vector<int> valsA(v1, queue);
        compute::vector<int> keysBf(k2f, queue);
        compute::vector<int> keysBs(k2s, queue);
        compute::vector<int> valsB(v2, queue);

        const std::size_t sizeSum = aSize + bSize;
        compute::vector<int> keysFRes(sizeSum, ctx);
        compute::vector<int> keysSRes(sizeSum, ctx);
        compute::vector<int> valsRes(sizeSum, ctx);

        const std::ptrdiff_t mergedSize = spla::MergeByPairKey(
                keysAf.begin(), keysAf.end(), keysAs.begin(), valsA.begin(),
                keysBf.begin(), keysBf.end(), keysBs.begin(), valsB.begin(),
                keysFRes.begin(), keysSRes.begin(),
                valsRes.begin(),
                queue);

        for (auto it = keysFRes.begin(); it < keysFRes.begin() + mergedSize; ++it) {
            std::size_t ind = it - keysFRes.begin();
            EXPECT_EQ(it.read(queue), expectedKeyFRes[ind]);
        }

        for (auto it = keysSRes.begin(); it < keysSRes.begin() + mergedSize; ++it) {
            std::size_t ind = it - keysSRes.begin();
            EXPECT_EQ(it.read(queue), expectedKeySRes[ind]);
        }

        for (auto it = valsRes.begin(); it < valsRes.begin() + mergedSize; ++it) {
            std::size_t ind = it - valsRes.begin();
            EXPECT_EQ(it.read(queue), expectedValRes[ind]);
        }
    }
}

TEST(MergeByKey, StressSmall) {
    MergeByKeyStress(100, 55, 34);
    MergeByKeyPairStress(100, 55, 34);
}

TEST(MergeByKey, StressMedium) {
    MergeByKeyStress(5, 5000, 4000);
    MergeByKeyPairStress(5, 5000, 4000);
}

SPLA_GTEST_MAIN
