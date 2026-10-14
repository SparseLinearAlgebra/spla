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

uint upper_bound(const uint           x,
                 uint                 first,
                 uint                 size,
                 __global const uint* array) {
    while (size > 0) {
        int step = size / 2;

        if (array[first + step] <= x) {
            first = first + step + 1;
            size -= step + 1;
        } else {
            size = step;
        }
    }
    return first;
}

__kernel void merge_intersect_count(
        __global const uint* a_keys,
        const uint           a_size,
        __global const uint* b_keys,
        const uint           b_size,
        __global uint*       tile_counts,
        const uint           tile_size) {

    const uint t  = get_global_id(0);
    const uint i0 = t * tile_size;
    if (i0 >= a_size) return;

    const uint i1 = min(i0 + tile_size, a_size);

    const uint j0 = lower_bound(a_keys[i0], 0, b_size, b_keys);
    const uint j1 = upper_bound(a_keys[i1 - 1], 0, b_size, b_keys);

    uint i     = i0;
    uint j     = j0;
    uint count = 0;

    while (i < i1 && j < j1) {
        const uint ka = a_keys[i];
        const uint kb = b_keys[j];

        if (ka < kb) {
            ++i;
        } else if (kb < ka) {
            ++j;
        } else {
            ++count;
            ++i;
            ++j;
        }
    }

    tile_counts[t] = count;
}

__kernel void merge_intersect_scatter(
        __global const uint* a_keys,
        __global const TYPE* a_vals,
        const uint           a_size,
        __global const uint* b_keys,
        __global const TYPE* b_vals,
        const uint           b_size,
        __global const uint* tile_offsets,
        __global uint*       r_keys_ai,
        __global uint*       r_keys_ax,
        __global uint*       r_vals_ai,
        __global TYPE*       r_vals_ax,
        const uint           tile_size) {

    const uint t  = get_global_id(0);
    const uint i0 = t * tile_size;
    if (i0 >= a_size) return;

    const uint i1 = min(i0 + tile_size, a_size);

    const uint j0 = lower_bound(a_keys[i0], 0, b_size, b_keys);
    const uint j1 = upper_bound(a_keys[i1 - 1], 0, b_size, b_keys);

    uint i = i0;
    uint j = j0;
    uint k = tile_offsets[t];

    while (i < i1 && j < j1) {
        const uint ka = a_keys[i];
        const uint kb = b_keys[j];

        if (ka < kb) {
            ++i;
        } else if (kb < ka) {
            ++j;
        } else {
            r_keys_ai[k] = k;
            r_keys_ax[k] = ka;
            r_vals_ai[k] = k;
            r_vals_ax[k] = OP_BINARY(a_vals[i], b_vals[j]);
            ++k;
            ++i;
            ++j;
        }
    }
}
