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

#ifndef SPLA_SPLAFUNCTIONUNARY_HPP
#define SPLA_SPLAFUNCTIONUNARY_HPP

#include <bitset>
#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class FunctionUnary
     * @brief Unary Typed element function f: a -> b.
     *
     * Unary function accepts value of typed input object, with type
     * `A` and returns new value for typed object with type `B`.
     * Can be used as unary op in transform operations for matrices, vectors and scalars.
     *
     * Source code for the function must be provided in the string.
     * Function signature is following `void(_ACCESS_A const void* vp_a, _ACCESS_B void* vp_b)`,
     * where a, and b pointers to values, b must be written by the function.
     * It is up to user to cast this pointers to appropriate types and check values sizes.
     */
    class SPLA_API FunctionUnary final : public Object {
    public:
        ~FunctionUnary() override = default;

        /** @return Input type a. */
        const RefPtr<Type> &GetA() const;

        /** @return Result type b. */
        const RefPtr<Type> &GetB() const;

        /** @return OpenCL function body source code. */
        const std::string &GetSource() const;

        /**
         * Check if can apply this function to provided objects.
         *
         * @param a Input typed a object.
         * @param b Result typed b object.
         *
         * @return True if can apply.
         */
        bool CanApply(const TypedObject &a, const TypedObject &b) const;

        /**
         * Makes new function unary instance.
         *
         * @param a Input type a
         * @param b Result type b
         * @param source OpenCL function body source code
         * @param library Library global state
         *
         * @return New function binary instance
         */
        static RefPtr<FunctionUnary> Make(RefPtr<Type> a, RefPtr<Type> b, std::string source, Library &library);

    private:
        FunctionUnary(RefPtr<Type> a, RefPtr<Type> b, std::string source, Library &library);

        RefPtr<Type> mA;
        RefPtr<Type> mB;
        std::string mSource;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONUNARY_HPP
