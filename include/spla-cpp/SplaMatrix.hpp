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

#ifndef SPLA_SPLAMATRIX_HPP
#define SPLA_SPLAMATRIX_HPP

#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>

namespace spla {

    /**
     * @class Matrix
     */
    class SPLA_API Matrix final: public Object, public TypedObject {
    public:
        ~Matrix() override;

        /** @return Number of matrix rows */
        size_t GetNrows() const;

        /** @return Number of matrix columns */
        size_t GetNcols() const;

        /** @return Number of matrix values */
        size_t GetNvals() const;

        /**
         * Make new matrix with specified size
         *
         * @param nrows Number of matrix rows
         * @param ncols Number of matrix columns
         * @param type Type of stored values
         * @param library Library global instance
         *
         * @return New matrix instance
         */
        static RefPtr<Matrix> Make(size_t nrows, size_t ncols, const RefPtr<Type> &type, class Library& library);

    private:
        Matrix(size_t nrows, size_t ncols, const RefPtr<Type> &type, class Library& library);

        // Separate storage for private impl
        //RefPtr<class MatrixStorage> mStorage;
    };

}

#endif //SPLA_SPLAMATRIX_HPP