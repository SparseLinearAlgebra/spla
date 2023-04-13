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

#include <cinttypes>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using uint = std::uint32_t;


std::string kernel_global =
        "__kernel void search_coord_tile (__global const TYPE* arr_a,   \n"
        "                                 __global const TYPE* arr_b,   \n"
        "                                 __global uint* tile_a,        \n"
        "                                 __global uint* tile_b,        \n"
        "                                 const uint size_arr_a,        \n"
        "                                 const uint size_arr_b,        \n"
        "                                 const uint count_tile) {      \n"
        "                                                               \n"
        "   uint i = get_global_id(0) + 1;                              \n"
        "   if (i < count_tile) {                                       \n"
        "       uint target = i * TILE_SIZE;                            \n"
        "       uint start = max(0, (int)target - (int)size_arr_b);     \n"
        "       uint end = min(target, size_arr_a);                     \n"
        "       uint a_ind, b_ind;                                      \n"
        "       while (start < end) {                                   \n"
        "           a_ind = (start + end) / 2;                          \n"
        "           b_ind = (target - a_ind - 1);                       \n"
        "           if (arr_b[b_ind] >= arr_a[a_ind]) {                 \n"
        "               start = a_ind + 1;                              \n"
        "           } else {                                            \n"
        "               end = a_ind;                                    \n"
        "           }                                                   \n"
        "       }                                                       \n"
        "       tile_a[i] = start;                                      \n"
        "       tile_b[i] = target - start;                             \n"
        "   }                                                           \n"
        "}                                                              \n";


std::string lernel_merge_v1_thread_per_tile =
        "__kernel void merge (__global const TYPE* arr_a,                                                   \n"
        "                     __global const TYPE* arr_b,                                                   \n"
        "                     __global uint* tile_a,                                                        \n"
        "                     __global uint* tile_b,                                                        \n"
        "                     const uint count_tile,                                                        \n"
        "                     __global TYPE* result) {                                                      \n"
        "                                                                                                   \n"
        "   uint i = get_global_id(0);                                                                      \n"
        "   if (i < count_tile) {                                                                           \n"
        "       uint start1 = tile_a[i];                                                                    \n"
        "       uint end1 = tile_a[i + 1];                                                                  \n"
        "       uint start2 = tile_b[i];                                                                    \n"
        "       uint end2 =  tile_b[i + 1];                                                                 \n"
        "       for (uint ind = i * TILE_SIZE; ind < TILE_SIZE * (i + 1); ind++) {                          \n"
        "           if (start1 != end1 && (start2 == end2 || arr_b[start2] > arr_a[start1])) {              \n"
        "               result[ind] = arr_a[start1++];                                                      \n"
        "           } else {                                                                                \n"
        "               result[ind] = arr_b[start2++];                                                      \n"
        "           }                                                                                       \n"
        "       }                                                                                           \n"
        "   }                                                                                               \n"
        "}                                                                                                  \n";


