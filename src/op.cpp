/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
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


    ref_ptr<OpSelect> GZERO_INT;
    ref_ptr<OpSelect> GZERO_UINT;
    ref_ptr<OpSelect> GZERO_FLOAT;

#define min std::min
#define max std::max

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

        DECL_OP_BIN_S(MIN_INT, ONE, T_INT, { return min(a, b); });
        DECL_OP_BIN_S(MIN_UINT, ONE, T_UINT, { return min(a, b); });
        DECL_OP_BIN_S(MIN_FLOAT, ONE, T_FLOAT, { return min(a, b); });

        DECL_OP_BIN_S(MAX_INT, ONE, T_INT, { return max(a, b); });
        DECL_OP_BIN_S(MAX_UINT, ONE, T_UINT, { return max(a, b); });
        DECL_OP_BIN_S(MAX_FLOAT, ONE, T_FLOAT, { return max(a, b); });
    }

}// namespace spla