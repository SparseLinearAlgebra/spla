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

#include <spla.hpp>

#include <algorithm>
#include <climits>
#include <functional>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using uint = spla::uint;

inline std::mt19937& generator() {
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

template<typename T>
T my_rand(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(generator());
}

template<>
float my_rand(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(generator());
}

inline bool ensure_opencl() {
    auto* lib = spla::Library::get();
    if (lib->get_accelerator() != nullptr) {
        return true;
    }
    return lib->set_accelerator(spla::AcceleratorType::OpenCL) == spla::Status::Ok &&
           lib->get_accelerator() != nullptr;
}

template<typename T>
void fill_vector(spla::ref_ptr<spla::Vector>& vec, const std::vector<T>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        if constexpr (std::is_same_v<T, float>) {
            vec->set_float(i, data[i]);
        } else if constexpr (std::is_same_v<T, int>) {
            vec->set_int(i, data[i]);
        } else if constexpr (std::is_same_v<T, uint>) {
            vec->set_uint(i, data[i]);
        }
    }
}

template<typename T>
std::vector<std::pair<uint, T>> collect_sorted(spla::ref_ptr<spla::Vector>& r_keys,
                                               spla::ref_ptr<spla::Vector>& r_vals) {
    auto count = spla::Scalar::make_int(0);
    spla::exec_v_count_mf(count, r_keys);

    std::vector<std::pair<uint, T>> res;
    res.reserve(count->as_int());
    for (int i = 0; i < count->as_int(); ++i) {
        uint key;
        T    val;
        r_keys->get_uint(static_cast<uint>(i), key);
        if constexpr (std::is_same_v<T, float>) {
            r_vals->get_float(static_cast<uint>(i), val);
        } else if constexpr (std::is_same_v<T, int>) {
            r_vals->get_int(static_cast<uint>(i), val);
        } else {
            r_vals->get_uint(static_cast<uint>(i), val);
        }
        res.emplace_back(key, val);
    }

    std::sort(res.begin(), res.end(),
              [](const std::pair<uint, T>& a, const std::pair<uint, T>& b) { return a.first < b.first; });
    return res;
}

template<typename T>
std::vector<std::pair<uint, T>> reference_intersect(const std::vector<uint>&      a_keys,
                                                    const std::vector<T>&         a_vals,
                                                    const std::vector<uint>&      b_keys,
                                                    const std::vector<T>&         b_vals,
                                                    const std::function<T(T, T)>& op) {
    std::vector<std::pair<uint, T>> res;
    size_t                          i = 0, j = 0;
    while (i < a_keys.size() && j < b_keys.size()) {
        if (a_keys[i] < b_keys[j]) {
            ++i;
        } else if (b_keys[j] < a_keys[i]) {
            ++j;
        } else {
            res.emplace_back(a_keys[i], op(a_vals[i], b_vals[j]));
            ++i;
            ++j;
        }
    }
    return res;
}

template<typename T>
void expect_result(const std::vector<std::pair<uint, T>>& got,
                   const std::vector<std::pair<uint, T>>& expected) {
    ASSERT_EQ(got.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(got[i].first, expected[i].first);
        if constexpr (std::is_same_v<T, float>) {
            EXPECT_FLOAT_EQ(got[i].second, expected[i].second);
        } else {
            EXPECT_EQ(got[i].second, expected[i].second);
        }
    }
}

std::vector<uint> random_sorted_unique_keys(uint n, uint key_range) {
    std::vector<uint> pool(key_range);
    std::iota(pool.begin(), pool.end(), 0u);
    std::shuffle(pool.begin(), pool.end(), generator());

    std::vector<uint> keys(pool.begin(), pool.begin() + n);
    std::sort(keys.begin(), keys.end());
    return keys;
}

static int count_keys(const spla::ref_ptr<spla::Vector>& keys) {
    auto count = spla::Scalar::make_int(0);
    spla::exec_v_count_mf(count, keys);
    return count->as_int();
}

static spla::ref_ptr<spla::Vector> make_dense_uint(const std::vector<uint>& data) {
    auto v = spla::Vector::make(static_cast<uint>(data.size()), spla::UINT);
    v->set_format(spla::FormatVector::CpuDense);
    for (std::size_t i = 0; i < data.size(); ++i) {
        v->set_uint(static_cast<uint>(i), data[i]);
    }
    return v;
}