std::string kernel_merge_v2_group_per_tile =
        "__kernel void merge             (__global const TYPE* arr_a,                                  \n"
        "                                 __global const TYPE* arr_b,                                  \n"
        "                                 __global TYPE* arr_res,                                      \n"
        "                                 __global uint* tile_a,                                       \n"
        "                                 __global uint* tile_b) {                                     \n"
        "   __local TYPE local_arr_a[TILE_SIZE];                                                       \n"
        "   __local TYPE local_arr_b[TILE_SIZE];                                                       \n"
        "   __local TYPE result[TILE_SIZE];                                                            \n"
        "                                                                                              \n"
        "   __local uint size_local_arr_a, size_local_arr_b;                                           \n"
        "                                                                                              \n"
        "   __local uint local_tile_a[GROUP_SIZE + 1];                                                 \n"
        "   __local uint local_tile_b[GROUP_SIZE + 1];                                                 \n"
        "                                                                                              \n"
        "   uint i = get_global_id(0);                                                                 \n"
        "   uint j = get_local_id(0);                                                                  \n"
        "   uint id_group = get_group_id(0);                                                           \n"
        "                                                                                              \n"
        "                                                                                              \n"
        "   if (j == 0) {                                                                              \n"
        "       size_local_arr_a = tile_a[id_group + 1] - tile_a[id_group];                            \n"
        "       size_local_arr_b = tile_b[id_group + 1] - tile_b[id_group];                            \n"
        "                                                                                              \n"
        "       local_tile_a[GROUP_SIZE] = size_local_arr_a;                                           \n"
        "       local_tile_b[GROUP_SIZE] = size_local_arr_b;                                           \n"
        "                                                                                              \n"
        "       local_tile_a[0] = 0;                                                                   \n"
        "       local_tile_b[0] = 0;                                                                   \n"
        "   }                                                                                          \n"
        "                                                                                              \n"
        "   barrier(CLK_LOCAL_MEM_FENCE);                                                              \n"
        "                                                                                              \n"
        "   for (uint it = j; it < TILE_SIZE; it += GROUP_SIZE) {                                      \n"
        "       if (it < size_local_arr_a) {                                                           \n"
        "           local_arr_a[it] = arr_a[tile_a[id_group] + it];                                    \n"
        "       }                                                                                      \n"
        "                                                                                              \n"
        "       if (it < size_local_arr_b) {                                                           \n"
        "           local_arr_b[it] = arr_b[tile_b[id_group] + it];                                    \n"
        "       }                                                                                      \n"
        "   }                                                                                          \n"
        "                                                                                              \n"
        "   barrier(CLK_LOCAL_MEM_FENCE);                                                              \n"
        "                                                                                              \n"
        "   if (j < GROUP_SIZE) {                                                                      \n"
        "       uint target = (j + 1) * MICRO_TILE_SIZE;                                               \n"
        "       uint start = max(0, (int)target - (int)size_local_arr_b);                              \n"
        "       uint end = min(target, size_local_arr_a);                                              \n"
        "       uint a_ind, b_ind;                                                                     \n"
        "                                                                                              \n"
        "       while (start < end) {                                                                  \n"
        "           a_ind = (start + end) / 2;                                                         \n"
        "           b_ind = target - a_ind - 1;                                                        \n"
        "           if (local_arr_b[b_ind] >= local_arr_a[a_ind]) {                                    \n"
        "               start = a_ind + 1;                                                             \n"
        "           } else {                                                                           \n"
        "               end = a_ind;                                                                   \n"
        "           }                                                                                  \n"
        "       }                                                                                      \n"
        "                                                                                              \n"
        "       local_tile_a[j + 1] = start;                                                           \n"
        "       local_tile_b[j + 1] = target - start;                                                  \n"
        "   }                                                                                          \n"
        "                                                                                              \n"
        "   barrier(CLK_LOCAL_MEM_FENCE);                                                              \n"
        "                                                                                              \n"
        "   uint start1 = local_tile_a[j];                                                             \n"
        "   uint end1   = local_tile_a[j + 1];                                                         \n"
        "   uint start2 = local_tile_b[j];                                                             \n"
        "   uint end2   =  local_tile_b[j + 1];                                                        \n"
        "   uint index  = MICRO_TILE_SIZE * j;                                                         \n"
        "                                                                                              \n"
        "   for (uint ind = MICRO_TILE_SIZE * j; ind < MICRO_TILE_SIZE * (j + 1); ind++) {             \n"
        "       if (start1 != end1 && (start2 == end2 || local_arr_b[start2] > local_arr_a[start1])) { \n"
        "           result[ind] = local_arr_a[start1++];                                               \n"
        "       } else {                                                                               \n"
        "           result[ind] = local_arr_b[start2++];                                               \n"
        "       }                                                                                      \n"
        "   }                                                                                          \n"
        "                                                                                              \n"
        "   barrier(CLK_LOCAL_MEM_FENCE);                                                              \n"
        "                                                                                              \n"
        "   for (uint it = j; it < TILE_SIZE; it += GROUP_SIZE) {                                      \n"
        "       arr_res[TILE_SIZE * id_group + it] = result[it];                                       \n"
        "   }                                                                                          \n"
        "}                                                                                             \n";


