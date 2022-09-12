/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
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

#include <spla/spla.hpp>

TEST(library, default_log) {
    spla::get_library()->set_default_callback();
    spla::get_library()->finalize();
}

TEST(library, default_accelerator) {
    spla::get_library()->set_accelerator(spla::AcceleratorType::OpenCL);
    spla::get_library()->finalize();
}

TEST(library, default_accelerator_select_props) {
    spla::get_library()->set_accelerator(spla::AcceleratorType::OpenCL);
    spla::get_library()->set_platform(1);
    spla::get_library()->set_device(0);
    spla::get_library()->finalize();
}

TEST(library, default_types) {
    EXPECT_EQ(spla::BYTE->get_size(), 1);
    EXPECT_EQ(spla::INT->get_size(), 4);
    EXPECT_EQ(spla::UINT->get_size(), 4);
    EXPECT_EQ(spla::FLOAT->get_size(), 4);

    std::cout << "Type " << spla::BYTE->get_description() << " id=" << spla::BYTE->get_id() << std::endl;
    std::cout << "Type " << spla::INT->get_description() << " id=" << spla::INT->get_id() << std::endl;
    std::cout << "Type " << spla::UINT->get_description() << " id=" << spla::UINT->get_id() << std::endl;
    std::cout << "Type " << spla::FLOAT->get_description() << " id=" << spla::FLOAT->get_id() << std::endl;
}

SPLA_GTEST_MAIN