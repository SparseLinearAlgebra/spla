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

#ifndef SPLA_SPLAVECTOR_HPP
#define SPLA_SPLAVECTOR_HPP

#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>

namespace spla {

    /**
     * @class Vector
     */
    class SPLA_API Vector final : public Object, public TypedObject {
    public:
        ~Vector() override;

        /** @return Number of vector rows */
        size_t GetNrows() const;

        /** @return Number of vector values */
        size_t GetNvals() const;

        /**
         * Make new vector with specified size
         *
         * @param nrows Number of vector rows
         * @param type Type of stored values
         * @param library Library global instance
         *
         * @return New vector instance
         */
        static RefPtr<Vector> Make(size_t nrows, const RefPtr<Type> &type, class Library &library);

    private:
        Vector(size_t nrows, const RefPtr<Type> &type, class Library &library);

        // Separate storage for private impl
        //RefPtr<class VectorStorage> mStorage;
    };

}// namespace spla

#endif//SPLA_SPLAVECTOR_HPP
