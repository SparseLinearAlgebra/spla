/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#ifndef SPLA_VECTOR_HPP
#define SPLA_VECTOR_HPP

#include "object.hpp"
#include "type.hpp"

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Vector
     * @brief Generalized N dimensional vector object
     */
    class Vector : public Object {
    public:
        SPLA_API ~Vector() override                                 = default;
        SPLA_API virtual Status        hint_state(StateHint hint)   = 0;
        SPLA_API virtual Status        hint_format(FormatHint hint) = 0;
        SPLA_API virtual uint          get_n_rows()                 = 0;
        SPLA_API virtual ref_ptr<Type> get_type()                   = 0;
    };

    /**
     * @brief Make new vector instance with specified dim and values type
     *
     * @param n_rows Number of vector rows; must be > 0;
     * @param type Type of vector elements
     *
     * @return New vector instance or null if failed to create
     */
    SPLA_API ref_ptr<Vector> make_vector(uint n_rows, const ref_ptr<Type>& type);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_VECTOR_HPP
