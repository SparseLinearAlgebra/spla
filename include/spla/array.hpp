/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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

#ifndef SPLA_ARRAY_HPP
#define SPLA_ARRAY_HPP

#include "config.hpp"
#include "object.hpp"
#include "type.hpp"

namespace spla {

    /**
     * @class Array
     * @brief One-dimensional dense tightly packed array of typed values
     *
     * Represents typed array of elements, which can be queried and modified.
     * An array may be used to construct some collection from user data,
     * store it, pass around, and use for Vector or Matrix construction.
     *
     * Also an array may be used to inspect internal storage of a particular
     * Vector or Matrix, allowing to extract arrays of COO, CSR, etc. formats.,
     * including their Acc (OpenCL) representation.
     */
    class Array : public Object {
    public:
        SPLA_API ~Array() override                                       = default;
        SPLA_API virtual uint          get_n_values()                    = 0;
        SPLA_API virtual ref_ptr<Type> get_type()                        = 0;
        SPLA_API virtual Status        set_int(uint i, T_INT value)      = 0;
        SPLA_API virtual Status        set_uint(uint i, T_UINT value)    = 0;
        SPLA_API virtual Status        set_float(uint i, T_FLOAT value)  = 0;
        SPLA_API virtual Status        get_int(uint i, T_INT& value)     = 0;
        SPLA_API virtual Status        get_uint(uint i, T_UINT& value)   = 0;
        SPLA_API virtual Status        get_float(uint i, T_FLOAT& value) = 0;
        SPLA_API virtual Status        resize(uint n_values)             = 0;
        SPLA_API virtual Status        clear()                           = 0;

        SPLA_API static ref_ptr<Array> make(uint n_values, const ref_ptr<Type>& type);
    };


}// namespace spla

#endif//SPLA_ARRAY_HPP
