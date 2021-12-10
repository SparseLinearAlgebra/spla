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

    /**
     * @addtogroup API
     * @{
     */

#define SPLA_DECLARE_BINARY_OPERATOR(name, typeName) \
    static RefPtr<FunctionBinary> name##typeName(Library &library);

#define SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(name, typeName) \
    static RefPtr<FunctionBinary> name##typeName(Library &library);

#define SPLA_DECLARE_TAKE_FIRST(name, typeName) \
    static RefPtr<FunctionBinary> name##typeName(Library &library);

#define SPLA_DECLARE_TAKE_SECOND(name, typeName) \
    static RefPtr<FunctionBinary> name##typeName(Library &library);

#define SPLA_DECLARE_BINARY_FUNCTION(name, typeName) \
    static RefPtr<FunctionBinary> name##typeName(Library &library);

#define SPLA_DECLARE_BINARY_OPERATORS(typeName)                  \
    SPLA_DECLARE_BINARY_OPERATOR(Plus, typeName)                 \
    SPLA_DECLARE_BINARY_OPERATOR(Minus, typeName)                \
    SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(ReverseMinus, typeName) \
    SPLA_DECLARE_BINARY_OPERATOR(Mult, typeName)                 \
    SPLA_DECLARE_BINARY_OPERATOR(Div, typeName)                  \
    SPLA_DECLARE_FLIPPED_BINARY_OPERATOR(ReverseDiv, typeName)   \
    SPLA_DECLARE_TAKE_FIRST(TakeFirst, typeName)                 \
    SPLA_DECLARE_TAKE_SECOND(TakeSecond, typeName)

#define SPLA_DECLARE_LOGIC_OPERATORS(typeName)  \
    SPLA_DECLARE_BINARY_OPERATOR(Or, typeName)  \
    SPLA_DECLARE_BINARY_OPERATOR(And, typeName) \
    SPLA_DECLARE_BINARY_OPERATOR(Xor, typeName)

#define SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(typeName) \
    SPLA_DECLARE_BINARY_FUNCTION(Min, typeName)       \
    SPLA_DECLARE_BINARY_FUNCTION(Max, typeName)

#define SPLA_DECLARE_INT_BINARY_FUNCTIONS(typeName) \
    SPLA_DECLARE_BINARY_FUNCTION(Min, typeName)     \
    SPLA_DECLARE_BINARY_FUNCTION(Max, typeName)

#define SPLA_DECLARE_INT_FUNCTIONS(MACRO, typeNamePrefix) \
    MACRO(typeNamePrefix##8)                              \
    MACRO(typeNamePrefix##16)                             \
    MACRO(typeNamePrefix##32)                             \
    MACRO(typeNamePrefix##64)

    /**
     * @class Functions
     * @brief Predefined functions for built-in types.
     * Allows to access standard plus, minus, min, max function for built-in types.
     */
    class SPLA_API Functions {
    public:
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_BINARY_OPERATORS, Int)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_LOGIC_OPERATORS, Int)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_INT_BINARY_FUNCTIONS, Int)

        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_BINARY_OPERATORS, UInt)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_LOGIC_OPERATORS, UInt)
        SPLA_DECLARE_INT_FUNCTIONS(SPLA_DECLARE_INT_BINARY_FUNCTIONS, UInt)

        SPLA_DECLARE_BINARY_OPERATORS(Float32)
        SPLA_DECLARE_BINARY_OPERATORS(Float64)

        SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(Float32)
        SPLA_DECLARE_FLOAT_BINARY_FUNCTIONS(Float64)

    private:
        // friend class Library;
        // todo: static void InitFunctions(Library &library);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONS_HPP
