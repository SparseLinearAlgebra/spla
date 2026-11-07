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

#include <opencl/cl_mxm.hpp>

#include <limits>

using uint = spla::uint;

inline bool ensure_opencl() {
    auto* lib = spla::Library::get();
    if (lib->get_accelerator() != nullptr) {
        return true;
    }
    return lib->set_accelerator(spla::AcceleratorType::OpenCL) == spla::Status::Ok &&
           lib->get_accelerator() != nullptr;
}

TEST(opencl_mxm, pool_estimate) {
    const uint a_rows = 3, a_nnz = 6, b_rows = 3, b_nnz = 6, b_cols = 3;

    // CPU reference of the GALATIC formula (GALATIC/source/device/Multiply.cuh).
    const double overalloc = 1.1;
    const uint   retain    = 4;
    const uint   threads   = 256;

    const double a_avg   = static_cast<double>(a_nnz) / static_cast<double>(a_rows);
    const double b_avg   = static_cast<double>(b_nnz) / static_cast<double>(b_rows);
    const double overlap = b_avg / static_cast<double>(b_cols);

    const double output_estimate =
            overalloc * a_rows * b_avg * (1.0 - std::pow(1.0 - overlap, a_avg)) / overlap;

    double single_chunk = b_avg;
    double cur_overlap  = overlap;
    double merges       = 1.0;
    for (; merges < a_avg + 1.0; merges += 1.0) {
        if (single_chunk >= static_cast<double>(retain * threads)) break;
        single_chunk += (1.0 - cur_overlap) * b_avg;
        cur_overlap = cur_overlap + (1.0 - cur_overlap) * overlap;
    }
    const double intermediate_estimate =
            overalloc * a_avg / std::min(merges, a_avg) * single_chunk * a_rows;

    const double expected =
            std::max(10000000.0, std::min(500000000.0, std::max(intermediate_estimate, output_estimate)));

    const std::size_t got = spla::mxm_detail::estimate_pool_elements(a_rows, a_nnz, b_rows, b_nnz, b_cols);

    EXPECT_EQ(got, static_cast<std::size_t>(expected));
    EXPECT_GE(got, static_cast<std::size_t>(10000000));
    EXPECT_LE(got, static_cast<std::size_t>(500000000));
}

TEST(opencl_mxm, empty) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto init = spla::Scalar::make_float(0.0f);

    // A empty, B non-empty -> R empty
    {
        auto A = spla::Matrix::make(3, 3, spla::FLOAT);
        auto B = spla::Matrix::make(3, 3, spla::FLOAT);
        auto R = spla::Matrix::make(3, 3, spla::FLOAT);

        B->set_float(0, 0, 1.0f);

        ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

        float v;
        EXPECT_EQ(R->get_float(0, 0, v), spla::Status::Ok);
        EXPECT_EQ(v, 0.0f);
    }

    // A non-empty, B empty -> R empty
    {
        auto A = spla::Matrix::make(3, 3, spla::FLOAT);
        auto B = spla::Matrix::make(3, 3, spla::FLOAT);
        auto R = spla::Matrix::make(3, 3, spla::FLOAT);

        A->set_float(0, 0, 1.0f);

        ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

        float v;
        EXPECT_EQ(R->get_float(0, 0, v), spla::Status::Ok);
        EXPECT_EQ(v, 0.0f);
    }

    // both empty -> R empty
    {
        auto A = spla::Matrix::make(3, 3, spla::FLOAT);
        auto B = spla::Matrix::make(3, 3, spla::FLOAT);
        auto R = spla::Matrix::make(3, 3, spla::FLOAT);

        ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

        float v;
        EXPECT_EQ(R->get_float(2, 2, v), spla::Status::Ok);
        EXPECT_EQ(v, 0.0f);
    }
}

TEST(opencl_mxm, incompatible) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A    = spla::Matrix::make(3, 4, spla::FLOAT);
    auto B    = spla::Matrix::make(5, 2, spla::FLOAT);
    auto R    = spla::Matrix::make(3, 2, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0f);

    EXPECT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::InvalidArgument);
}

