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

__kernel void vxm_sparse_count(__global const uint* g_vi,
                               __global const TYPE* g_vx,
                               __global const uint* g_Ap,
                               __global const uint* g_Aj,
                               __global const TYPE* g_mask,
                               __global uint*       g_size,
                               const uint           n) {
    const uint gid     = get_global_id(0);  // id of v entry to touch
    const uint gstride = get_global_size(0);// step between v entries

    for (int idx = gid; idx < n; idx += gstride) {
        const uint vi = g_vi[idx];
        const TYPE vx = g_vx[idx];

        const uint start = g_Ap[vi];
        const uint end   = g_Ap[vi + 1];

        uint count = 0;

        for (uint i = start; i < end; i++) {
            if (OP_SELECT(g_mask[g_Aj[i]])) count += 1;
        }

        atomic_add(g_size, count);
    }
}

__kernel void vxm_sparse_collect(__global const uint* g_vi,
                                 __global const TYPE* g_vx,
                                 __global const uint* g_Ap,
                                 __global const uint* g_Aj,
                                 __global const TYPE* g_Ax,
                                 __global const TYPE* g_mask,
                                 __global uint*       g_ri,
                                 __global TYPE*       g_rx,
                                 __global uint*       g_roffset,
                                 const uint           n) {
    const uint gid     = get_global_id(0);  // id of v entry to touch
    const uint gstride = get_global_size(0);// step between v entries

    for (int idx = gid; idx < n; idx += gstride) {
        const uint vi = g_vi[idx];
        const TYPE vx = g_vx[idx];

        const uint start = g_Ap[vi];
        const uint end   = g_Ap[vi + 1];

        uint count = 0;

        for (uint i = start; i < end; i++) {
            if (OP_SELECT(g_mask[g_Aj[i]])) count += 1;
        }

        uint offset = atomic_add(g_roffset, count);

        for (uint i = start; i < end; i++) {
            const uint col_id = g_Aj[i];

            if (OP_SELECT(g_mask[col_id])) {
                g_ri[offset] = col_id;
                g_rx[offset] = OP_BINARY1(vx, g_Ax[i]);
                offset += 1;
            }
        }
    }
}