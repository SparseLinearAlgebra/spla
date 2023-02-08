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

__kernel void sparse_to_dense(__global TYPE*       g_rx,
                              __global const uint* g_vi,
                              __global const TYPE* g_vx,
                              __global uint*       g_fdbi,
                              __global TYPE*       g_fdbx,
                              __global uint*       g_fdb_size,
                              const uint           n) {
    const uint gid   = get_global_id(0);
    const uint gsize = get_global_size(0);

    for (uint k = gid; k < n; k += gsize) {
        const uint i    = g_vi[k];
        const TYPE prev = g_rx[i];

        g_rx[i] = OP_BINARY(prev, g_vx[k]);

        if (prev != g_rx[i]) {
            const uint offset = atomic_inc(g_fdb_size);
            g_fdbi[offset]    = i;
            g_fdbx[offset]    = g_rx[i];
        }
    }
}