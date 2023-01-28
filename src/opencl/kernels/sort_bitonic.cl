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

__kernel void bitonic_sort_local(__global uint* g_keys,
                                 __global TYPE* g_values,
                                 const uint     total_n) {
    const uint grid  = get_group_id(0);
    const uint lid   = get_local_id(0);
    const uint lsize = get_local_size(0);

    const uint offset    = grid * BLOCK_SIZE;
    const uint border    = min(offset + BLOCK_SIZE, total_n);
    const uint n         = border - offset;
    const uint n_aligned = ceil_to_pow2(n);
    const uint n_threads = n_aligned / 2;

    __local uint s_keys[BLOCK_SIZE];
    __local TYPE s_values[BLOCK_SIZE];

    for (uint i = lid; i + offset < border; i += lsize) {
        s_keys[i]   = g_keys[i + offset];
        s_values[i] = g_values[i + offset];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint segment_size = 2; segment_size <= n_aligned; segment_size *= 2) {
        const uint segment_size_half = segment_size / 2;

        for (uint tid = lid; tid < n_threads; tid += lsize) {
            const uint segment_id       = tid / segment_size_half;
            const uint inner_id         = tid % segment_size_half;
            const uint inner_id_sibling = segment_size - inner_id - 1;
            const uint i                = segment_id * segment_size + inner_id;
            const uint j                = segment_id * segment_size + inner_id_sibling;

            if (i < n && j < n && s_keys[i] > s_keys[j]) {
                SWAP_KEYS(s_keys[i], s_keys[j]);
                SWAP_VALUES(s_values[i], s_values[j]);
            }
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        for (uint k = segment_size_half / 2; k > 0; k /= 2) {
            for (uint tid = lid; tid < n_threads; tid += lsize) {
                const uint segment_size_inner = k * 2;
                const uint segment_id         = tid / k;
                const uint inner_id           = tid % k;
                const uint inner_id_sibling   = inner_id + k;
                const uint i                  = segment_id * segment_size_inner + inner_id;
                const uint j                  = segment_id * segment_size_inner + inner_id_sibling;

                if (i < n && j < n && s_keys[i] > s_keys[j]) {
                    SWAP_KEYS(s_keys[i], s_keys[j]);
                    SWAP_VALUES(s_values[i], s_values[j]);
                }
            }

            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }

    for (uint i = lid; i + offset < border; i += lsize) {
        g_keys[i + offset]   = s_keys[i];
        g_values[i + offset] = s_values[i];
    }
}

__kernel void bitonic_sort_global(__global uint* g_keys,
                                  __global TYPE* g_values,
                                  const uint     n,
                                  const uint     segment_start) {
    const uint lid       = get_local_id(0);
    const uint lsize     = get_local_size(0);
    const uint n_aligned = ceil_to_pow2(n);
    const uint n_threads = n_aligned / 2;

    for (uint segment_size = segment_start; segment_size <= n_aligned; segment_size *= 2) {
        const uint segment_size_half = segment_size / 2;

        for (uint tid = lid; tid < n_threads; tid += lsize) {
            const uint segment_id       = tid / segment_size_half;
            const uint inner_id         = tid % segment_size_half;
            const uint inner_id_sibling = segment_size - inner_id - 1;
            const uint i                = segment_id * segment_size + inner_id;
            const uint j                = segment_id * segment_size + inner_id_sibling;

            if (i < n && j < n && g_keys[i] > g_keys[j]) {
                SWAP_KEYS(g_keys[i], g_keys[j]);
                SWAP_VALUES(g_values[i], g_values[j]);
            }
        }

        barrier(CLK_GLOBAL_MEM_FENCE);

        for (uint k = segment_size_half / 2; k > 0; k /= 2) {
            for (uint tid = lid; tid < n_threads; tid += lsize) {
                const uint segment_size_inner = k * 2;
                const uint segment_id         = tid / k;
                const uint inner_id           = tid % k;
                const uint inner_id_sibling   = inner_id + k;
                const uint i                  = segment_id * segment_size_inner + inner_id;
                const uint j                  = segment_id * segment_size_inner + inner_id_sibling;

                if (i < n && j < n && g_keys[i] > g_keys[j]) {
                    SWAP_KEYS(g_keys[i], g_keys[j]);
                    SWAP_VALUES(g_values[i], g_values[j]);
                }
            }

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}