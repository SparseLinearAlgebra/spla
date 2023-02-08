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

#include <core/top.hpp>

#include <algorithm>

namespace spla {

    ref_ptr<OpBinary> PLUS_INT;
    ref_ptr<OpBinary> PLUS_UINT;
    ref_ptr<OpBinary> PLUS_FLOAT;
    ref_ptr<OpBinary> MINUS_INT;
    ref_ptr<OpBinary> MINUS_UINT;
    ref_ptr<OpBinary> MINUS_FLOAT;
    ref_ptr<OpBinary> MULT_INT;
    ref_ptr<OpBinary> MULT_UINT;
    ref_ptr<OpBinary> MULT_FLOAT;
    ref_ptr<OpBinary> DIV_INT;
    ref_ptr<OpBinary> DIV_UINT;
    ref_ptr<OpBinary> DIV_FLOAT;

    ref_ptr<OpBinary> FIRST_INT;
    ref_ptr<OpBinary> FIRST_UINT;
    ref_ptr<OpBinary> FIRST_FLOAT;
    ref_ptr<OpBinary> SECOND_INT;
    ref_ptr<OpBinary> SECOND_UINT;
    ref_ptr<OpBinary> SECOND_FLOAT;

    ref_ptr<OpBinary> ONE_INT;
    ref_ptr<OpBinary> ONE_UINT;
    ref_ptr<OpBinary> ONE_FLOAT;

    ref_ptr<OpBinary> MIN_INT;
    ref_ptr<OpBinary> MIN_UINT;
    ref_ptr<OpBinary> MIN_FLOAT;
    ref_ptr<OpBinary> MAX_INT;
    ref_ptr<OpBinary> MAX_UINT;
    ref_ptr<OpBinary> MAX_FLOAT;

    ref_ptr<OpBinary> BOR_INT;
    ref_ptr<OpBinary> BOR_UINT;
    ref_ptr<OpBinary> BAND_INT;
    ref_ptr<OpBinary> BAND_UINT;
    ref_ptr<OpBinary> BXOR_INT;
    ref_ptr<OpBinary> BXOR_UINT;

    ref_ptr<OpSelect> EQZERO_INT;
    ref_ptr<OpSelect> EQZERO_UINT;
    ref_ptr<OpSelect> EQZERO_FLOAT;
    ref_ptr<OpSelect> NQZERO_INT;
    ref_ptr<OpSelect> NQZERO_UINT;
    ref_ptr<OpSelect> NQZERO_FLOAT;
    ref_ptr<OpSelect> GTZERO_INT;
    ref_ptr<OpSelect> GTZERO_UINT;
    ref_ptr<OpSelect> GTZERO_FLOAT;
    ref_ptr<OpSelect> GEZERO_INT;
    ref_ptr<OpSelect> GEZERO_UINT;
    ref_ptr<OpSelect> GEZERO_FLOAT;
    ref_ptr<OpSelect> LTZERO_INT;
    ref_ptr<OpSelect> LTZERO_UINT;
    ref_ptr<OpSelect> LTZERO_FLOAT;
    ref_ptr<OpSelect> LEZERO_INT;
    ref_ptr<OpSelect> LEZERO_UINT;
    ref_ptr<OpSelect> LEZERO_FLOAT;
    ref_ptr<OpSelect> ALWAYS_INT;
    ref_ptr<OpSelect> ALWAYS_UINT;
    ref_ptr<OpSelect> ALWAYS_FLOAT;
    ref_ptr<OpSelect> NEVER_INT;
    ref_ptr<OpSelect> NEVER_UINT;
    ref_ptr<OpSelect> NEVER_FLOAT;

    template<typename T>
    inline T min(T a, T b) { return std::min(a, b); }

    template<typename T>
    inline T max(T a, T b) { return std::max(a, b); }

