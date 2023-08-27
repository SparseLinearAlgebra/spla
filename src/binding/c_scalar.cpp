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

spla_Status spla_Scalar_make(spla_Scalar* s, spla_Type type) {
    auto scalar = spla::Scalar::make(as_ref<spla::Type>(type));
    *s          = as_ptr<spla_Scalar_t>(scalar.release());
    return SPLA_STATUS_OK;
}
spla_Status spla_Scalar_set_int(spla_Scalar s, int value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->set_int(value));
}
spla_Status spla_Scalar_set_uint(spla_Scalar s, unsigned int value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->set_uint(value));
}
spla_Status spla_Scalar_set_float(spla_Scalar s, float value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->set_float(value));
}
spla_Status spla_Scalar_get_int(spla_Scalar s, int* value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->get_int(*value));
}
spla_Status spla_Scalar_get_uint(spla_Scalar s, unsigned int* value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->get_uint(*value));
}
spla_Status spla_Scalar_get_float(spla_Scalar s, float* value) {
    return to_c_status(as_ptr<spla::Scalar>(s)->get_float(*value));
}