/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#include "test_common.hpp"

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION  120
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>

#include <iostream>
#include <thread>
#include <vector>

TEST(opencl, basic_gpu) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    std::vector<int> vec_a = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> vec_b = {0, -1, 2, -3, 4, -5, 6, -7, 8, -9};
    std::vector<int> vec_c(10);

    cl::Buffer a(queue, vec_a.begin(), vec_a.end(), true, false);
    cl::Buffer b(queue, vec_b.begin(), vec_b.end(), true, false);
    cl::Buffer c(context, CL_MEM_READ_WRITE, sizeof(int) * vec_c.size());

    std::string kernel_code =
            "__kernel void add(__global const int* a, __global const int* b, __global int* c, int count) { "
            "   size_t idx = get_global_id(0); "
            "   if (idx < count) { c[idx] = a[idx] + b[idx]; } "
            "}";

    cl::Program program(context, kernel_code);
    program.build(device, "-cl-std=CL1.2");

    cl::Kernel kernel(program, "add");
    kernel.setArg(0, a);
    kernel.setArg(1, b);
    kernel.setArg(2, c);
    kernel.setArg(3, 10);

    cl::NDRange global(32);
    cl::NDRange local(32);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);

    cl::copy(queue, c, vec_c.begin(), vec_c.end());

    for (auto value : vec_c)
        std::cout << value << std::endl;
}

#include "../src/opencl/generated/auto_sort_bitonic.hpp"

TEST(opencl, bitonic_sort_local) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    const int                 N = 2000;
    std::vector<unsigned int> keys(N);
    std::vector<int>          values(N);

    for (int i = 0; i < N; ++i) {
        keys[i]   = N - i;
        values[i] = ((N - i) % 2) * 10;
    }

    cl::Buffer buffer_keys(queue, keys.begin(), keys.end(), false, false);
    cl::Buffer buffer_values(queue, values.begin(), values.end(), false, false);

    std::string kernel_code = source_sort_bitonic;

    cl::Program program(context, kernel_code);
    program.build(device, "-cl-std=CL1.2 -DBLOCK_SIZE=2048 -DTYPE=int");

    cl::Kernel kernel(program, "bitonic_sort_local");
    kernel.setArg(0, buffer_keys);
    kernel.setArg(1, buffer_values);
    kernel.setArg(2, N);

    cl::NDRange global(256);
    cl::NDRange local(256);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);

    cl::copy(queue, buffer_keys, keys.begin(), keys.end());
    cl::copy(queue, buffer_values, values.begin(), values.end());

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(keys[i], i + 1);
    }

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(values[i], ((i + 1) % 2) * 10);
    }
}

TEST(opencl, bitonic_sort_global) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    const int                 N = 100000;
    std::vector<unsigned int> keys(N);
    std::vector<int>          values(N);

    for (int i = 0; i < N; ++i) {
        keys[i]   = N - i;
        values[i] = ((N - i) % 2) * 10;
    }

    cl::Buffer buffer_keys(queue, keys.begin(), keys.end(), false, false);
    cl::Buffer buffer_values(queue, values.begin(), values.end(), false, false);

    std::string kernel_code = source_sort_bitonic;

    cl::Program program(context, kernel_code);
    program.build(device, "-cl-std=CL1.2 -DBLOCK_SIZE=2048 -DTYPE=int");

    cl::Kernel kernel(program, "bitonic_sort_global");
    kernel.setArg(0, buffer_keys);
    kernel.setArg(1, buffer_values);
    kernel.setArg(2, N);
    kernel.setArg(3, 2);

    cl::NDRange global(256);
    cl::NDRange local(256);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(), global, local);

    cl::copy(queue, buffer_keys, keys.begin(), keys.end());
    cl::copy(queue, buffer_values, values.begin(), values.end());

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(keys[i], i + 1);
    }

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(values[i], ((i + 1) % 2) * 10);
    }
}

TEST(opencl, custom_value) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    struct f2 {
        float x;
        float y;
    };

    f2 init{1.0f, 2.0f};

    const int       N = 64;
    std::vector<f2> host(N);
    cl::Buffer      buffer(context, CL_MEM_WRITE_ONLY, sizeof(f2) * N);

    std::string kernel_code =
            "struct f2 {\n"
            "        float x;\n"
            "        float y;\n"
            "};"
            ""
            "__kernel void fill(__global struct f2* buffer, struct f2 value) {"
            "  buffer[get_global_id(0)] = value;"
            "}";

    cl::Program program(context, kernel_code);
    program.build(device, "-cl-std=CL1.2");

    cl::Kernel kernel(program, "fill");
    kernel.setArg(0, buffer);
    kernel.setArg(1, init);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(N), cl::NDRange(32));
    queue.enqueueReadBuffer(buffer, true, 0, sizeof(f2) * N, host.data());

    for (auto& v : host) {
        EXPECT_EQ(v.x, init.x);
        EXPECT_EQ(v.y, init.y);
    }
}

