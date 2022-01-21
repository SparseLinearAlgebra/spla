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

#include <sstream>

#include <Testing.hpp>

#include <spla-cpp/SplaUtils.hpp>

TEST(LoadMatrix, Int) {
    std::string content =
            "% hello, this\n"
            "% is \n"
            "% a comment :)\n"
            "4 5 3\n"
            "1  1  1\n"
            "1 2  4\n"
            "4 5 2\n";
    {
        std::istringstream is(content.data());
        spla::MatrixLoader<int> loader;
        loader.Load<int>(is);
        EXPECT_EQ(loader.GetNnvals(), 3);
        EXPECT_EQ(loader.GetNcols(), 5);
        EXPECT_EQ(loader.GetNrows(), 4);
        EXPECT_EQ(loader.GetRowIndices(), (std::vector<spla::Index>{1, 1, 4}));
        EXPECT_EQ(loader.GetColIndices(), (std::vector<spla::Index>{1, 2, 5}));
        EXPECT_EQ(loader.GetValues(), (std::vector<int>{1, 4, 2}));
    }
    {
            // // Should not compile
            //        std::istrstream is(content.data());
            //        spla::MatrixLoader<int> loader;
            //        loader.Load<void>(is);
    } {
        std::istringstream is(content.data());
        spla::MatrixLoader<void> loader;
        loader.Load<int>(is);
        EXPECT_EQ(loader.GetNnvals(), 3);
        EXPECT_EQ(loader.GetNcols(), 5);
        EXPECT_EQ(loader.GetNrows(), 4);
        EXPECT_EQ(loader.GetRowIndices(), (std::vector<spla::Index>{1, 1, 4}));
        EXPECT_EQ(loader.GetColIndices(), (std::vector<spla::Index>{1, 2, 5}));
        EXPECT_EQ(loader.GetValues(), nullptr);
    }
}


TEST(LoadMatrix, Void) {
    std::string content =
            "% hello, this\n"
            "% is \n"
            "% a comment :)\n"
            "4 5 3\n"
            "1 1\n"
            "1 2\n"
            "4 5";
    {
        std::istringstream is(content.data());
        spla::MatrixLoader<void> loader;
        loader.Load<void>(is);
        EXPECT_EQ(loader.GetNnvals(), 3);
        EXPECT_EQ(loader.GetNcols(), 5);
        EXPECT_EQ(loader.GetNrows(), 4);
        EXPECT_EQ(loader.GetRowIndices(), (std::vector<spla::Index>{1, 1, 4}));
        EXPECT_EQ(loader.GetColIndices(), (std::vector<spla::Index>{1, 2, 5}));
        EXPECT_EQ(loader.GetValues(), nullptr);
    }
}

SPLA_GTEST_MAIN
