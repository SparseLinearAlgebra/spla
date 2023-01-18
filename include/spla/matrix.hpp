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

#ifndef SPLA_MATRIX_HPP
#define SPLA_MATRIX_HPP

#include "object.hpp"
#include "op.hpp"
#include "type.hpp"

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Matrix
     * @brief Generalized M x N dimensional matrix object
     */
    class Matrix : public Object {
    public:
        SPLA_API ~Matrix() override                                                             = default;
        SPLA_API virtual Status        hint_state(StateHint hint)                               = 0;
        SPLA_API virtual uint          get_n_rows()                                             = 0;
        SPLA_API virtual uint          get_n_cols()                                             = 0;
        SPLA_API virtual ref_ptr<Type> get_type()                                               = 0;
        SPLA_API virtual Status        set_reduce(ref_ptr<OpBinary> resolve_duplicates)         = 0;
        SPLA_API virtual Status        set_byte(uint row_id, uint col_id, std::int8_t value)    = 0;
        SPLA_API virtual Status        set_int(uint row_id, uint col_id, std::int32_t value)    = 0;
        SPLA_API virtual Status        set_uint(uint row_id, uint col_id, std::uint32_t value)  = 0;
        SPLA_API virtual Status        set_float(uint row_id, uint col_id, float value)         = 0;
        SPLA_API virtual Status        get_byte(uint row_id, uint col_id, std::int8_t& value)   = 0;
        SPLA_API virtual Status        get_int(uint row_id, uint col_id, std::int32_t& value)   = 0;
        SPLA_API virtual Status        get_uint(uint row_id, uint col_id, std::uint32_t& value) = 0;
        SPLA_API virtual Status        get_float(uint row_id, uint col_id, float& value)        = 0;
        SPLA_API virtual Status        clear()                                                  = 0;
    };

    /**
     * @brief Make new matrix instance with specified dim and values type
     *
     * @param n_rows Number of matrix rows; must be > 0;
     * @param n_cols Number of matrix columns; must be > 0;
     * @param type Type of matrix elements
     *
     * @return New matrix instance or null if failed to create
     */
    SPLA_API ref_ptr<Matrix> make_matrix(uint n_rows, uint n_cols, const ref_ptr<Type>& type);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_MATRIX_HPP
