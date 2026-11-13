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

uint merge_co_rank(const uint           k,
                   const uint           a_size,
                   const uint           b_size,
                   __global const uint* a_keys,
                   __global const uint* b_keys) {
    uint start = max(0, (int) k - (int) b_size);
    uint end   = min(k, a_size);

    while (start < end) {
        const uint a_ind = (start + end) / 2;
        const uint b_ind = k - a_ind - 1;

        if (a_keys[a_ind] <= b_keys[b_ind]) {
            start = a_ind + 1;
        } else {
            end = a_ind;
        }
    }

    return start;
}

__kernel void merge_union_count(
        __global const uint* a_keys,
        const uint           a_offset,
        const uint           a_size,
        __global const uint* b_keys,
        const uint           b_offset,
        const uint           b_size,
        __global uint*       tile_counts,
        const uint           tile_size,
        const uint           count_tile) {

    const uint t = get_global_id(0);
    if (t >= count_tile) return;

    const uint k0 = t * tile_size;
    const uint k1 = min((t + 1) * tile_size, a_size + b_size);

    uint i0 = merge_co_rank(k0, a_size, b_size, a_keys + a_offset, b_keys + b_offset);
    uint j0 = k0 - i0;
    if (i0 > 0 && j0 < b_size && a_keys[a_offset + i0 - 1] == b_keys[b_offset + j0]) {
        --i0;
    }

    uint i1 = merge_co_rank(k1, a_size, b_size, a_keys + a_offset, b_keys + b_offset);
    uint j1 = k1 - i1;
    if (i1 > 0 && j1 < b_size && a_keys[a_offset + i1 - 1] == b_keys[b_offset + j1]) {
        --i1;
    }

    uint i     = i0;
    uint j     = j0;
    uint count = 0;

    while (i < i1 && j < j1) {
        const uint ka = a_keys[a_offset + i];
        const uint kb = b_keys[b_offset + j];

        if (ka < kb) {
            ++i;
        } else if (kb < ka) {
            ++j;
        } else {
            ++i;
            ++j;
        }

        ++count;
    }

    while (i < i1) {
        ++i;
        ++count;
    }
    while (j < j1) {
        ++j;
        ++count;
    }

    tile_counts[t] = count;
}

__kernel void merge_union_scatter(
        __global const uint* a_keys,
        __global const TYPE* a_vals,
        const uint           a_offset,
        const uint           a_size,
        __global const uint* b_keys,
        __global const TYPE* b_vals,
        const uint           b_offset,
        const uint           b_size,
        __global const uint* tile_offsets,
        __global uint*       r_keys,
        __global TYPE*       r_vals,
        const uint           r_offset,
        const uint           tile_size,
        const uint           count_tile) {

    const uint t = get_global_id(0);
    if (t >= count_tile) return;

    const uint k0 = t * tile_size;
    const uint k1 = min((t + 1) * tile_size, a_size + b_size);

    uint i0 = merge_co_rank(k0, a_size, b_size, a_keys + a_offset, b_keys + b_offset);
    uint j0 = k0 - i0;
    if (i0 > 0 && j0 < b_size && a_keys[a_offset + i0 - 1] == b_keys[b_offset + j0]) {
        --i0;
    }

    uint i1 = merge_co_rank(k1, a_size, b_size, a_keys + a_offset, b_keys + b_offset);
    uint j1 = k1 - i1;
    if (i1 > 0 && j1 < b_size && a_keys[a_offset + i1 - 1] == b_keys[b_offset + j1]) {
        --i1;
    }

    uint i = i0;
    uint j = j0;
    uint k = tile_offsets[t];

    while (i < i1 && j < j1) {
        const uint ka = a_keys[a_offset + i];
        const uint kb = b_keys[b_offset + j];

        if (ka < kb) {
            r_keys[r_offset + k] = ka;
            r_vals[r_offset + k] = a_vals[a_offset + i];
            ++i;
        } else if (kb < ka) {
            r_keys[r_offset + k] = kb;
            r_vals[r_offset + k] = b_vals[b_offset + j];
            ++j;
        } else {
            r_keys[r_offset + k] = ka;
            r_vals[r_offset + k] = OP_BINARY(a_vals[a_offset + i], b_vals[b_offset + j]);
            ++i;
            ++j;
        }

        ++k;
    }

    while (i < i1) {
        r_keys[r_offset + k] = a_keys[a_offset + i];
        r_vals[r_offset + k] = a_vals[a_offset + i];
        ++i;
        ++k;
    }
    while (j < j1) {
        r_keys[r_offset + k] = b_keys[b_offset + j];
        r_vals[r_offset + k] = b_vals[b_offset + j];
        ++j;
        ++k;
    }
}
