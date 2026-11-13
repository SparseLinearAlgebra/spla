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

#include "common_def.cl"

struct PathIndex {
    uint middle;
    uint height;
};

struct PathIndex path_index_add(struct PathIndex a, struct PathIndex b) {
    bool a_bottom = (a.middle == 0 && a.height == 0);
    bool b_bottom = (b.middle == 0 && b.height == 0);
    if (a_bottom) return b;
    if (b_bottom) return a;
    if (a.height != b.height) return a.height < b.height ? a : b;
    return a.middle <= b.middle ? a : b;
}

struct PathIndex path_index_multiply(struct PathIndex a, uint k, struct PathIndex b) {
    struct PathIndex r;
    bool a_bottom = (a.middle == 0 && a.height == 0);
    bool b_bottom = (b.middle == 0 && b.height == 0);
    if (a_bottom || b_bottom) {
        r.middle = 0;
        r.height = 0;
    } else {
        r.middle = k;
        r.height = max(a.height, b.height) + 1;
    }
    return r;
}

#ifndef USE_PATH_INDEX_SEMANTICS
#define OP_BINARY1_INDEX(a, k, b) OP_BINARY1(a, b)
#define OP_IS_INIT(v, init)       ((v) == (init))
#else
#define OP_BINARY1_INDEX(a, k, b) path_index_multiply(a, k, b)
#define OP_BINARY2(a, b)          path_index_add(a, b)
#define OP_IS_INIT(v, init)       ((v).middle == 0 && (v).height == 0)
#endif

uint random_gen_java(ulong seed) {
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48L) - 1);
    return seed >> 16;
}

int random_int(ulong seed) {
    return random_gen_java(seed);
}

uint random_uint(ulong seed) {
    return random_gen_java(seed);
}

float random_float(ulong seed) {
    float ptr;
    return fract(sin(((float) random_uint(seed)) * 112.9898f) * 43758.5453f, &ptr);
}

uint fasu(float v) {
    return *((uint*) &v);
}