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
#include <boost/compute.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

TEST(Basic, BoostExample) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    std::default_random_engine engine(std::chrono::system_clock::now().time_since_epoch().count());
    auto dist = std::uniform_real_distribution<float>(0.0, 100.0);

    // generate random numbers on the host
    std::vector<float> host_vector(1000000);
    std::generate(host_vector.begin(), host_vector.end(), [&](){ return dist(engine); });

    // create vector on the device
    compute::vector<float> device_vector(1000000, ctx);

    // copy data to the device
    compute::copy(host_vector.begin(), host_vector.end(), device_vector.begin(), queue);

    // sort data on the device
    compute::sort(device_vector.begin(), device_vector.end(), queue);

    // copy data back to the host
    compute::copy(device_vector.begin(), device_vector.end(), host_vector.begin(), queue);

    // Ensure, that data is sorted
    float prev = -1;
    for (auto v: host_vector) {
        EXPECT_LE(prev, v);
        prev = v;
    }
}

SPLA_GTEST_MAIN