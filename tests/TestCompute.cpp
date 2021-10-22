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