TEST(opencl_mxm, semiring_minplus) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A    = spla::Matrix::make(2, 2, spla::FLOAT);
    auto B    = spla::Matrix::make(2, 2, spla::FLOAT);
    auto R    = spla::Matrix::make(2, 2, spla::FLOAT);
    auto init = spla::Scalar::make_float(std::numeric_limits<float>::infinity());

    A->set_float(0, 0, 1.0f);
    A->set_float(0, 1, 3.0f);
    A->set_float(1, 0, 2.0f);
    A->set_float(1, 1, 1.0f);

    B->set_float(0, 0, 1.0f);
    B->set_float(0, 1, 2.0f);
    B->set_float(1, 0, 4.0f);
    B->set_float(1, 1, 1.0f);

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::PLUS_FLOAT, spla::MIN_FLOAT, init), spla::Status::Ok);

    float v;
    R->get_float(0, 0, v);
    EXPECT_FLOAT_EQ(v, 2.0f);
    R->get_float(0, 1, v);
    EXPECT_FLOAT_EQ(v, 3.0f);
    R->get_float(1, 0, v);
    EXPECT_FLOAT_EQ(v, 3.0f);
    R->get_float(1, 1, v);
    EXPECT_FLOAT_EQ(v, 2.0f);
}

TEST(opencl_mxm, semiring_bool) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A    = spla::Matrix::make(2, 2, spla::INT);
    auto B    = spla::Matrix::make(2, 2, spla::INT);
    auto R    = spla::Matrix::make(2, 2, spla::INT);
    auto init = spla::Scalar::make_int(0);

    A->set_int(0, 1, 1);
    B->set_int(1, 0, 1);

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::LAND_INT, spla::LOR_INT, init), spla::Status::Ok);

    int v;
    R->get_int(0, 0, v);
    EXPECT_EQ(v, 1);
    R->get_int(0, 1, v);
    EXPECT_EQ(v, 0);
    R->get_int(1, 0, v);
    EXPECT_EQ(v, 0);
    R->get_int(1, 1, v);
    EXPECT_EQ(v, 0);
}

TEST(opencl_mxm, rectangular) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A    = spla::Matrix::make(2, 3, spla::FLOAT);
    auto B    = spla::Matrix::make(3, 4, spla::FLOAT);
    auto R    = spla::Matrix::make(2, 4, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0f);

    A->set_float(0, 0, 1.0f);
    A->set_float(0, 2, 2.0f);

    B->set_float(0, 0, 1.0f);
    B->set_float(0, 3, 2.0f);
    B->set_float(2, 1, 3.0f);

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

    float v;
    R->get_float(0, 0, v);
    EXPECT_FLOAT_EQ(v, 1.0f);
    R->get_float(0, 1, v);
    EXPECT_FLOAT_EQ(v, 6.0f);
    R->get_float(0, 2, v);
    EXPECT_FLOAT_EQ(v, 0.0f);
    R->get_float(0, 3, v);
    EXPECT_FLOAT_EQ(v, 2.0f);
}

TEST(opencl_mxm, path_zero_drop) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A    = spla::Matrix::make(1, 2, spla::FLOAT);
    auto B    = spla::Matrix::make(2, 2, spla::FLOAT);
    auto R    = spla::Matrix::make(1, 2, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0f);

    A->set_float(0, 0, 1.0f);
    A->set_float(0, 1, -1.0f);

    B->set_float(0, 0, 1.0f);
    B->set_float(1, 0, 1.0f);

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

    float v;
    EXPECT_EQ(R->get_float(0, 0, v), spla::Status::Ok);
    EXPECT_FLOAT_EQ(v, 0.0f);
    EXPECT_EQ(R->get_float(0, 1, v), spla::Status::Ok);
    EXPECT_FLOAT_EQ(v, 0.0f);
}

TEST(opencl_mxm, direct) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    const uint K = 1500;

    auto A    = spla::Matrix::make(1, 1, spla::FLOAT);
    auto B    = spla::Matrix::make(1, K, spla::FLOAT);
    auto R    = spla::Matrix::make(1, K, spla::FLOAT);
    auto init = spla::Scalar::make_float(0.0f);

    A->set_float(0, 0, 2.0f);
    for (uint j = 0; j < K; ++j) {
        B->set_float(0, j, static_cast<float>(j + 1));
    }

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MULT_FLOAT, spla::PLUS_FLOAT, init), spla::Status::Ok);

    float v;
    R->get_float(0, 0, v);
    EXPECT_FLOAT_EQ(v, 2.0f);
    R->get_float(0, 1, v);
    EXPECT_FLOAT_EQ(v, 4.0f);
    R->get_float(0, K - 1, v);
    EXPECT_FLOAT_EQ(v, 2.0f * static_cast<float>(K));
}

SPLA_GTEST_MAIN_WITH_FINALIZE
