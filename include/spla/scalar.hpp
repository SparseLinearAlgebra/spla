/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_SCALAR_HPP
#define SPLA_SCALAR_HPP

#include "object.hpp"
#include "type.hpp"

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Scalar
     * @brief Box for a single typed scalar value
     */
    class Scalar : public Object {
    public:
        SPLA_API ~Scalar() override                                   = default;
        SPLA_API virtual ref_ptr<Type> get_type()                     = 0;
        SPLA_API virtual Status        set_int(std::int32_t value)    = 0;
        SPLA_API virtual Status        set_uint(std::uint32_t value)  = 0;
        SPLA_API virtual Status        set_float(float value)         = 0;
        SPLA_API virtual Status        get_int(std::int32_t& value)   = 0;
        SPLA_API virtual Status        get_uint(std::uint32_t& value) = 0;
        SPLA_API virtual Status        get_float(float& value)        = 0;
        SPLA_API virtual T_INT         as_int()                       = 0;
        SPLA_API virtual T_UINT        as_uint()                      = 0;
        SPLA_API virtual T_FLOAT       as_float()                     = 0;
    };

    SPLA_API ref_ptr<Scalar> make_scalar(const ref_ptr<Type>& type);
    SPLA_API ref_ptr<Scalar> make_byte(std::int8_t value);
    SPLA_API ref_ptr<Scalar> make_int(std::int32_t value);
    SPLA_API ref_ptr<Scalar> make_uint(std::uint32_t value);
    SPLA_API ref_ptr<Scalar> make_float(float value);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SCALAR_HPP
