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
#include <random>

using std::vector;
using std::cout;
using std::endl;

#define TILE_SIZE 256
#define GROUP_SIZE 64

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

std::string kernel_locale =
        "__kernel void merge (__global const TYPE* arr_a,                                                   "
        "                     __global const TYPE* arr_b,                                                   "
        "                     __global uint* tile_a,                                                        "
        "                     __global uint* tile_b,                                                        "
        "                     const uint count_tile,                                                        "
        "                     __global TYPE* result) {                                                      "
        "                                                                                                   "
        "   uint i = get_global_id(0);                                                                      "
        "   if (i < count_tile) {                                                                           "
        "       uint start1 = tile_a[i];                                                                    "
        "       uint end1 = tile_a[i + 1];                                                                  "
        "       uint start2 = tile_b[i];                                                                    "
        "       uint end2 =  tile_b[i + 1];                                                                 "
        "       for (uint ind = i * TILE_SIZE; ind < TILE_SIZE * (i + 1); ind++) {                          "
        "           if (start1 != end1 && (start2 == end2 || arr_b[start2] > arr_a[start1])) {              "
        "               result[ind] = arr_a[start1++];                                                      "
        "           } else {                                                                                "
        "               result[ind] = arr_b[start2++];                                                      "
        "           }                                                                                       "
        "       }                                                                                           "
        "   }                                                                                               "
        "}                                                                                                  ";

std::string kernel_global =
        "__kernel void search_coord_tile (__global const TYPE* arr_a,   "
        "                                 __global const TYPE* arr_b,   "
        "                                 __global uint* tile_a,        "
        "                                 __global uint* tile_b,        "
        "                                 const uint size_arr_a,        "
        "                                 const uint size_arr_b,        "
        "                                 const uint count_tile) {      "
        "                                                               "
        "   uint i = get_global_id(0) + 1;                              "
        "   if (i < count_tile) {                                       "
        "       uint target = i * TILE_SIZE;                            "
        "       uint start = max(0, int(target) - int(size_arr_b));     "
        "       uint end = min(target, size_arr_a);                     "
        "       uint a_ind, b_ind;                                      "
        "       while (start < end) {                                   "
        "           a_ind = (start + end) / 2;                          "
        "           b_ind = (target - a_ind - 1);                       "
        "           if (arr_b[b_ind] >= arr_a[a_ind]) {                 "
        "               start = a_ind + 1;                              "
        "           } else {                                            "
        "               end = a_ind;                                    "
        "           }                                                   "
        "       }                                                       "
        "       tile_a[i] = start;                                      "
        "       tile_b[i] = target - start;                             "
        "   }                                                           "
        "}                                                              ";


