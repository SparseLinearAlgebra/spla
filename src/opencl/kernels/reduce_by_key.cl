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
__kernel void reduce_by_key_scalar(__global const uint* g_keys,
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

__kernel void reduce_by_key_small(__global const uint* g_keys,
                                  __global const TYPE* g_values,
                                  __global uint*       g_unique_keys,
                                  __global TYPE*       g_reduce_values,
                                  __global uint*       g_reduced_count,
                                  const uint           n_keys) {
    const uint lid = get_local_id(0);

    __local uint s_offsets[BLOCK_SIZE];

    uint gen_key = 0;
    if (lid < n_keys) {
        bool is_neq   = lid > 0 && g_keys[lid] != g_keys[lid - 1];
        bool is_first = lid == 0;
        gen_key       = is_neq || is_first ? 1 : 0;
    }
    s_offsets[lid] = gen_key;

    for (uint offset = 1; offset < BLOCK_SIZE; offset *= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);
        uint value = s_offsets[lid];

        if (offset <= lid) {
            value += s_offsets[lid - offset];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
        s_offsets[lid] = value;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    const uint n_values = s_offsets[BLOCK_SIZE - 1];

    if (lid < n_values) {
        const uint id        = lid + 1;
        const uint start_idx = lower_bound_local(id, 0, n_keys, s_offsets);
        TYPE       value     = g_values[start_idx];

        for (uint i = start_idx + 1; i < n_keys && id == s_offsets[i]; i += 1) {
            value = OP_BINARY(value, g_values[i]);
        }

        g_unique_keys[lid]   = g_keys[start_idx];
        g_reduce_values[lid] = value;
    }

    if (lid == 0) {
        g_reduced_count[0] = n_values;
    }
}

__kernel void reduce_by_key_sequential(__global const uint* g_keys,
                                       __global const TYPE* g_values,
                                       __global uint*       g_unique_keys,
                                       __global TYPE*       g_reduce_values,
                                       __global uint*       g_reduced_count,
                                       const uint           n_keys) {
    uint count         = 0;
    uint current_key   = g_keys[0];
    TYPE current_value = g_values[0];

    for (uint read_offset = 1; read_offset < n_keys; read_offset += 1) {
        if (g_keys[read_offset] == current_key) {
            current_value = OP_BINARY(current_value, g_values[read_offset]);
        } else {
            g_unique_keys[count]   = current_key;
            g_reduce_values[count] = current_value;
            current_key            = g_keys[read_offset];
            current_value          = g_values[read_offset];
            count += 1;
        }
    }

    g_unique_keys[count]   = current_key;
    g_reduce_values[count] = current_value;
    g_reduced_count[0]     = count + 1;
}