static spla::ref_ptr<spla::Vector> make_dense_float(const std::vector<float>& data) {
    auto v = spla::Vector::make(static_cast<uint>(data.size()), spla::FLOAT);
    v->set_format(spla::FormatVector::CpuDense);
    for (std::size_t i = 0; i < data.size(); ++i) {
        v->set_float(static_cast<uint>(i), data[i]);
    }
    return v;
}

TEST(opencl_intersect, simple_case_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {0, 1, 4, 7};
    std::vector<float> a_vals = {10, 10, 5, 30};
    std::vector<uint>  b_keys = {2, 4, 5, 7, 8};
    std::vector<float> b_vals = {20, 2, 2, 2, 30};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::FLOAT);

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{4, 7}, {7, 32}});
}

TEST(opencl_intersect, simple_case_int) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint> a_keys = {0, 1, 4, 7};
    std::vector<int>  a_vals = {10, 10, 5, 30};
    std::vector<uint> b_keys = {2, 4, 5, 7, 8};
    std::vector<int>  b_vals = {20, 2, 2, 2, 30};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::INT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::INT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::INT);

    auto op = spla::PLUS_INT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<int>(r_keys, r_vals),
                  std::vector<std::pair<uint, int>>{{4, 7}, {7, 32}});
}

TEST(opencl_intersect, no_keys_match_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {1, 3, 5, 7};
    std::vector<float> a_vals = {10, 30, 50, 70};
    std::vector<uint>  b_keys = {2, 4, 6, 8};
    std::vector<float> b_vals = {20, 40, 60, 80};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(a_keys.size(), spla::UINT);
    auto r_vals = spla::Vector::make(a_keys.size(), spla::FLOAT);

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals), std::vector<std::pair<uint, float>>{});
}

TEST(opencl_intersect, all_keys_match_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {1, 2, 3, 4};
    std::vector<float> a_vals = {10, 20, 30, 40};
    std::vector<uint>  b_keys = {1, 2, 3, 4};
    std::vector<float> b_vals = {1, 2, 3, 4};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(a_keys.size(), spla::UINT);
    auto r_vals = spla::Vector::make(a_keys.size(), spla::FLOAT);

    auto op = spla::MULT_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{1, 10}, {2, 40}, {3, 90}, {4, 160}});
}

TEST(opencl_intersect, different_sizes_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {1, 10, 100};
    std::vector<float> a_vals = {100, 200, 300};
    std::vector<uint>  b_keys = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100};
    std::vector<float> b_vals = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1000};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(a_keys.size(), spla::UINT);
    auto r_vals = spla::Vector::make(a_keys.size(), spla::FLOAT);

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{1, 101}, {10, 210}, {100, 1300}});
}

TEST(opencl_intersect, subtraction_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {0, 1, 4, 7};
    std::vector<float> a_vals = {10, 10, 5, 30};
    std::vector<uint>  b_keys = {2, 4, 5, 7, 8};
    std::vector<float> b_vals = {20, 2, 2, 2, 30};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::FLOAT);

    auto op = spla::OpBinary::make_float("sub",
                                         "(float a, float b) { return a - b; }",
                                         [](float a, float b) { return a - b; });

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{4, 3}, {7, 28}});
}

TEST(opencl_intersect, zero_key_and_value_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {0, 3};
    std::vector<float> a_vals = {5, 7};
    std::vector<uint>  b_keys = {0, 3};
    std::vector<float> b_vals = {5, 7};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::FLOAT);

    auto op = spla::OpBinary::make_float("sub",
                                         "(float a, float b) { return a - b; }",
                                         [](float a, float b) { return a - b; });

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{0, 0}, {3, 0}});
}

TEST(opencl_intersect, uint_type) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint> a_keys = {0, 1, 4, 7};
    std::vector<uint> a_vals = {10, 10, 5, 30};
    std::vector<uint> b_keys = {2, 4, 5, 7, 8};
    std::vector<uint> b_vals = {20, 2, 2, 2, 30};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::UINT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::UINT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);

    auto op = spla::PLUS_UINT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<uint>(r_keys, r_vals),
                  std::vector<std::pair<uint, uint>>{{4, 7}, {7, 32}});
}

