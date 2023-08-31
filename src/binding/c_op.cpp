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

spla_OpUnary spla_OpUnary_IDENTITY_INT() { return as_ptr<spla_OpUnary_t>(spla::IDENTITY_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_IDENTITY_UINT() { return as_ptr<spla_OpUnary_t>(spla::IDENTITY_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_IDENTITY_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::IDENTITY_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_AINV_INT() { return as_ptr<spla_OpUnary_t>(spla::AINV_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_AINV_UINT() { return as_ptr<spla_OpUnary_t>(spla::AINV_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_AINV_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::AINV_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_MINV_INT() { return as_ptr<spla_OpUnary_t>(spla::MINV_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_MINV_UINT() { return as_ptr<spla_OpUnary_t>(spla::MINV_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_MINV_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::MINV_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_LNOT_INT() { return as_ptr<spla_OpUnary_t>(spla::LNOT_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_LNOT_UINT() { return as_ptr<spla_OpUnary_t>(spla::LNOT_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_LNOT_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::LNOT_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_UONE_INT() { return as_ptr<spla_OpUnary_t>(spla::UONE_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_UONE_UINT() { return as_ptr<spla_OpUnary_t>(spla::UONE_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_UONE_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::UONE_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ABS_INT() { return as_ptr<spla_OpUnary_t>(spla::ABS_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ABS_UINT() { return as_ptr<spla_OpUnary_t>(spla::ABS_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ABS_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::ABS_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_BNOT_INT() { return as_ptr<spla_OpUnary_t>(spla::BNOT_INT.ref_and_get()); }
spla_OpUnary spla_OpUnary_BNOT_UINT() { return as_ptr<spla_OpUnary_t>(spla::BNOT_UINT.ref_and_get()); }
spla_OpUnary spla_OpUnary_SQRT_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::SQRT_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_LOG_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::LOG_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_EXP_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::EXP_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_SIN_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::SIN_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_COS_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::COS_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_TAN_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::TAN_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ASIN_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::ASIN_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ACOS_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::ACOS_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ATAN_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::ATAN_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_CEIL_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::CEIL_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_FLOOR_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::FLOOR_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_ROUND_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::ROUND_FLOAT.ref_and_get()); }
spla_OpUnary spla_OpUnary_TRUNC_FLOAT() { return as_ptr<spla_OpUnary_t>(spla::TRUNC_FLOAT.ref_and_get()); }

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
spla_OpBinary spla_OpBinary_BONE_INT() { return as_ptr<spla_OpBinary_t>(spla::BONE_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BONE_UINT() { return as_ptr<spla_OpBinary_t>(spla::BONE_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_BONE_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::BONE_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_INT() { return as_ptr<spla_OpBinary_t>(spla::MIN_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_UINT() { return as_ptr<spla_OpBinary_t>(spla::MIN_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MIN_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MIN_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_INT() { return as_ptr<spla_OpBinary_t>(spla::MAX_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_UINT() { return as_ptr<spla_OpBinary_t>(spla::MAX_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_MAX_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::MAX_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LOR_INT() { return as_ptr<spla_OpBinary_t>(spla::LOR_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LOR_UINT() { return as_ptr<spla_OpBinary_t>(spla::LOR_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LOR_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::LOR_FLOAT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LAND_INT() { return as_ptr<spla_OpBinary_t>(spla::LAND_INT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LAND_UINT() { return as_ptr<spla_OpBinary_t>(spla::LAND_UINT.ref_and_get()); }
spla_OpBinary spla_OpBinary_LAND_FLOAT() { return as_ptr<spla_OpBinary_t>(spla::LAND_FLOAT.ref_and_get()); }
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