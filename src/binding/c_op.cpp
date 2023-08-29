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

spla_OpBinary spla_OpBinary_PLUS_INT() { return as_ptr<spla_OpBinary_t>(spla::PLUS_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_PLUS_UINT() { return as_ptr<spla_OpBinary_t>(spla::PLUS_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_PLUS_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::PLUS_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_INT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_UINT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MULT_INT() { return as_ptr<spla_OpBinary_t>(spla::MULT_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MULT_UINT() { return as_ptr<spla_OpBinary_t>(spla::MULT_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MULT_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MULT_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_DIV_INT() { return as_ptr<spla_OpBinary_t>(spla::DIV_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_DIV_UINT() { return as_ptr<spla_OpBinary_t>(spla::DIV_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_DIV_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::DIV_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_POW2_INT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_POW2_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_POW2_UINT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_POW2_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MINUS_POW2_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MINUS_POW2_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_FIRST_INT() { return as_ptr<spla_OpBinary_t>(spla::FIRST_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_FIRST_UINT() { return as_ptr<spla_OpBinary_t>(spla::FIRST_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_FIRST_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::FIRST_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_SECOND_INT() { return as_ptr<spla_OpBinary_t>(spla::SECOND_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_SECOND_UINT() { return as_ptr<spla_OpBinary_t>(spla::SECOND_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_SECOND_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::SECOND_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_ONE_INT() { return as_ptr<spla_OpBinary_t>(spla::ONE_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_ONE_UINT() { return as_ptr<spla_OpBinary_t>(spla::ONE_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_ONE_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::ONE_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_INT() { return as_ptr<spla_OpBinary_t>(spla::MIN_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_UINT() { return as_ptr<spla_OpBinary_t>(spla::MIN_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MIN_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_INT() { return as_ptr<spla_OpBinary_t>(spla::MAX_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_UINT() { return as_ptr<spla_OpBinary_t>(spla::MAX_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MAX_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BOR_INT() { return as_ptr<spla_OpBinary_t>(spla::BOR_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BOR_UINT() { return as_ptr<spla_OpBinary_t>(spla::BOR_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BAND_INT() { return as_ptr<spla_OpBinary_t>(spla::BAND_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BAND_UINT() { return as_ptr<spla_OpBinary_t>(spla::BAND_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BXOR_INT() { return as_ptr<spla_OpBinary_t>(spla::BXOR_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BXOR_UINT() { return as_ptr<spla_OpBinary_t>(spla::BXOR_UINT.ref_and_get()); }

spla_OpSelect spla_OpSelect_EQZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::EQZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_EQZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::EQZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_EQZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::EQZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NQZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::NQZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NQZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::NQZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NQZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::NQZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GTZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::GTZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GTZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::GTZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GTZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::GTZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GEZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::GEZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GEZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::GEZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_GEZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::GEZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LTZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::LTZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LTZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::LTZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LTZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::LTZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LEZERO_INT() { return as_ptr<spla_OpSelect_t>(spla::LEZERO_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LEZERO_UINT() { return as_ptr<spla_OpSelect_t>(spla::LEZERO_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_LEZERO_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::LEZERO_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_ALWAYS_INT() { return as_ptr<spla_OpSelect_t>(spla::ALWAYS_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_ALWAYS_UINT() { return as_ptr<spla_OpSelect_t>(spla::ALWAYS_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_ALWAYS_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::ALWAYS_FLOAT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NEVER_INT() { return as_ptr<spla_OpSelect_t>(spla::NEVER_INT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NEVER_UINT() { return as_ptr<spla_OpSelect_t>(spla::NEVER_UINT.ref_and_get()); }
spla_OpSelect spla_OpSelect_NEVER_FLOAT() { return as_ptr<spla_OpSelect_t>(spla::NEVER_FLOAT.ref_and_get()); }