std::string kernel_local_v2 =
        "__kernel void merge             (__global const TYPE* arr_a,   "
        "                                 __global const TYPE* arr_b,   "
        "                                 __global TYPE* arr_res,       "
        "                                 __global uint* tile_a,        "
        "                                 __global uint* tile_b,        "
        "                                 uint size_arr_a,              "
        "                                 uint size_arr_b) {            "
        "   __local TYPE local_arr_a[TILE_SIZE];                    "
        "   __local TYPE local_arr_b[TILE_SIZE];                    "
        "   __local TYPE result[TILE_SIZE];                         "
        "                                                           "
        "   __local uint size_local_arr_a, size_local_arr_b;        "
        "                                                           "
        "   __local uint local_tile_a[GROUP_SIZE + 1];              "
        "   __local uint local_tile_b[GROUP_SIZE + 1];              "
        "                                                           "
        "   uint i = get_global_id(0);                              "
        "   uint j = get_local_id(0);                               "
        "   uint id_group = get_group_id(0);                        "
        "                                                           "
        "                                                           "
        "   if (j == 0) {                                                   "
        "       size_local_arr_a = tile_a[id_group + 1] - tile_a[id_group]; "
        "       size_local_arr_b = tile_b[id_group + 1] - tile_b[id_group]; "
        "                                                                   "
        "       local_tile_a[GROUP_SIZE] = size_local_arr_a;                "
        "       local_tile_b[GROUP_SIZE] = size_local_arr_b;                "
        "                                                                   "
        "       local_tile_a[0] = 0;                                        "
        "       local_tile_b[0] = 0;                                        "
        "   }                                                               "
        "                                                                   "
        "   barrier(CLK_LOCAL_MEM_FENCE);                                   "
        "                                                                   "
        "   for (uint it = j; it < TILE_SIZE; it += GROUP_SIZE) {           "
        "       if (it < size_local_arr_a) {                                "
        "           local_arr_a[it] = arr_a[tile_a[id_group] + it];         "
        "       }                                                           "
        "                                                                   "
        "       if (it < size_local_arr_b) {                                "
        "           local_arr_b[it] = arr_b[tile_b[id_group] + it];         "
        "       }                                                           "
        "   }                                                               "
        "                                                                   "
        "   barrier(CLK_LOCAL_MEM_FENCE);                                   "
        "                                                                   "
        "   if (j < GROUP_SIZE) {                                           "
        "       uint target = (j + 1) * MICRO_TILE_SIZE;                    "
        "       uint start = max(0, int(target) - int(size_local_arr_b));   "
        "       uint end = min(target, size_local_arr_a);                   "
        "       uint a_ind, b_ind;                                          "
        "                                                                   "
        "       while (start < end) {                                       "
        "           a_ind = (start + end) / 2;                              "
        "           b_ind = target - a_ind - 1;                             "
        "           if (local_arr_b[b_ind] >= local_arr_a[a_ind]) {         "
        "               start = a_ind + 1;                                  "
        "           } else {                                                "
        "               end = a_ind;                                        "
        "           }                                                       "
        "       }                                                           "
        "                                                                   "
        "       local_tile_a[j + 1] = start;                                "
        "       local_tile_b[j + 1] = target - start;                       "
        "   }                                                               "
        "                                                                   "
        "   barrier(CLK_LOCAL_MEM_FENCE);                                   "
        "                                                                   "
        "   uint start1 = local_tile_a[j];                                  "
        "   uint end1   = local_tile_a[j + 1];                              "
        "   uint start2 = local_tile_b[j];                                  "
        "   uint end2   =  local_tile_b[j + 1];                             "
        "   uint index  = MICRO_TILE_SIZE * j;                              "
        "                                                                   "
        "   for (uint ind = MICRO_TILE_SIZE * j; ind < MICRO_TILE_SIZE * (j + 1); ind++) {              "
        "       if (start1 != end1 && (start2 == end2 || local_arr_b[start2] > local_arr_a[start1])) {  "
        "           result[ind] = local_arr_a[start1++];                                                "
        "       } else {                                                                                "
        "           result[ind] = local_arr_b[start2++];                                                "
        "       }                                                                                       "
        "   }                                                                                           "
        "                                                                                               "
        "   barrier(CLK_LOCAL_MEM_FENCE);                                                               "
        "                                                                                               "
        "   for (uint it = j; it < TILE_SIZE; it += GROUP_SIZE) {                                       "
        "       arr_res[TILE_SIZE * id_group + it] = result[it];                                        "
        "   }                                                                                           "
        "}                                                                                              ";

static cl::Program program_gl;
static cl::Program program_loc_v1;
static cl::Program program_loc_v2;


/////// Naive realization merge (search coordinate)