template<typename T>
void merge_search_coord_tile(std::vector<T>&    a,
                             std::vector<T>&    b,
                             uint               size_tile,
                             std::vector<uint>& tile_a,
                             std::vector<uint>& tile_b) {
    tile_a[tile_a.size() - 1] = a.size();
    tile_b[tile_b.size() - 1] = b.size();

    for (uint pointer_a = 0, pointer_b = 0, pointer_c = 0; pointer_c < (a.size() + b.size()); pointer_c++) {
        if ((pointer_a + pointer_b) % size_tile == 0) {
            tile_a[(pointer_a + pointer_b) / size_tile] = pointer_a;
            tile_b[(pointer_a + pointer_b) / size_tile] = pointer_b;
        }

        if (pointer_a != a.size() && (pointer_b == b.size() || a[pointer_a] <= b[pointer_b])) {
            pointer_a++;
        } else {
            pointer_b++;
        }
    }
}

template<typename T>
std::vector<T> merge(std::vector<T>& a,
                     std::vector<T>& b) {

    std::vector<T> res(a.size() + b.size());

    for (auto pointer_a = a.begin(), pointer_b = b.begin(), pointer_res = res.begin();
         pointer_res != res.end(); pointer_res++) {
        if (pointer_a != a.end() && (pointer_b == b.end() || *pointer_a <= *pointer_b)) {
            *(pointer_res) = *(pointer_a++);
        } else {
            *(pointer_res) = *(pointer_b++);
        }
    }

    return res;
}

bool equal_tile(const std::vector<uint>& tile_a,
                const std::vector<uint>& tile_b,
                const std::vector<uint>& _tile_a,
                const std::vector<uint>& _tile_b) {
    for (uint i = 1; i + 1 < tile_a.size(); i++) {
        if ((tile_a[i] != _tile_a[i]) && (tile_b[i] != _tile_b[i])) {
            return false;
        }
    }

    return true;
}

void output_tile_size(const std::vector<uint>& tile_x, const std::vector<uint>& tile_y) {
    for (uint i = 1; i < tile_x.size(); i++) {
        std::cout << (tile_x[i] - tile_x[i - 1] + tile_y[i] - tile_y[i - 1]) << '\t';
    }
    std::cout << std::endl;
}

