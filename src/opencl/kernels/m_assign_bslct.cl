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

__kernel void assign_bslct_csr(__global const uint* g_Ap,
                                     __global const uint* g_Aj,
                                     __global const TYPE* g_mask,
                                     __global TYPE*       g_Rx,
                                    const TYPE           init,
                                    const uint           n) {
    uint gid     = get_global_id(0);
    uint gstride = get_global_size(0);
    const uint lid     = get_local_id(1);
    const uint lsize   = get_local_size(1);

    for (uint row = gid; row < n; row += gstride) {
        TYPE mask_row = g_mask[row];
        uint start = g_Ap[row];
        uint end   = g_Ap[row + 1];

       for (uint idx = start + lid; idx < end; idx += lsize) {
            const uint col = g_Aj[idx];
            if (OP_SELECT_BIN(mask_row, g_mask[col])) {
                g_Rx[idx] = OP_BINARY(g_Rx[idx], init);
            }
        }
    }
}
