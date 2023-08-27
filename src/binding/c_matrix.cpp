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

#include "c_config.hpp"

spla_Status spla_Matrix_make(spla_Matrix* M, spla_uint n_rows, spla_uint n_cols, spla_Type type) {
    auto matrix = spla::Matrix::make(n_rows, n_cols, as_ref<spla::Type>(type));
    *M          = as_ptr<spla_Matrix_t>(matrix.release());
    return SPLA_STATUS_OK;
}
spla_Status spla_Matrix_set_fill_value(spla_Matrix M, spla_Scalar value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->set_fill_value(as_ref<spla::Scalar>(value)));
}
spla_Status spla_Matrix_set_reduce(spla_Matrix M, spla_OpBinary reduce) {
    return to_c_status(as_ptr<spla::Matrix>(M)->set_reduce(as_ref<spla::OpBinary>(reduce)));
}
spla_Status spla_Matrix_set_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->set_int(row_id, col_id, value));
}
spla_Status spla_Matrix_set_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->set_uint(row_id, col_id, value));
}
spla_Status spla_Matrix_set_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->set_float(row_id, col_id, value));
}
spla_Status spla_Matrix_get_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int* value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->get_int(row_id, col_id, *value));
}
spla_Status spla_Matrix_get_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int* value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->get_uint(row_id, col_id, *value));
}
spla_Status spla_Matrix_get_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float* value) {
    return to_c_status(as_ptr<spla::Matrix>(M)->get_float(row_id, col_id, *value));
}
spla_Status spla_Matrix_build(spla_Matrix M, spla_Array keys1, spla_Array keys2, spla_Array values) {
    return to_c_status(as_ptr<spla::Matrix>(M)->build(as_ref<spla::Array>(keys1), as_ref<spla::Array>(keys2), as_ref<spla::Array>(values)));
}
spla_Status spla_Matrix_read(spla_Matrix M, spla_Array keys1, spla_Array keys2, spla_Array values) {
    return to_c_status(as_ptr<spla::Matrix>(M)->read(as_ref<spla::Array>(keys1), as_ref<spla::Array>(keys2), as_ref<spla::Array>(values)));
}
spla_Status spla_Matrix_clear(spla_Matrix M) {
    return to_c_status(as_ptr<spla::Matrix>(M)->clear());
}