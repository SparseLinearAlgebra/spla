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
#include "common_func.cl"

// generate uint offsets for unique keys to store result
__kernel void reduce_by_key_generate_offsets(__global const uint* g_keys,
                                             __global uint*       g_offsets,
                                             const uint           n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        bool is_neq    = gid + 1 < n && g_keys[gid] != g_keys[gid + 1];
        g_offsets[gid] = is_neq ? 1 : 0;
    }
}

// scalar reduction for each group of keys
__kernel void reduce_by_key_naive(__global const TYPE* g_keys,
                                  __global const TYPE* g_values,
                                  __global const uint* g_offsets,
                                  __global uint*       g_unique_keys,
                                  __global TYPE*       g_reduce_values,
                                  const uint           n_keys,
                                  const uint           n_groups) {
    const uint gid = get_global_id(0);

    if (gid < n_groups) {
        const uint start_idx = lower_bound(gid, 0, n_keys, g_offsets);
        TYPE       value     = g_values[start_idx];

        for (uint i = start_idx + 1; i < n_keys && gid == g_offsets[i]; i += 1) {
            value = OP_BINARY(value, g_values[i]);
        }

        g_unique_keys[gid]   = g_keys[start_idx];
        g_reduce_values[gid] = value;
    }
}
