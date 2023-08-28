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

spla_Status spla_Array_make(spla_Array* v, spla_uint n_values, spla_Type type) {
    auto array = spla::Array::make(n_values, as_ref<spla::Type>(type));
    *v         = as_ptr<spla_Array_t>(array.release());
    return SPLA_STATUS_OK;
}
spla_Status spla_Array_get_n_values(spla_Array a, spla_uint* values) {
    *values = as_ptr<spla::Array>(a)->get_n_values();
    return SPLA_STATUS_OK;
}
spla_Status spla_Array_set_int(spla_Array a, spla_uint i, int value) {
    return to_c_status(as_ptr<spla::Array>(a)->set_int(i, value));
}
spla_Status spla_Array_set_uint(spla_Array a, spla_uint i, unsigned int value) {
    return to_c_status(as_ptr<spla::Array>(a)->set_uint(i, value));
}
spla_Status spla_Array_set_float(spla_Array a, spla_uint i, float value) {
    return to_c_status(as_ptr<spla::Array>(a)->set_float(i, value));
}
spla_Status spla_Array_get_int(spla_Array a, spla_uint i, int* value) {
    return to_c_status(as_ptr<spla::Array>(a)->get_int(i, *value));
}
spla_Status spla_Array_get_uint(spla_Array a, spla_uint i, unsigned int* value) {
    return to_c_status(as_ptr<spla::Array>(a)->get_uint(i, *value));
}
spla_Status spla_Array_get_float(spla_Array a, spla_uint i, float* value) {
    return to_c_status(as_ptr<spla::Array>(a)->get_float(i, *value));
}
spla_Status spla_Array_resize(spla_Array a, spla_uint n) {
    return to_c_status(as_ptr<spla::Array>(a)->resize(n));
}
spla_Status spla_Array_clear(spla_Array a) {
    return to_c_status(as_ptr<spla::Array>(a)->clear());
}