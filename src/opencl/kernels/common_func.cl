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

// memory bank conflict-free address and local buffer size
#ifdef LM_NUM_MEM_BANKS
    #define LM_ADDR(address) (address + ((address) / LM_NUM_MEM_BANKS))
    #define LM_SIZE(size)    (size + (size) / LM_NUM_MEM_BANKS)
#endif

#define SWAP_KEYS(x, y) \
    uint tmp1 = x;      \
    x         = y;      \
    y         = tmp1;

#define SWAP_VALUES(x, y) \
    TYPE tmp2 = x;        \
    x         = y;        \
    y         = tmp2;

// nearest power of two number greater equals n
uint ceil_to_pow2(uint n) {
    uint r = 1;
    while (r < n) r *= 2;
    return r;
}

// find first element in a sorted array such x <= element
uint lower_bound(const uint           x,
                 uint                 first,
                 uint                 size,
                 __global const uint* array) {
    while (size > 0) {
        int step = size / 2;

        if (array[first + step] < x) {
            first = first + step + 1;
            size -= step + 1;
        } else {
            size = step;
        }
    }
    return first;
}