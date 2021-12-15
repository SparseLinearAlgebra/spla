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

#ifndef SPLA_SPLASCALAR_HPP
#define SPLA_SPLASCALAR_HPP

#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class Scalar
     *
     * Single scalar value of specified Type.
     * Can be used in matrix and vector expressions as additional initial value or param.
     *
     * @note Can be empty, i.e. contain no value.
     * @note Can be updated from the host using ScalarDataWrite expression node.
     * @note Scalar content can be accessed from host using ScalarDataRead expression node.
     *
     * @details
     *  Actual scalar values has type `Maybe Type`, where non-zero values stored as is (`Just Value`),
     *  and null values are not stored (`Nothing`). In expressions actual operations
     *  are applied only to values `Just Value`.
     *
     * @see Expression
     * @see FunctionUnary
     * @see FunctionBinary
     */
    class SPLA_API Scalar final : public TypedObject {
    public:
        ~Scalar() override;

        /** @return True if scalar stores value */
        bool HasValue() const;

        /** @return Internal scalar storage (for private usage only) */
        [[nodiscard]] const RefPtr<class ScalarStorage> &GetStorage() const;

        /** Dump scalar content to provided stream */
        void Dump(std::ostream &stream) const;

        /** @copydoc Object::Clone() */
        RefPtr<Object> Clone() const override;

        /**
         * Make new scalar with specified params
         *
         * @param type Type of stored value
         * @param library Library global instance
         *
         * @return New scalar instance
         */
        static RefPtr<Scalar> Make(const RefPtr<Type> &type, class Library &library);

    private:
        Scalar(const RefPtr<Type> &type, class Library &library, RefPtr<class ScalarStorage> storage = nullptr);
        RefPtr<Object> CloneEmpty() override;
        void CopyData(const RefPtr<Object> &object) override;

        // Separate storage for private impl
        RefPtr<class ScalarStorage> mStorage;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLASCALAR_HPP
