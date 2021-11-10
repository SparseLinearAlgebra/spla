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

#ifndef SPLA_SPLAFUNCTIONBINARY_HPP
#define SPLA_SPLAFUNCTIONBINARY_HPP

#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class FunctionBinary
     * @brief Binary Typed element function `f: a x b -> c`.
     *
     * Binary function accepts two values of typed input objects, with types
     * `A` and `B` and returns new value for typed object with type `C`.
     * Can be used as binary op in element wise operations and as accum
     * for matrices, vectors and scalars.
     *
     * Source code for the function must be provided in the string.
     * Function signature is following `void(_ACCESS_A const void* vp_a, _ACCESS_B const void* vp_b, _ACCESS_C void* vp_c)`,
     * where a, b and c pointers to values, c must be written by the function.
     * It is up to user to cast these pointers to appropriate types and check values sizes.
     */
    class SPLA_API FunctionBinary final : public Object {
    public:
        ~FunctionBinary() override = default;

        /** @return Input type a. */
        const RefPtr<Type> &GetA() const;

        /** @return Input type b. */
        const RefPtr<Type> &GetB() const;

        /** @return Result type c. */
        const RefPtr<Type> &GetC() const;

        /** @return OpenCL function body source code. */
        const std::string &GetSource() const;

        /**
         * Check if can apply this function to provided objects.
         *
         * @param a Input typed a object.
         * @param b Input typed b object.
         * @param c Result typed c object.
         *
         * @return True if can apply.
         */
        bool CanApply(const TypedObject &a, const TypedObject &b, const TypedObject &c) const;

        /**
         * Makes new function binary instance.
         *
         * @param a Input type a
         * @param b Input type b
         * @param c Result type c
         * @param source OpenCL function body source code
         * @param library Library global state
         *
         * @return New function binary instance
         */
        static RefPtr<FunctionBinary> Make(RefPtr<Type> a, RefPtr<Type> b, RefPtr<Type> c, std::string source, Library &library);

    private:
        FunctionBinary(RefPtr<Type> a, RefPtr<Type> b, RefPtr<Type> c, std::string source, Library &library);

        RefPtr<Type> mA;
        RefPtr<Type> mB;
        RefPtr<Type> mC;
        std::string mSource;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONBINARY_HPP
