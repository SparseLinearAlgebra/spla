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

#include <spla-cpp/SplaFunctions.hpp>
#include <spla-cpp/SplaTypes.hpp>
#include <sstream>

namespace {
    std::string MakeBody(const char *clType, const char *clOp) {
        std::stringstream stream;
        stream << clType << " a = *((_ACCESS_A const " << clType << "*)vp_a);\n"
               << clType << " b = *((_ACCESS_A const " << clType << "*)vp_b);\n"
               << "_ACCESS_C " << clType << "* c = (_ACCESS_C " << clType << "*)vp_c;\n"
               << "*c = a " << clOp << " b;";
        return stream.str();
    }
}// namespace

#define SPLA_MAKE_BINARY_FUNCTION(opName, typeName, type, binop)                                   \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::opName##typeName(spla::Library &library) { \
        auto t = Types::typeName(library);                                                         \
        return FunctionBinary::Make(t, t, t, MakeBody(#type, #binop), library);                    \
    }

SPLA_MAKE_BINARY_FUNCTION(Plus, Int32, int, +)
SPLA_MAKE_BINARY_FUNCTION(Plus, Int64, long, +)
SPLA_MAKE_BINARY_FUNCTION(Plus, Float32, float, +)
SPLA_MAKE_BINARY_FUNCTION(Plus, Float64, double, +)

//SPLA_MAKE_BINARY_FUNCTION(Plus, UInt32, unsigned int, +)
//SPLA_MAKE_BINARY_FUNCTION(Plus, UInt64, long, +)

SPLA_MAKE_BINARY_FUNCTION(Mult, Int32, int, *)
SPLA_MAKE_BINARY_FUNCTION(Mult, Int64, long, *)
SPLA_MAKE_BINARY_FUNCTION(Mult, Float32, float, *)
SPLA_MAKE_BINARY_FUNCTION(Mult, Float64, double, *)
