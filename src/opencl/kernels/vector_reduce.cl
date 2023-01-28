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

// wave-wide reduction in local memory
void reduction_group(uint                   block_size,
                     uint                   lid,
                     volatile __local TYPE* s_sum) {
    if (BLOCK_SIZE >= block_size) {
        if (lid < (block_size / 2)) {
            s_sum[lid] = OP_BINARY(s_sum[lid], s_sum[lid + (block_size / 2)]);
        }
        if (block_size > WARP_SIZE) {
            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }
}

__kernel void reduce(__global const TYPE* g_vec,
                     __global const TYPE* g_init,
                     __global TYPE*       g_sum,
                     const uint           stride,
                     const uint           n) {
    const uint gid   = get_group_id(0);
    const uint lsize = get_local_size(0);
    const uint lid   = get_local_id(0);

    __local TYPE s_sum[BLOCK_SIZE];
    TYPE         sum = g_init[0];

    const uint gstart = gid * stride;
    const uint gend   = gstart + stride;

    for (uint i = gstart + lid; i < gend && i < n; i += lsize) {
        sum = OP_BINARY(sum, g_vec[i]);
    }

    s_sum[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    reduction_group(1024, lid, s_sum);
    reduction_group(512, lid, s_sum);
    reduction_group(256, lid, s_sum);
    reduction_group(128, lid, s_sum);
    reduction_group(64, lid, s_sum);
    reduction_group(32, lid, s_sum);
    reduction_group(16, lid, s_sum);
    reduction_group(8, lid, s_sum);
    reduction_group(4, lid, s_sum);
    reduction_group(2, lid, s_sum);

    if (lid == 0) {
        g_sum[gid] = s_sum[0];
    }
}