// A function to return a seeded random number generator.
inline std::mt19937& generator() {
    // the generator will only be seeded once (per thread) since it's static
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

// A function to generate integers in the range [min, max]
template<typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr>
T my_rand(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(generator());
}

template<typename T>
void rand_value(std::vector<T>& arr) {
    for (auto& v : arr) {
        v = my_rand(0, 1000);
    }
}

bool check_sort(const std::vector<int>& v) {
    for (uint i = 1; i + 1 < v.size(); i++) {
        if (v[i] < v[i - 1]) {
            std::cout << i << " " << std::endl;
            return false;
        }
    }
    return true;
}


inline cl::Kernel cl_merge_thread_per_tile(const cl::Buffer&  buf_arr_a,
                                           const cl::Buffer&  buf_arr_b,
                                           const cl::Buffer&  buf_tile_a,
                                           const cl::Buffer&  buf_tile_b,
                                           const cl::Buffer&  buf_arr_result,
                                           const cl::Program& program,
                                           const uint         count_tile) {
    cl::Kernel kernel = cl::Kernel(program, "merge");

    kernel.setArg(0, buf_arr_a);
    kernel.setArg(1, buf_arr_b);
    kernel.setArg(2, buf_tile_a);
    kernel.setArg(3, buf_tile_b);
    kernel.setArg(4, count_tile);
    kernel.setArg(5, buf_arr_result);

    return kernel;
}

inline cl::Kernel cl_merge_group_per_tile(const cl::Buffer&  buf_arr_a,
                                          const cl::Buffer&  buf_arr_b,
                                          const cl::Buffer&  buf_tile_a,
                                          const cl::Buffer&  buf_tile_b,
                                          const cl::Buffer&  buf_arr_result,
                                          const cl::Program& program) {
    cl::Kernel kernel = cl::Kernel(program, "merge");

    kernel.setArg(0, buf_arr_a);
    kernel.setArg(1, buf_arr_b);
    kernel.setArg(2, buf_arr_result);
    kernel.setArg(3, buf_tile_a);
    kernel.setArg(4, buf_tile_b);

    return kernel;
}

inline cl::Kernel cl_search_cord_tile(const cl::Buffer&  buf_arr_a,
                                      const cl::Buffer&  buf_arr_b,
                                      const cl::Buffer&  buf_tile_a,
                                      const cl::Buffer&  buf_tile_b,
                                      const uint         size_buf_arr_a,
                                      const uint         size_buf_arr_b,
                                      const uint         count_tile,
                                      const cl::Program& program) {
    cl::Kernel kernel(program, "search_coord_tile");

    kernel.setArg(0, buf_arr_a);
    kernel.setArg(1, buf_arr_b);
    kernel.setArg(2, buf_tile_a);
    kernel.setArg(3, buf_tile_b);
    kernel.setArg(4, size_buf_arr_a);
    kernel.setArg(5, size_buf_arr_b);
    kernel.setArg(6, count_tile);

    return kernel;
}

void cl_merge_v1(const cl::CommandQueue& queue,
                 const cl::Buffer&       buf_arr_a,
                 const cl::Buffer&       buf_arr_b,
                 const cl::Buffer&       buf_arr_result,
                 const cl::Program&      program_gl,
                 const cl::Program&      program_loc_v1,
                 const uint              size_buf_arr_a,
                 const uint              size_buf_arr_b,
                 const uint              TILE_SIZE) {

    uint count_tile = (size_buf_arr_a + size_buf_arr_b) / TILE_SIZE;

    if ((size_buf_arr_a + size_buf_arr_b) % TILE_SIZE) {
        count_tile++;
    }

    int null = 0;

    cl::Buffer buf_tile_a(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));
    cl::Buffer buf_tile_b(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_a);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_b);

    cl::Kernel kernel_gl = cl_search_cord_tile(buf_arr_a, buf_arr_b, buf_tile_a, buf_tile_b, size_buf_arr_a, size_buf_arr_b, count_tile, program_gl);

    cl::Kernel kernel_loc = cl_merge_thread_per_tile(buf_arr_a, buf_arr_b, buf_tile_a, buf_tile_b, buf_arr_result, program_loc_v1, count_tile);

    queue.enqueueNDRangeKernel(kernel_gl, cl::NDRange(), std::max(count_tile, uint(64)));

    queue.enqueueNDRangeKernel(kernel_loc, cl::NDRange(), std::max(count_tile, uint(64)));
}

void cl_merge_v2(const cl::CommandQueue& queue,
                 const cl::Buffer&       buf_arr_a,
                 const cl::Buffer&       buf_arr_b,
                 const cl::Buffer&       buf_arr_result,
                 const cl::Program&      program_gl,
                 const cl::Program&      program_loc_v2,
                 const uint              size_buf_arr_a,
                 const uint              size_buf_arr_b,
                 const uint              TILE_SIZE,
                 const uint              GROUP_SIZE) {

    uint count_tile = (size_buf_arr_a + size_buf_arr_b) / TILE_SIZE;

    if ((size_buf_arr_a + size_buf_arr_b) % TILE_SIZE) {
        count_tile++;
    }

    int null = 0;

    cl::Buffer buf_tile_a(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));
    cl::Buffer buf_tile_b(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_a);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_b);

    cl::Kernel kernel_gl = cl_search_cord_tile(buf_arr_a, buf_arr_b, buf_tile_a, buf_tile_b, size_buf_arr_a, size_buf_arr_b, count_tile, program_gl);

    cl::Kernel kernel_loc = cl_merge_group_per_tile(buf_arr_a, buf_arr_b, buf_tile_a, buf_tile_b, buf_arr_result, program_loc_v2);

    queue.enqueueNDRangeKernel(kernel_gl, cl::NDRange(), std::max(count_tile, uint(64)));

    queue.enqueueNDRangeKernel(kernel_loc, cl::NDRange(),
                               count_tile * GROUP_SIZE, GROUP_SIZE);
}