TEST(opencl_intersect, edge_keys_uint) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint> a_keys = {0, 1, UINT_MAX};
    std::vector<uint> a_vals = {10, 20, 30};
    std::vector<uint> b_keys = {0, 5, UINT_MAX};
    std::vector<uint> b_vals = {1, 2, 3};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::UINT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::UINT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);

    auto op = spla::PLUS_UINT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<uint>(r_keys, r_vals),
                  std::vector<std::pair<uint, uint>>{{0, 11}, {UINT_MAX, 33}});
}

TEST(opencl_intersect, null_input_returns_error) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto a_vals = make_dense_float({1.0f, 2.0f});
    auto b_keys = make_dense_uint({0u, 1u});
    auto b_vals = make_dense_float({1.0f, 2.0f});
    auto r_keys = spla::Vector::make(2, spla::UINT);
    auto r_vals = spla::Vector::make(2, spla::FLOAT);

    spla::ref_ptr<spla::Vector> a_keys;

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys, a_vals, b_keys, b_vals, r_keys, r_vals, op);
    EXPECT_EQ(status, spla::Status::InvalidArgument);
}

TEST(opencl_intersect, null_op_returns_error) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto a_keys = make_dense_uint({0u, 1u});
    auto a_vals = make_dense_float({1.0f, 2.0f});
    auto b_keys = make_dense_uint({0u, 1u});
    auto b_vals = make_dense_float({1.0f, 2.0f});
    auto r_keys = spla::Vector::make(2, spla::UINT);
    auto r_vals = spla::Vector::make(2, spla::FLOAT);

    spla::ref_ptr<spla::OpBinary> op;

    auto status = spla::exec_intersect(a_keys, a_vals, b_keys, b_vals, r_keys, r_vals, op);
    EXPECT_EQ(status, spla::Status::Error);
}

TEST(opencl_intersect, deferred_task) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint>  a_keys = {0, 1, 4, 7};
    std::vector<float> a_vals = {10, 10, 5, 30};
    std::vector<uint>  b_keys = {2, 4, 5, 7, 8};
    std::vector<float> b_vals = {20, 2, 2, 2, 30};

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::FLOAT);

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    spla::ref_ptr<spla::ScheduleTask> task;
    auto                              status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op,
                                                                    spla::ref_ptr<spla::Descriptor>(), &task);
    ASSERT_EQ(status, spla::Status::Ok);
    ASSERT_TRUE(task);

    auto schedule = spla::make_schedule();
    ASSERT_EQ(schedule->step_task(task), spla::Status::Ok);
    ASSERT_EQ(schedule->submit(), spla::Status::Ok);

    expect_result(collect_sorted<float>(r_keys, r_vals),
                  std::vector<std::pair<uint, float>>{{4, 7}, {7, 32}});
}

TEST(opencl_intersect, large_disjoint_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    const uint N = 500000;

    std::vector<uint>  a_keys(N);
    std::vector<uint>  b_keys(N);
    std::vector<float> a_vals(N, 1.0f);
    std::vector<float> b_vals(N, 2.0f);

    for (uint i = 0; i < N; ++i) {
        a_keys[i] = 2 * i;
        b_keys[i] = 2 * i + 1;
    }

    auto a_keys_vec = make_dense_uint(a_keys);
    auto a_vals_vec = make_dense_float(a_vals);
    auto b_keys_vec = make_dense_uint(b_keys);
    auto b_vals_vec = make_dense_float(b_vals);
    auto r_keys     = spla::Vector::make(N, spla::UINT);
    auto r_vals     = spla::Vector::make(N, spla::FLOAT);
    auto op         = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);
    EXPECT_EQ(count_keys(r_keys), 0);
}

TEST(opencl_intersect, large_full_overlap_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    const uint N = 500000;

    std::vector<uint>  a_keys(N);
    std::vector<uint>  b_keys(N);
    std::vector<float> a_vals(N, 3.0f);
    std::vector<float> b_vals(N, 4.0f);

    for (uint i = 0; i < N; ++i) {
        a_keys[i] = i;
        b_keys[i] = i;
    }

    auto a_keys_vec = make_dense_uint(a_keys);
    auto a_vals_vec = make_dense_float(a_vals);
    auto b_keys_vec = make_dense_uint(b_keys);
    auto b_vals_vec = make_dense_float(b_vals);
    auto r_keys     = spla::Vector::make(N, spla::UINT);
    auto r_vals     = spla::Vector::make(N, spla::FLOAT);
    auto op         = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);
    EXPECT_EQ(count_keys(r_keys), static_cast<int>(N));
}

