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

#include <spla-cpp/SplaFunctions.hpp>

namespace {
    template<typename T>
    std::string MakeBody(const char *clType, T apply) {
        std::stringstream stream;
        stream << clType << " a = *((_ACCESS_A const " << clType << "*)vp_a);\n"
               << clType << " b = *((_ACCESS_A const " << clType << "*)vp_b);\n"
               << "_ACCESS_C " << clType << "* c = (_ACCESS_C " << clType << "*)vp_c;\n"
               << "*c = ";
        apply(stream);
        stream << ";";
        return stream.str();
    }

    std::string MakeBinaryFunctionBody(const char *clType, const char *clFun) {
        return MakeBody(clType, [&](std::ostream &os) {
            os << clFun << "(a, b)";
        });
    }

    std::string MakeBinaryOperatorBody(const char *clType, const char *clOp) {
        return MakeBody(clType, [&](std::ostream &os) {
            os << "a " << clOp << " b";
        });
    }

    std::string MakeFlippedBinaryOperatorBody(const char *clType, const char *clOp) {
        return MakeBody(clType, [&](std::ostream &os) {
            os << "b " << clOp << " a";
        });
    }

    std::string MakeFirstBinaryOperatorBody(const char *clType) {
        return MakeBody(clType, [&](std::ostream &os) {
            os << "a";
        });
    }

    std::string MakeSecondBinaryOperatorBody(const char *clType) {
        return MakeBody(clType, [&](std::ostream &os) {
            os << "b";
        });
    }
}// namespace

/**
 * @addtogroup API
 * @{
 */

#define SPLA_DEFINE_BINARY_OPERATOR(name, typeName, clType, op)                              \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::name##typeName(Library &library) {   \
        auto t = Types::typeName(library);                                                   \
        return FunctionBinary::Make(t, t, t, MakeBinaryOperatorBody(#clType, #op), library); \
    }

#define SPLA_DEFINE_FLIPPED_BINARY_OPERATOR(name, typeName, clType, op)                             \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::name##typeName(Library &library) {          \
        auto t = Types::typeName(library);                                                          \
        return FunctionBinary::Make(t, t, t, MakeFlippedBinaryOperatorBody(#clType, #op), library); \
    }

#define SPLA_DEFINE_TAKE_FIRST(name, typeName, clType)                                       \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::name##typeName(Library &library) {   \
        auto t = Types::typeName(library);                                                   \
        return FunctionBinary::Make(t, t, t, MakeFirstBinaryOperatorBody(#clType), library); \
    }

#define SPLA_DEFINE_TAKE_SECOND(name, typeName, clType)                                       \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::name##typeName(Library &library) {    \
        auto t = Types::typeName(library);                                                    \
        return FunctionBinary::Make(t, t, t, MakeSecondBinaryOperatorBody(#clType), library); \
    }

#define SPLA_DEFINE_BINARY_FUNCTION(name, typeName, type, fun)                              \
    spla::RefPtr<spla::FunctionBinary> spla::Functions::name##typeName(Library &library) {  \
        auto t = Types::typeName(library);                                                  \
        return FunctionBinary::Make(t, t, t, MakeBinaryFunctionBody(#type, #fun), library); \
    }

#define SPLA_DEFINE_BINARY_OPERATORS(typeName, clType)                     \
    SPLA_DEFINE_BINARY_OPERATOR(Plus, typeName, clType, +)                 \
    SPLA_DEFINE_BINARY_OPERATOR(Minus, typeName, clType, -)                \
    SPLA_DEFINE_FLIPPED_BINARY_OPERATOR(ReverseMinus, typeName, clType, -) \
    SPLA_DEFINE_BINARY_OPERATOR(Mult, typeName, clType, *)                 \
    SPLA_DEFINE_BINARY_OPERATOR(Div, typeName, clType, /)                  \
    SPLA_DEFINE_FLIPPED_BINARY_OPERATOR(ReverseDiv, typeName, clType, /)   \
    SPLA_DEFINE_TAKE_FIRST(TakeFirst, typeName, clType)                    \
    SPLA_DEFINE_TAKE_SECOND(TakeSecond, typeName, clType)

#define SPLA_DEFINE_LOGIC_OPERATORS(typeName, clType)     \
    SPLA_DEFINE_BINARY_OPERATOR(Or, typeName, clType, |)  \
    SPLA_DEFINE_BINARY_OPERATOR(And, typeName, clType, &) \
    SPLA_DEFINE_BINARY_OPERATOR(Xor, typeName, clType, ^)

#define SPLA_DEFINE_FLOAT_BINARY_FUNCTIONS(typeName, clType) \
    SPLA_DEFINE_BINARY_FUNCTION(Min, typeName, clType, fmin) \
    SPLA_DEFINE_BINARY_FUNCTION(Max, typeName, clType, fmax)

#define SPLA_DEFINE_INT_BINARY_FUNCTIONS(typeName, clType)  \
    SPLA_DEFINE_BINARY_FUNCTION(Min, typeName, clType, min) \
    SPLA_DEFINE_BINARY_FUNCTION(Max, typeName, clType, max)

#define SPLA_DEFINE_INT_FUNCTIONS(MACRO, typeNamePrefix, clTypePrefix) \
    MACRO(typeNamePrefix##8, clTypePrefix##char)                       \
    MACRO(typeNamePrefix##16, clTypePrefix##short)                     \
    MACRO(typeNamePrefix##32, clTypePrefix##int)                       \
    MACRO(typeNamePrefix##64, clTypePrefix##long)


SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_BINARY_OPERATORS, Int, )
SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_LOGIC_OPERATORS, Int, )
SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_INT_BINARY_FUNCTIONS, Int, )

SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_BINARY_OPERATORS, UInt, u)
SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_LOGIC_OPERATORS, UInt, u)
SPLA_DEFINE_INT_FUNCTIONS(SPLA_DEFINE_INT_BINARY_FUNCTIONS, UInt, u)

SPLA_DEFINE_BINARY_OPERATORS(Float32, float)
SPLA_DEFINE_BINARY_OPERATORS(Float64, double)

SPLA_DEFINE_FLOAT_BINARY_FUNCTIONS(Float32, float)
SPLA_DEFINE_FLOAT_BINARY_FUNCTIONS(Float64, double)