TEST(opencl_merge, merge_path_v1) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.back();

    std::cout << "Platforms: " << std::endl;
    for (auto& it : platforms) {
        std::cout << "- " << it.getInfo<CL_PLATFORM_NAME>() << std::endl;
    }

    std::cout << "Current platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices[0];

    std::cout << "Devices: " << std::endl;
    for (auto& it : devices) {
        std::cout << "- " << it.getInfo<CL_DEVICE_NAME>() << std::endl;
    }

    std::cout << "Current device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    cl::Program program_gl;
    cl::Program program_loc_v1;
    cl::Program program_loc_v2;

    program_gl = cl::Program(context, kernel_global);
    program_gl.build(device, "-cl-std=CL1.2 -DTILE_SIZE=8 -DTYPE=int");

    program_loc_v1 = cl::Program(context, lernel_merge_v1_thread_per_tile);
    program_loc_v1.build(device, "-cl-std=CL1.2 -DTILE_SIZE=8 -DTYPE=int");

    std::chrono::time_point                   start_time      = std::chrono::steady_clock::now();
    std::chrono::time_point                   end_time        = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> result_time_cpu = end_time - start_time;
    std::chrono::duration<double, std::milli> result_time_gpu = end_time - start_time;

    const uint N_RUNS    = 10;
    const uint N         = 1 * 1024 * 1024;
    const uint TILE_SIZE = 8;

    for (uint t = 0; t < N_RUNS; t++) {
        std::cout << "Test #" << t + 1 << " ";

        using T               = int;
        const uint size_arr_a = N;
        const uint size_arr_b = N;

        std::vector<T> arr_a(size_arr_a);
        std::vector<T> arr_b(size_arr_b);
        std::vector<T> arr_c(size_arr_a + size_arr_b);
        rand_value(arr_a);
        rand_value(arr_b);

        std::sort(arr_a.begin(), arr_a.end());
        std::sort(arr_b.begin(), arr_b.end());

        cl::Buffer buf_arr_a(queue, arr_a.begin(), arr_a.end(), true, false);
        cl::Buffer buf_arr_b(queue, arr_b.begin(), arr_b.end(), true, false);
        cl::Buffer buf_arr_res(context, CL_MEM_READ_WRITE, sizeof(T) * (size_arr_a + size_arr_b));

        start_time = std::chrono::steady_clock::now();
        cl_merge_v1(queue, buf_arr_a, buf_arr_b, buf_arr_res, program_gl, program_loc_v1, size_arr_a, size_arr_b, TILE_SIZE);
        queue.finish();
        end_time = std::chrono::steady_clock::now();

        std::cout << " gpu " << (std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time)).count() << " ms";

        cl::copy(queue, buf_arr_res, arr_c.begin(), arr_c.end());
        result_time_gpu += (end_time - start_time);

        start_time       = std::chrono::steady_clock::now();
        std::vector<T> c = merge(arr_a, arr_b);
        end_time         = std::chrono::steady_clock::now();
        result_time_cpu += (end_time - start_time);

        std::cout << " cpu " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count() << " ms\n";

        EXPECT_TRUE(check_sort(arr_c) && check_sort(c));
        EXPECT_EQ(arr_c, c);
    }

    std::cout << "Time taken for Merge Gpu: " << (result_time_gpu / N_RUNS).count() << " ms\n";
    std::cout << "Time taken for Merge Cpu: " << (result_time_cpu / N_RUNS).count() << " ms\n";
}

