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

spla_Status spla_Vector_make(spla_Vector* v, spla_uint n_rows, spla_Type type) {
    auto vector = spla::Vector::make(n_rows, as_ref<spla::Type>(type));
    *v          = as_ptr<spla_Vector_t>(vector.release());
    return SPLA_STATUS_OK;
}
spla_Status spla_Vector_set_format(spla_Vector v, int format) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_format(static_cast<spla::FormatVector>(format)));
}
spla_Status spla_Vector_set_fill_value(spla_Vector v, spla_Scalar value) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_fill_value(as_ref<spla::Scalar>(value)));
}
spla_Status spla_Vector_set_reduce(spla_Vector v, spla_OpBinary reduce) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_reduce(as_ref<spla::OpBinary>(reduce)));
}
spla_Status spla_Vector_set_int(spla_Vector v, spla_uint row_id, int value) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_int(row_id, value));
}
spla_Status spla_Vector_set_uint(spla_Vector v, spla_uint row_id, unsigned int value) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_uint(row_id, value));
}
spla_Status spla_Vector_set_float(spla_Vector v, spla_uint row_id, float value) {
    return to_c_status(as_ptr<spla::Vector>(v)->set_float(row_id, value));
}
spla_Status spla_Vector_get_int(spla_Vector v, spla_uint row_id, int* value) {
    return to_c_status(as_ptr<spla::Vector>(v)->get_int(row_id, *value));
}
spla_Status spla_Vector_get_uint(spla_Vector v, spla_uint row_id, unsigned int* value) {
    return to_c_status(as_ptr<spla::Vector>(v)->get_uint(row_id, *value));
}
spla_Status spla_Vector_get_float(spla_Vector v, spla_uint row_id, float* value) {
    return to_c_status(as_ptr<spla::Vector>(v)->get_float(row_id, *value));
}
spla_Status spla_Vector_build(spla_Vector v, spla_MemView keys, spla_MemView values) {
    return to_c_status(as_ptr<spla::Vector>(v)->build(as_ref<spla::MemView>(keys), as_ref<spla::MemView>(values)));
}
spla_Status spla_Vector_read(spla_Vector v, spla_MemView* keys, spla_MemView* values) {
    spla::ref_ptr<spla::MemView> out_keys;
    spla::ref_ptr<spla::MemView> out_values;
    const auto                   status = as_ptr<spla::Vector>(v)->read(out_keys, out_values);
    if (status == spla::Status::Ok) {
        *keys   = as_ptr<spla_MemView_t>(out_keys.release());
        *values = as_ptr<spla_MemView_t>(out_values.release());
        return SPLA_STATUS_OK;
    }
    return to_c_status(status);
}
spla_Status spla_Vector_clear(spla_Vector v) {
    return to_c_status(as_ptr<spla::Vector>(v)->clear());
}