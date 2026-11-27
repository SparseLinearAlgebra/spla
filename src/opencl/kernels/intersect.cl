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

__kernel void intersect_naive(
        __global const uint* x_keys,
        __global const TYPE* x_vals,
        const uint           x_size,
        __global const uint* y_keys,
        __global const TYPE* y_vals,
        const uint           y_size,
        __global uint*       r_keys_ai,
        __global uint*       r_keys_ax,
        __global uint*       r_vals_ai,
        __global TYPE*       r_vals_ax,
        __global uint*       r_size,
        const uint           swapped) {

    const uint gid   = get_global_id(0);
    const uint gsize = get_global_size(0);

    for (uint i = gid; i < x_size; i += gsize) {
        const uint key   = x_keys[i];
        const TYPE val_x = x_vals[i];

        const uint pos = lower_bound(key, 0, y_size, y_keys);

        if (pos < y_size && y_keys[pos] == key) {
            const uint idx = atomic_add(r_size, 1);
            r_keys_ai[idx] = idx;
            r_keys_ax[idx] = key;
            r_vals_ai[idx] = idx;
            r_vals_ax[idx] = swapped ? OP_BINARY(y_vals[pos], val_x) : OP_BINARY(val_x, y_vals[pos]);
        }
    }
}