TEST(opencl_intersect, asymmetric_sizes_large_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    const uint N = 500000;

    std::vector<uint>  a_keys = {5, 500, N - 1};
    std::vector<float> a_vals = {1.0f, 2.0f, 3.0f};
    std::vector<uint>  b_keys(N);
    std::vector<float> b_vals(N, 1.0f);

    for (uint i = 0; i < N; ++i) {
        b_keys[i] = i;
    }

    auto a_keys_vec = make_dense_uint(a_keys);
    auto a_vals_vec = make_dense_float(a_vals);
    auto b_keys_vec = make_dense_uint(b_keys);
    auto b_vals_vec = make_dense_float(b_vals);
    auto r_keys     = spla::Vector::make(a_keys.size(), spla::UINT);
    auto r_vals     = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto op         = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);
    EXPECT_EQ(count_keys(r_keys), 3);
}

TEST(opencl_intersect, large_values_uint) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    std::vector<uint> a_keys = {0, 1, 2};
    std::vector<uint> a_vals = {4000000000u, 3000000000u, 1u};
    std::vector<uint> b_keys = {0, 1, 2};
    std::vector<uint> b_vals = {1u, 2u, 3u};

    auto a_keys_vec = make_dense_uint(a_keys);
    auto a_vals_vec = make_dense_uint(a_vals);
    auto b_keys_vec = make_dense_uint(b_keys);
    auto b_vals_vec = make_dense_uint(b_vals);
    auto r_keys     = spla::Vector::make(3, spla::UINT);
    auto r_vals     = spla::Vector::make(3, spla::UINT);
    auto op         = spla::PLUS_UINT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    expect_result(collect_sorted<uint>(r_keys, r_vals),
                  std::vector<std::pair<uint, uint>>{{0, 4000000001u}, {1, 3000000002u}, {2, 4u}});
}

TEST(opencl_intersect, random_large_float) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    const uint N         = 200000;
    const uint KEY_RANGE = 400000;

    std::vector<uint>  a_keys = random_sorted_unique_keys(N, KEY_RANGE);
    std::vector<uint>  b_keys = random_sorted_unique_keys(N, KEY_RANGE);
    std::vector<float> a_vals(N);
    std::vector<float> b_vals(N);

    for (uint i = 0; i < N; ++i) {
        a_vals[i] = my_rand(0.0f, 1000.0f);
        b_vals[i] = my_rand(0.0f, 1000.0f);
    }

    auto a_keys_vec = spla::Vector::make(a_keys.size(), spla::UINT);
    auto a_vals_vec = spla::Vector::make(a_vals.size(), spla::FLOAT);
    auto b_keys_vec = spla::Vector::make(b_keys.size(), spla::UINT);
    auto b_vals_vec = spla::Vector::make(b_vals.size(), spla::FLOAT);

    fill_vector(a_keys_vec, a_keys);
    fill_vector(a_vals_vec, a_vals);
    fill_vector(b_keys_vec, b_keys);
    fill_vector(b_vals_vec, b_vals);

    auto r_keys = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::UINT);
    auto r_vals = spla::Vector::make(std::min(a_keys.size(), b_keys.size()), spla::FLOAT);

    auto op = spla::PLUS_FLOAT.template cast_safe<spla::OpBinary>();

    auto status = spla::exec_intersect(a_keys_vec, a_vals_vec, b_keys_vec, b_vals_vec, r_keys, r_vals, op);
    ASSERT_EQ(status, spla::Status::Ok);

    auto got      = collect_sorted<float>(r_keys, r_vals);
    auto expected = reference_intersect<float>(a_keys, a_vals, b_keys, b_vals,
                                               [](float a, float b) { return a + b; });

    EXPECT_GT(got.size(), 0u);
    expect_result(got, expected);
}

SPLA_GTEST_MAIN_WITH_FINALIZE