TEST(opencl_merge, merge_path_v2) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.back();

    std::cout << "Platforms: " << std::endl;
    for (auto& it : platforms) {
        std::cout << "- " << it.getInfo<CL_PLATFORM_NAME>() << std::endl;
    }

    std::cout << "Current platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices[0];

    std::cout << "Devices: " << std::endl;
    for (auto& it : devices) {
        std::cout << "- " << it.getInfo<CL_DEVICE_NAME>() << std::endl;
    }

    std::cout << "Current device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    cl::Program program_gl;
    cl::Program program_loc_v1;
    cl::Program program_loc_v2;

    program_gl = cl::Program(context, kernel_global);
    program_gl.build(device, "-cl-std=CL1.2 -DTILE_SIZE=128 -DTYPE=int");

    program_loc_v2 = cl::Program(context, kernel_merge_v2_group_per_tile);
    program_loc_v2.build(device, "-cl-std=CL1.2 -DTILE_SIZE=128 -DGROUP_SIZE=64 -DMICRO_TILE_SIZE=TILE_SIZE/GROUP_SIZE -DTYPE=int");

    std::chrono::time_point                   start_time      = std::chrono::steady_clock::now();
    std::chrono::time_point                   end_time        = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> result_time_cpu = end_time - start_time;
    std::chrono::duration<double, std::milli> result_time_gpu = end_time - start_time;

    const uint N_RUNS     = 10;
    const uint N          = 1 * 1024 * 1024;
    const uint TILE_SIZE  = 128;
    const uint GROUP_SIZE = 64;

    for (uint t = 0; t < N_RUNS; t++) {
        std::cout << "Test #" << t + 1 << " ";

        using T               = int;
        const uint size_arr_a = N;
        const uint size_arr_b = N;

        std::vector<T> arr_a(size_arr_a);
        std::vector<T> arr_b(size_arr_b);
        std::vector<T> arr_c(size_arr_a + size_arr_b);
        rand_value(arr_a);
        rand_value(arr_b);

        std::sort(arr_a.begin(), arr_a.end());
        std::sort(arr_b.begin(), arr_b.end());

        cl::Buffer buf_arr_a(queue, arr_a.begin(), arr_a.end(), true, false);
        cl::Buffer buf_arr_b(queue, arr_b.begin(), arr_b.end(), true, false);
        cl::Buffer buf_arr_res(context, CL_MEM_READ_WRITE, sizeof(T) * (size_arr_a + size_arr_b));

        start_time = std::chrono::steady_clock::now();
        cl_merge_v2(queue, buf_arr_a, buf_arr_b, buf_arr_res, program_gl, program_loc_v2, size_arr_a, size_arr_b, TILE_SIZE, GROUP_SIZE);
        queue.finish();
        end_time = std::chrono::steady_clock::now();

        std::cout << " gpu " << (std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time)).count() << " ms";

        cl::copy(queue, buf_arr_res, arr_c.begin(), arr_c.end());
        result_time_gpu += (end_time - start_time);

        start_time       = std::chrono::steady_clock::now();
        std::vector<T> c = merge(arr_a, arr_b);
        end_time         = std::chrono::steady_clock::now();
        result_time_cpu += (end_time - start_time);

        std::cout << " cpu " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count() << " ms\n";

        EXPECT_TRUE(check_sort(arr_c) && check_sort(c));
        EXPECT_EQ(arr_c, c);
    }

    std::cout << "Time taken for Merge Gpu: " << (result_time_gpu / N_RUNS).count() << " ms\n";
    std::cout << "Time taken for Merge Cpu: " << (result_time_cpu / N_RUNS).count() << " ms\n";
}

SPLA_GTEST_MAIN