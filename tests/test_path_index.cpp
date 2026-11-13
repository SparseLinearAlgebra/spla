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

inline bool ensure_opencl() {
    auto* lib = spla::Library::get();
    if (lib->get_accelerator() != nullptr) {
        return true;
    }
    return lib->set_accelerator(spla::AcceleratorType::OpenCL) == spla::Status::Ok &&
           lib->get_accelerator() != nullptr;
}

TEST(path_index, type) {
    auto s = spla::Scalar::make(spla::PATH_INDEX);

    const spla::PathIndex p(3, 2);
    ASSERT_EQ(s->set_path_index(p), spla::Status::Ok);

    spla::PathIndex q;
    ASSERT_EQ(s->get_path_index(q), spla::Status::Ok);

    EXPECT_EQ(q.middle, 3u);
    EXPECT_EQ(q.height, 2u);
    EXPECT_TRUE(p == q);
    EXPECT_FALSE(p != q);
}

TEST(path_index, add) {
    const spla::PathIndex bottom(0, 0);

    EXPECT_EQ(spla::path_index_add(spla::PathIndex(5, 2), spla::PathIndex(9, 3)), spla::PathIndex(5, 2));
    EXPECT_EQ(spla::path_index_add(spla::PathIndex(5, 2), spla::PathIndex(3, 2)), spla::PathIndex(3, 2));
    EXPECT_EQ(spla::path_index_add(spla::PathIndex(5, 2), bottom), spla::PathIndex(5, 2));
    EXPECT_EQ(spla::path_index_add(bottom, bottom), bottom);
}

TEST(path_index, mult) {
    const spla::PathIndex bottom(0, 0);

    EXPECT_EQ(spla::path_index_multiply(spla::PathIndex(1, 2), 7, spla::PathIndex(2, 4)), spla::PathIndex(7, 5));
    EXPECT_EQ(spla::path_index_multiply(bottom, 7, spla::PathIndex(3, 1)), bottom);
    EXPECT_EQ(spla::path_index_multiply(spla::PathIndex(3, 1), 7, bottom), bottom);
}

TEST(path_index, init) {
    auto T = spla::Matrix::make(2, 2, spla::PATH_INDEX);

    T->set_path_index(0, 1, spla::PathIndex(0, 1));
    T->set_path_index(0, 0, spla::PathIndex(0, 1));
    T->set_path_index(1, 1, spla::PathIndex(1, 1));

    spla::PathIndex v;
    T->get_path_index(0, 1, v);
    EXPECT_EQ(v, spla::PathIndex(0, 1));
    T->get_path_index(1, 1, v);
    EXPECT_EQ(v, spla::PathIndex(1, 1));
}

TEST(path_index, zero_drop) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    auto A = spla::Matrix::make(1, 2, spla::PATH_INDEX);
    auto B = spla::Matrix::make(2, 2, spla::PATH_INDEX);
    auto R = spla::Matrix::make(1, 2, spla::PATH_INDEX);

    auto init = spla::Scalar::make(spla::PATH_INDEX);
    ASSERT_EQ(init->set_path_index(spla::PathIndex(0, 0)), spla::Status::Ok);

    A->set_path_index(0, 1, spla::PathIndex(0, 1));
    B->set_path_index(1, 0, spla::PathIndex(0, 1));

    ASSERT_EQ(spla::exec_mxm(R, A, B, spla::MUL_PATH_INDEX, spla::MIN_PATH_INDEX, init), spla::Status::Ok);

    spla::PathIndex v;
    R->get_path_index(0, 0, v);
    EXPECT_EQ(v, spla::PathIndex(1, 2));

    // B empty -> R empty
    auto B2 = spla::Matrix::make(2, 2, spla::PATH_INDEX);
    auto R2 = spla::Matrix::make(1, 2, spla::PATH_INDEX);
    ASSERT_EQ(spla::exec_mxm(R2, A, B2, spla::MUL_PATH_INDEX, spla::MIN_PATH_INDEX, init), spla::Status::Ok);

    spla::PathIndex w;
    R2->get_path_index(0, 0, w);
    EXPECT_EQ(w, spla::PathIndex(0, 0));
}

SPLA_GTEST_MAIN_WITH_FINALIZE
