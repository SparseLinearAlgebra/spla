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

TEST(single_path_cfpq, anbn) {
    if (!ensure_opencl()) GTEST_SKIP() << "OpenCL accelerator is not available";

    // graph: a: 0->1, 1->2, 2->0; b: 0->3, 3->0.
    auto adj_a = spla::Matrix::make(4, 4, spla::INT);
    adj_a->set_int(0, 1, 1);
    adj_a->set_int(1, 2, 1);
    adj_a->set_int(2, 0, 1);

    auto adj_b = spla::Matrix::make(4, 4, spla::INT);
    adj_b->set_int(0, 3, 1);
    adj_b->set_int(3, 0, 1);

    // grammar (nonterminals S=0, S1=1, A=2, B=3):
    // S -> AB, S -> A S1, S1 -> S B, A -> a, B -> b.
    std::vector<std::array<uint, 3>>   binary   = {{0, 2, 3}, {0, 2, 1}, {1, 0, 3}};
    std::vector<std::pair<uint, uint>> terminal = {{2, 0}, {3, 1}};

    std::vector<spla::ref_ptr<spla::Matrix>> T;
    spla::single_path_cfpq(T, {adj_a, adj_b}, binary, terminal, {}, 4);

    auto expect = [&](uint i, uint j, spla::PathIndex p) {
        spla::PathIndex v;
        T[0]->get_path_index(i, j, v);
        EXPECT_EQ(v, p);
    };

    expect(0, 0, spla::PathIndex(1, 12));
    expect(0, 3, spla::PathIndex(1, 6));
    expect(1, 0, spla::PathIndex(2, 4));
    expect(1, 3, spla::PathIndex(2, 10));
    expect(2, 0, spla::PathIndex(0, 8));
    expect(2, 3, spla::PathIndex(0, 2));
    expect(0, 1, spla::PathIndex(0, 0));
    expect(1, 1, spla::PathIndex(0, 0));
}

SPLA_GTEST_MAIN_WITH_FINALIZE