template<typename T>
void merge_search_coord_tile(vector<T> &a,
                             vector<T> &b,
                             uint size_tile,
                             vector<uint> &tile_a,
                             vector<uint> &tile_b) {
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

///////


/////// Naive realization merge

template<typename T>
vector<T> merge(vector<T> &a,
                vector<T> &b) {

    vector<T> res(a.size() + b.size());

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


///////


bool equal_tile(const vector<uint> &tile_a,
                const vector<uint> &tile_b,
                const vector<uint> &_tile_a,
                const vector<uint> &_tile_b) {
    for (uint i = 1; i + 1 < tile_a.size(); i++) {
        if ((tile_a[i] != _tile_a[i]) && (tile_b[i] != _tile_b[i])) {
            return false;
        }
    }

    return true;
}

template<typename T>
bool equal_arr(const vector<T>& a, const vector<T>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (uint i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}


void output_tile_size(const vector<uint> &tile_x, const vector<uint> &tile_y) {
    for (uint i = 1; i < tile_x.size(); i++) {
        cout << (tile_x[i] - tile_x[i - 1] + tile_y[i] - tile_y[i - 1]) << '\t';
    } cout << endl;
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

// A function to generate floats in the range [min, max)
template<typename T, std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
T my_rand(T min, T max) {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(generator());
}

template <typename T>
void rand_value(vector<T> &arr) {
    for (auto &v : arr) {
        v = my_rand(0, 1000);
    }
}

bool check_sort(const vector<int> &v) {
    for (uint i = 1; i + 1 < v.size(); i++) {
        if (v[i] < v[i - 1]) {
            cout << i << " " << endl;
            return false;
        }
    }
    return true;
}


void cl_merge(const cl::Context& context,
              const cl::CommandQueue& queue,
              const cl::Buffer& buf_arr_a,
              const cl::Buffer& buf_arr_b,
              const cl::Buffer& buf_arr_result,
              uint size_buf_arr_a,
              uint size_buf_arr_b) {

    uint count_tile = (size_buf_arr_a + size_buf_arr_b) / TILE_SIZE;

    if ((size_buf_arr_a + size_buf_arr_b) % TILE_SIZE) {
        count_tile++;
    }

    int null = 0;

    cl::Buffer buf_tile_a(context, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));
    cl::Buffer buf_tile_b(context, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile + 1));

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, 0, sizeof(uint), &null);

    queue.enqueueWriteBuffer(buf_tile_a, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_a);
    queue.enqueueWriteBuffer(buf_tile_b, CL_MEM_READ_WRITE, sizeof(uint) * (count_tile), sizeof(uint), &size_buf_arr_b);

    cl::Kernel kernel_gl(program_gl, "search_coord_tile");

    kernel_gl.setArg(0, buf_arr_a);
    kernel_gl.setArg(1, buf_arr_b);
    kernel_gl.setArg(2, buf_tile_a);
    kernel_gl.setArg(3, buf_tile_b);
    kernel_gl.setArg(4, size_buf_arr_a);
    kernel_gl.setArg(5, size_buf_arr_b);
    kernel_gl.setArg(6, count_tile);

    cl::Kernel kernel_loc_v1 = cl::Kernel(program_loc_v1, "merge");

    kernel_loc_v1.setArg(0, buf_arr_a);
    kernel_loc_v1.setArg(1, buf_arr_b);
    kernel_loc_v1.setArg(2, buf_tile_a);
    kernel_loc_v1.setArg(3, buf_tile_b);
    kernel_loc_v1.setArg(4, count_tile);
    kernel_loc_v1.setArg(5, buf_arr_result);

    cl::Kernel kernel_loc_v2 = cl::Kernel(program_loc_v2, "merge");

    kernel_loc_v2.setArg(0, buf_arr_a);
    kernel_loc_v2.setArg(1, buf_arr_b);
    kernel_loc_v2.setArg(2, buf_arr_result);
    kernel_loc_v2.setArg(3, buf_tile_a);
    kernel_loc_v2.setArg(4, buf_tile_b);
    kernel_loc_v2.setArg(5, size_buf_arr_a);
    kernel_loc_v2.setArg(6, size_buf_arr_b);

    queue.enqueueNDRangeKernel(kernel_gl, cl::NDRange(), std::max(count_tile, uint(64)));

    queue.enqueueNDRangeKernel(kernel_loc_v2, cl::NDRange(),
                               std::max(size_buf_arr_b + size_buf_arr_a, uint(64)), GROUP_SIZE);

}

TEST (opencl, merge_path) {
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices[1];

    cout << "////////// LIST DEVICE: " << endl;
    for (auto& it: devices) {
        cout << "- " << it.getInfo<CL_DEVICE_NAME>() << endl;
    }

    cout << "//////////" << endl;

    cout << "current device: " << device.getInfo<CL_DEVICE_NAME>() << endl;

    cl::Context      context(device);
    cl::CommandQueue queue(context);

    program_gl = cl::Program(context, kernel_global);
    program_gl.build(device, "-cl-std=CL1.2 -DTILE_SIZE=256 -DTYPE=int");

    program_loc_v1 = cl::Program(context, kernel_locale);
    program_loc_v1.build(device, "-cl-std=CL1.2 -DTILE_SIZE=256 -DTYPE=int");

    program_loc_v2 = cl::Program(context, kernel_local_v2);
    program_loc_v2.build(device, "-cl-std=CL1.2 -DTILE_SIZE=256 -DGROUP_SIZE=64 -DMICRO_TILE_SIZE=TILE_SIZE/GROUP_SIZE -DTYPE=int");

    std::chrono::time_point start_time = std::chrono::steady_clock::now();
    std::chrono::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> result_time_cpu = end_time - start_time;
    std::chrono::duration<double, std::milli> result_time_gpu = end_time - start_time;

    for (uint t = 0; t < 10 ; t++) {
        cout << "//////////" << endl;
        cout << "TEST #" << t + 1 << endl;

        //// GENERATE DATA
        using T               = int;
        const uint size_arr_a = 2 * 1024 * 1024;
        const uint size_arr_b = 2 * 1024 * 1024;

        vector<T> arr_a(size_arr_a);
        vector<T> arr_b(size_arr_b);
        vector<T> arr_c(size_arr_a + size_arr_b);
        rand_value(arr_a);
        rand_value(arr_b);

        std::sort(arr_a.begin(), arr_a.end());
        std::sort(arr_b.begin(), arr_b.end());

        cl::Buffer buf_arr_a(queue, arr_a.begin(), arr_a.end(), true, false);
        cl::Buffer buf_arr_b(queue, arr_b.begin(), arr_b.end(), true, false);
        cl::Buffer buf_arr_res(context, CL_MEM_READ_WRITE, sizeof(T) * (size_arr_a + size_arr_b));
        ////


        //// PARALLEL MERGE SORT
        start_time = std::chrono::steady_clock::now();
        cl_merge(context, queue, buf_arr_a, buf_arr_b, buf_arr_res, size_arr_a, size_arr_b);
        end_time = std::chrono::steady_clock::now();
        cl::copy(queue, buf_arr_res, arr_c.begin(), arr_c.end());
        result_time_gpu += (end_time - start_time);
        //fprintf(stderr, "Time taken for PARALLEL: %-8.3lfms\n", result_time.count());
        ////

        //// NAIVE MERGE SORT
        start_time = std::chrono::steady_clock::now();
        vector<T> c = merge(arr_a, arr_b);
        end_time = std::chrono::steady_clock::now();
        result_time_cpu += (end_time - start_time);
        //fprintf(stderr, "Time taken for STANDART: %-8.3lfms\n", result_time.count());
        ////

        // CHECKED SORT
        if (check_sort(arr_c) && equal_arr(arr_c, c)) {
            cout << "CORRECT" << endl;
        } else {
            cout << "NO CORRECT" << endl;
        }
        //
    }
    cout << "//////////" << endl;
    fprintf(stderr, "Time taken for PARALLEL: %-8.3lfms\n", (result_time_gpu / 10).count());
    fprintf(stderr, "Time taken for STANDART: %-8.3lfms\n", (result_time_cpu / 10).count());
}

SPLA_GTEST_MAIN