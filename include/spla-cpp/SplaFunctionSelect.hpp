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

#ifndef SPLA_SPLAFUNCTIONSELECT_HPP
#define SPLA_SPLAFUNCTIONSELECT_HPP

#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class FunctionSelect
     * @brief Typed element function `f: a -> bool`.
     *
     * Select function accepts value of typed input object, with type
     * `A` and returns boolean value (true or false) based on input value.
     * Can be used as select op in select operations for matrices, vectors and scalars.
     *
     * Source code for the function must be provided in the string.
     * Function signature is following `bool(_ACCESS_A const void* vp_a)`,
     * where a pointer to value, result must be returned by the function as boolean value.
     * It is up to user to cast this pointer to appropriate types and check value size.
     */
    class SPLA_API FunctionSelect final : public Object {
    public:
        ~FunctionSelect() override = default;

        /** @return Input type a. */
        const RefPtr<Type> &GetA() const;

        /** @return OpenCL function body source code. */
        const std::string &GetSource() const;

        /**
         * Check if can apply this function to provided object.
         *
         * @param a Input typed a object.
         *
         * @return True if can apply.
         */
        bool CanApply(const TypedObject &a) const;

        /**
         * Makes new function select instance.
         *
         * @param a Input type a
         * @param source OpenCL function body source code
         * @param library Library global state
         *
         * @return New function binary instance
         */
        static RefPtr<FunctionSelect> Make(RefPtr<Type> a, std::string source, Library &library);

    private:
        FunctionSelect(RefPtr<Type> a, std::string source, Library &library);

        RefPtr<Type> mA;
        std::string mSource;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONSELECT_HPP
