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

TEST(library, default_log) {
    spla::Library::get()->set_default_callback();
    spla::Library::get()->finalize();
}

TEST(library, default_accelerator) {
    spla::Library::get()->set_accelerator(spla::AcceleratorType::OpenCL);
    spla::Library::get()->finalize();
}

TEST(library, default_accelerator_select_props) {
    spla::Library::get()->set_accelerator(spla::AcceleratorType::OpenCL);
    spla::Library::get()->set_platform(1);
    spla::Library::get()->set_device(0);
    spla::Library::get()->finalize();
}

TEST(library, default_types) {
    EXPECT_EQ(spla::INT->get_size(), 4);
    EXPECT_EQ(spla::UINT->get_size(), 4);
    EXPECT_EQ(spla::FLOAT->get_size(), 4);

    std::cout << "Type " << spla::INT->get_description() << " id=" << spla::INT->get_id() << std::endl;
    std::cout << "Type " << spla::UINT->get_description() << " id=" << spla::UINT->get_id() << std::endl;
    std::cout << "Type " << spla::FLOAT->get_description() << " id=" << spla::FLOAT->get_id() << std::endl;
}

TEST(library, default_accelerator_info) {
    std::string acc_info;

    spla::Library::get()->set_accelerator(spla::AcceleratorType::OpenCL);
    spla::Library::get()->get_accelerator_info(acc_info);
    spla::Library::get()->finalize();

    std::cout << "Info: " << acc_info << std::endl;
}

SPLA_GTEST_MAIN