    void register_ops() {
        DECL_OP_BIN_S(PLUS_INT, PLUS, T_INT, { return a + b; });
        DECL_OP_BIN_S(PLUS_UINT, PLUS, T_UINT, { return a + b; });
        DECL_OP_BIN_S(PLUS_FLOAT, PLUS, T_FLOAT, { return a + b; });
        DECL_OP_BIN_S(MINUS_INT, MINUS, T_INT, { return a - b; });
        DECL_OP_BIN_S(MINUS_UINT, MINUS, T_UINT, { return a - b; });
        DECL_OP_BIN_S(MINUS_FLOAT, MINUS, T_FLOAT, { return a - b; });
        DECL_OP_BIN_S(MULT_INT, MULT, T_INT, { return a * b; });
        DECL_OP_BIN_S(MULT_UINT, MULT, T_UINT, { return a * b; });
        DECL_OP_BIN_S(MULT_FLOAT, MULT, T_FLOAT, { return a * b; });
        DECL_OP_BIN_S(DIV_INT, DIV, T_INT, { return a / b; });
        DECL_OP_BIN_S(DIV_UINT, DIV, T_UINT, { return a / b; });
        DECL_OP_BIN_S(DIV_FLOAT, DIV, T_FLOAT, { return a / b; });

        DECL_OP_BIN_S(FIRST_INT, FIRST, T_INT, { return a; });
        DECL_OP_BIN_S(FIRST_UINT, FIRST, T_UINT, { return a; });
        DECL_OP_BIN_S(FIRST_FLOAT, FIRST, T_FLOAT, { return a; });
        DECL_OP_BIN_S(SECOND_INT, SECOND, T_INT, { return b; });
        DECL_OP_BIN_S(SECOND_UINT, SECOND, T_UINT, { return b; });
        DECL_OP_BIN_S(SECOND_FLOAT, SECOND, T_FLOAT, { return b; });

        DECL_OP_BIN_S(ONE_INT, ONE, T_INT, { return 1; });
        DECL_OP_BIN_S(ONE_UINT, ONE, T_UINT, { return 1; });
        DECL_OP_BIN_S(ONE_FLOAT, ONE, T_FLOAT, { return 1; });

        DECL_OP_BIN_S(MIN_INT, MIN, T_INT, { return min(a, b); });
        DECL_OP_BIN_S(MIN_UINT, MIN, T_UINT, { return min(a, b); });
        DECL_OP_BIN_S(MIN_FLOAT, MIN, T_FLOAT, { return min(a, b); });
        DECL_OP_BIN_S(MAX_INT, MAX, T_INT, { return max(a, b); });
        DECL_OP_BIN_S(MAX_UINT, MAX, T_UINT, { return max(a, b); });
        DECL_OP_BIN_S(MAX_FLOAT, MAX, T_FLOAT, { return max(a, b); });

        DECL_OP_BIN_S(BOR_INT, BOR, T_INT, { return a | b; });
        DECL_OP_BIN_S(BOR_UINT, BOR, T_UINT, { return a | b; });
        DECL_OP_BIN_S(BAND_INT, BAND, T_INT, { return a & b; });
        DECL_OP_BIN_S(BAND_UINT, BAND, T_UINT, { return a & b; });
        DECL_OP_BIN_S(BXOR_INT, BXOR, T_INT, { return a ^ b; });
        DECL_OP_BIN_S(BXOR_UINT, BXOR, T_UINT, { return a ^ b; });

        DECL_OP_SELECT(EQZERO_INT, EQZERO, T_INT, { return a == 0; });
        DECL_OP_SELECT(EQZERO_UINT, EQZERO, T_UINT, { return a == 0; });
        DECL_OP_SELECT(EQZERO_FLOAT, EQZERO, T_FLOAT, { return a == 0; });
        DECL_OP_SELECT(NQZERO_INT, NQZERO, T_INT, { return a != 0; });
        DECL_OP_SELECT(NQZERO_UINT, NQZERO, T_UINT, { return a != 0; });
        DECL_OP_SELECT(NQZERO_FLOAT, NQZERO, T_FLOAT, { return a != 0; });
        DECL_OP_SELECT(GTZERO_INT, GTZERO, T_INT, { return a > 0; });
        DECL_OP_SELECT(GTZERO_UINT, GTZERO, T_UINT, { return a > 0; });
        DECL_OP_SELECT(GTZERO_FLOAT, GTZERO, T_FLOAT, { return a > 0; });
        DECL_OP_SELECT(GEZERO_INT, GEZERO, T_INT, { return a >= 0; });
        DECL_OP_SELECT(GEZERO_UINT, GEZERO, T_UINT, { return a >= 0; });
        DECL_OP_SELECT(GEZERO_FLOAT, GEZERO, T_FLOAT, { return a >= 0; });
        DECL_OP_SELECT(LTZERO_INT, LTZERO, T_INT, { return a < 0; });
        DECL_OP_SELECT(LTZERO_UINT, LTZERO, T_UINT, { return a < 0; });
        DECL_OP_SELECT(LTZERO_FLOAT, LTZERO, T_FLOAT, { return a < 0; });
        DECL_OP_SELECT(LEZERO_INT, LEZERO, T_INT, { return a <= 0; });
        DECL_OP_SELECT(LEZERO_UINT, LEZERO, T_UINT, { return a <= 0; });
        DECL_OP_SELECT(LEZERO_FLOAT, LEZERO, T_FLOAT, { return a <= 0; });
        DECL_OP_SELECT(ALWAYS_INT, ALWAYS, T_INT, { return 1; });
        DECL_OP_SELECT(ALWAYS_UINT, ALWAYS, T_UINT, { return 1; });
        DECL_OP_SELECT(ALWAYS_FLOAT, ALWAYS, T_FLOAT, { return 1; });
        DECL_OP_SELECT(NEVER_INT, NEVER, T_INT, { return 0; });
        DECL_OP_SELECT(NEVER_UINT, NEVER, T_UINT, { return 0; });
        DECL_OP_SELECT(NEVER_FLOAT, NEVER, T_FLOAT, { return 0; });
    }

}// namespace spla