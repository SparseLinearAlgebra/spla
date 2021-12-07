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

#ifndef SPLA_SPLAFUNCTIONS_HPP
#define SPLA_SPLAFUNCTIONS_HPP


#include <sstream>

#include <spla-cpp/SplaFunctionBinary.hpp>
#include <spla-cpp/SplaFunctionUnary.hpp>
#include <spla-cpp/SplaTypes.hpp>


namespace spla {

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

#define SPLA_DECLARE_BINARY_OPERATOR(name, typeName, clType, op)                             \
    static RefPtr<FunctionBinary> name##typeName(Library &library) {                         \
        auto t = Types::typeName(library);                                                   \
        return FunctionBinary::Make(t, t, t, MakeBinaryOperatorBody(#clType, #op), library); \
    }

#define SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(name, typeName, clType, op)                            \
    static RefPtr<FunctionBinary> name##typeName(Library &library) {                                \
        auto t = Types::typeName(library);                                                          \
        return FunctionBinary::Make(t, t, t, MakeFlippedBinaryOperatorBody(#clType, #op), library); \
    }

#define SPLA_DECLARE_TAKE_FIRST(name, typeName, clType)                                      \
    static RefPtr<FunctionBinary> name##typeName(Library &library) {                         \
        auto t = Types::typeName(library);                                                   \
        return FunctionBinary::Make(t, t, t, MakeFirstBinaryOperatorBody(#clType), library); \
    }

#define SPLA_DECLARE_TAKE_SECOND(name, typeName, clType)                                      \
    static RefPtr<FunctionBinary> name##typeName(Library &library) {                          \
        auto t = Types::typeName(library);                                                    \
        return FunctionBinary::Make(t, t, t, MakeSecondBinaryOperatorBody(#clType), library); \
    }

#define SPLA_DECLARE_BINARY_FUNCTION(name, typeName, type, fun)                             \
    static RefPtr<FunctionBinary> name##typeName(Library &library) {                        \
        auto t = Types::typeName(library);                                                  \
        return FunctionBinary::Make(t, t, t, MakeBinaryFunctionBody(#type, #fun), library); \
    }

#define SPLA_DECLARE_BINARY_OPERATORS(typeName, clType)                     \
    SPLA_DECLARE_BINARY_OPERATOR(Plus, typeName, clType, +)                 \
    SPLA_DECLARE_BINARY_OPERATOR(Minus, typeName, clType, -)                \
    SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(ReverseMinus, typeName, clType, -) \
    SPLA_DECLARE_BINARY_OPERATOR(Mult, typeName, clType, *)                 \
    SPLA_DECLARE_BINARY_OPERATOR(Div, typeName, clType, /)                  \
    SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(ReverseDiv, typeName, clType, /)   \
    SPLA_DECLARE_TAKE_FIRST(TakeFirst, typeName, clType)                    \
    SPLA_DECLARE_TAKE_SECOND(TakeSecond, typeName, clType)

#define SPLA_DECLARE_LOGIC_OPERATORS(typeName, clType)     \
    SPLA_DECLARE_BINARY_OPERATOR(Or, typeName, clType, |)  \
    SPLA_DECLARE_BINARY_OPERATOR(And, typeName, clType, &) \
    SPLA_DECLARE_BINARY_OPERATOR(Xor, typeName, clType, ^)

#define SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(typeName, clType) \
    SPLA_DECLARE_BINARY_FUNCTION(Min, typeName, clType, fmin) \
    SPLA_DECLARE_BINARY_FUNCTION(Max, typeName, clType, fmax)

#define SPLA_DECLARE_INT_BINARY_FUNCTIONS(typeName, clType)  \
    SPLA_DECLARE_BINARY_FUNCTION(Min, typeName, clType, min) \
    SPLA_DECLARE_BINARY_FUNCTION(Max, typeName, clType, max)

#define SPLA_DECLARE_INT_FUNCTIONS(MACRO, typeNamePrefix, clTypePrefix) \
    MACRO(typeNamePrefix##8, clTypePrefix##char)                        \
    MACRO(typeNamePrefix##16, clTypePrefix##short)                      \
    MACRO(typeNamePrefix##32, clTypePrefix##int)                        \
    MACRO(typeNamePrefix##64, clTypePrefix##long)

    /**
     * @class Functions
     * @brief Predefined functions for built-in types.
     * Allows to access standard plus, minus, min, max function for built-in types.
     */
    class SPLA_API Functions {
    public:
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_BINARY_OPERATORS, Int, )
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_LOGIC_OPERATORS, Int, )
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_INT_BINARY_FUNCTIONS, Int, )

        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_BINARY_OPERATORS, UInt, u)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_LOGIC_OPERATORS, UInt, u)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_INT_BINARY_FUNCTIONS, UInt, u)

        SPLA_DECLARE_BINARY_OPERATORS(Float32, float)
        SPLA_DECLARE_BINARY_OPERATORS(Float64, double)

        SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(Float32, float)
        SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(Float64, double)

    private:
        // friend class Library;
        // todo: static void InitFunctions(Library &library);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONS_HPP
