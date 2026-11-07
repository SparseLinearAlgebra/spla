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

__kernel void mxm_count(__global const uint* g_Ap,
                        __global const uint* g_Aj,
                        __global const uint* g_Bp,
                        __global uint*       g_total,
                        const uint           n_rows) {
    const uint gid     = get_global_id(0);
    const uint gstride = get_global_size(0);

    for (uint i = gid; i < n_rows; i += gstride) {
        for (uint a = g_Ap[i]; a < g_Ap[i + 1]; ++a) {
            const uint k     = g_Aj[a];
            const uint count = g_Bp[k + 1] - g_Bp[k];
            atomic_add(g_total, count);
        }
    }
}

__kernel void mxm_collect(__global const uint* g_Ap,
                          __global const uint* g_Aj,
                          __global const TYPE* g_Ax,
                          __global const uint* g_Bp,
                          __global const uint* g_Bj,
                          __global const TYPE* g_Bx,
                          __global uint*       g_keys,
                          __global TYPE*       g_vals,
                          __global uint*       g_offset,
                          const uint           K,
                          const uint           n_rows) {
    const uint gid     = get_global_id(0);
    const uint gstride = get_global_size(0);

    for (uint i = gid; i < n_rows; i += gstride) {
        for (uint a = g_Ap[i]; a < g_Ap[i + 1]; ++a) {
            const uint k    = g_Aj[a];
            const TYPE av   = g_Ax[a];
            const uint bs   = g_Bp[k];
            const uint be   = g_Bp[k + 1];
            const uint cnt  = be - bs;
            uint       off  = atomic_add(g_offset, cnt);

            for (uint b = bs; b < be; ++b) {
                g_keys[off] = i * K + g_Bj[b];
                g_vals[off] = OP_BINARY1(av, g_Bx[b]);
                off += 1;
            }
        }
    }
}

__kernel void mxm_identity_flag(__global const TYPE* g_vals,
                                __global uint*       g_flags,
                                __global uint*       g_count,
                                const TYPE           init,
                                const uint           n) {
    const uint gid = get_global_id(0);
    if (gid < n) {
        const uint keep = (g_vals[gid] != init) ? 1u : 0u;
        g_flags[gid]    = keep;
        atomic_add(g_count, keep);
    }
}

__kernel void mxm_identity_scatter(__global const uint* g_keys,
                                   __global const TYPE* g_vals,
                                   __global const uint* g_offsets,
                                   __global const uint* g_flags,
                                   __global uint*       g_okeys,
                                   __global TYPE*       g_ovals,
                                   const uint           n) {
    const uint gid = get_global_id(0);
    if (gid < n && g_flags[gid] == 1u) {
        const uint dst = g_offsets[gid];
        g_okeys[dst]   = g_keys[gid];
        g_ovals[dst]   = g_vals[gid];
    }
}

__kernel void mxm_write(__global const uint* g_keys,
                        __global const TYPE* g_vals,
                        __global uint*       g_Aj,
                        __global TYPE*       g_Ax,
                        __global uint*       g_row_sizes,
                        const uint           K,
                        const uint           n) {
    const uint gid = get_global_id(0);
    if (gid < n) {
        const uint key = g_keys[gid];
        const uint row = key / K;
        const uint col = key % K;

        g_Aj[gid] = col;
        g_Ax[gid] = g_vals[gid];

        atomic_add(&g_row_sizes[row], 1);
    }
}