TEST(opencl, reduce_by_key_small) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    cl::Context      context(device);
    cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);

    std::string kernel_code =
            R"( uint ceil_to_pow2(uint n) {
                    uint r = 1;
                    while (r < n) r *= 2;
                    return r;
                }
                uint lower_bound_local(const uint          x,
                                       uint                first,
                                       uint                size,
                                       __local const uint* array) {
                    while (size > 0) {
                        int step = size / 2;

                        if (array[first + step] < x) {
                            first = first + step + 1;
                            size -= step + 1;
                        } else {
                            size = step;
                        }
                    }
                    return first;
                }
                __kernel void reduce_by_key_small(__global const uint* g_keys,
                                                 __global const int*  g_values,
                                                 __global uint*       g_unique_keys,
                                                 __global int*        g_reduce_values,
                                                 __global uint*       g_reduced_count,
                                                 const uint           n_keys) {
                    const uint lid       = get_local_id(0);
                    const uint n_aligned = ceil_to_pow2(n_keys);

                    __local uint s_offsets[1024];

                    uint gen_key = 0;
                    if (lid < n_keys) {
                        bool is_neq   = lid > 0 && g_keys[lid] != g_keys[lid - 1];
                        bool is_first = lid == 0;
                        gen_key       = is_neq || is_first ? 1 : 0;
                    }
                    s_offsets[lid] = gen_key;

                    for (uint offset = 1; offset < n_aligned; offset *= 2) {
                        barrier(CLK_LOCAL_MEM_FENCE);
                        uint value = s_offsets[lid];

                        if (offset <= lid) {
                            value += s_offsets[lid - offset];
                        }

                        barrier(CLK_LOCAL_MEM_FENCE);
                        s_offsets[lid] = value;
                    }

                    barrier(CLK_LOCAL_MEM_FENCE);
                    const uint n_values = s_offsets[n_keys - 1];

                    if (lid < n_values) {
                        const uint id        = lid + 1;
                        const uint start_idx = lower_bound_local(id, 0, n_keys, s_offsets);
                        int        value     = g_values[start_idx];

                        for (uint i = start_idx + 1; i < n_keys && id == s_offsets[i]; i += 1) {
                            value = value | g_values[i];
                        }

                        g_unique_keys[lid]   = g_keys[start_idx];
                            g_reduce_values[lid] = value;
                        }

                    if (lid == 0) {
                        g_reduced_count[0] = n_values;
                    }
                })";

    cl::Program program(context, kernel_code);
    program.build(device, "-cl-std=CL1.2");

    const uint32_t        N = 109;
    std::vector<uint32_t> keys(N);
    std::vector<int>      values(N);

    cl::Buffer cl_keys_in(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint32_t) * N);
    cl::Buffer cl_vals_in(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(int) * N);
    cl::Buffer cl_keys_out(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(uint32_t) * N);
    cl::Buffer cl_vals_out(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(int) * N);
    cl::Buffer cl_count(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(uint32_t));

    cl::Kernel kernel(program, "reduce_by_key_small");
    kernel.setArg(0, cl_keys_in);
    kernel.setArg(1, cl_vals_in);
    kernel.setArg(2, cl_keys_out);
    kernel.setArg(3, cl_vals_out);
    kernel.setArg(4, cl_count);
    kernel.setArg(5, N);

    cl::Event event;

    cl::NDRange global(128);
    cl::NDRange local = global;

    std::this_thread::sleep_for(std::chrono::milliseconds{10});

    for (int i = 0; i < 20; i++) {
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local, nullptr, &event);
        event.wait();

        double kernel_queue = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        double kernel_exec  = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();

        uint32_t count;

        queue.enqueueReadBuffer(cl_count, true, 0, sizeof(count), &count, nullptr, &event);
        event.wait();

        double copy_queue = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        double copy_exec  = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();

        std::cout << "kernel " << kernel_queue * 1e-6 << " " << kernel_exec * 1e-6 << " ms\n";
        std::cout << "copy " << copy_queue * 1e-6 << " " << copy_exec * 1e-6 << " ms\n";
    }
}

SPLA_GTEST_MAIN