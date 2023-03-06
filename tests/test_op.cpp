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

#include <iostream>

static constexpr auto display_op_info = [](spla::ref_ptr<spla::OpBinary>& op) {
    std::cout << op->get_name() << " "
              << op->get_key() << " "
              << op->get_source_cl() << std::endl;
};

TEST(op_binary, info_built_in) {
    spla::Library::get();

    display_op_info(spla::PLUS_INT);
    display_op_info(spla::MULT_INT);

    display_op_info(spla::PLUS_UINT);
    display_op_info(spla::MULT_UINT);

    display_op_info(spla::MIN_FLOAT);
    display_op_info(spla::ONE_FLOAT);
}

TEST(op_binary, custom) {
    spla::Library::get();

    spla::ref_ptr<spla::OpBinary> custom_plus = spla::OpBinary::make_float(
            "custom_plus",
            "(float a, float b) { return 0.25f * a + 0.75f * b; }",
            [](float a, float b) { return 0.25f * a + 0.75f * b; });

    display_op_info(custom_plus);
}

SPLA_GTEST_MAIN_WITH_FINALIZE