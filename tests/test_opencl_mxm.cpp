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

SPLA_GTEST_MAIN_WITH_FINALIZE
