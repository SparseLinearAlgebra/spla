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

__kernel void assign_dense_to_dense(__global TYPE*       g_r,
                                    __global const TYPE* g_mask,
                                    const TYPE           init,
                                    const uint           n) {
    uint gid     = get_global_id(0);
    uint gstride = get_global_size(0);

    for (uint i = gid; i < n; i += gstride) {
        if (OP_SELECT(g_mask[i])) {
            g_r[i] = OP_BINARY(g_r[i], init);
        }
    }
}

__kernel void assign_sparse_to_dense(__global TYPE*       g_r,
                                     __global const uint* g_maski,
                                     __global const TYPE* g_maskx,
                                     const TYPE           init,
                                     const uint           n) {
    uint gid     = get_global_id(0);
    uint gstride = get_global_size(0);

    for (uint idx = gid; idx < n; idx += gstride) {
        const TYPE mx = g_maskx[idx];
        if (OP_SELECT(mx)) {
            const uint mi = g_maski[idx];
            g_r[mi]       = OP_BINARY(g_r[mi], init);
        }
    }
}