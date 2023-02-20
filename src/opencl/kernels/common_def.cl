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

#pragma once

#define TYPE             int
#define BLOCK_SIZE       32
#define LM_NUM_MEM_BANKS 32
#define BLOCK_COUNT      1
#define WARP_SIZE        32
#define OP_SELECT(a)     a
#define OP_UNARY(a)      a
#define OP_BINARY(a, b)  a + b
#define OP_BINARY1(a, b) a + b
#define OP_BINARY2(a, b) a + b

#define __kernel
#define __global
#define __local
#define __constant
#define __private
#define restrict

#define half float

struct float2 {
    float x;
};
struct float3 {
    float x, y, z;
};
struct float4 {
    float x, y, z, w;
};

#define uint  unsigned int
#define ulong unsigned long int

enum cl_mem_fence_flags {
    CLK_LOCAL_MEM_FENCE,
    CLK_GLOBAL_MEM_FENCE
};

void barrier(cl_mem_fence_flags flags);

size_t get_global_size(uint dimindx);
size_t get_global_id(uint dimindx);
size_t get_local_size(uint dimindx);
size_t get_local_id(uint dimindx);
size_t get_num_groups(uint dimindx);
size_t get_group_id(uint dimindx);
size_t get_global_offset(uint dimindx);
uint   get_work_dim();

#define atomic_add(p, val)          p[0] += val
#define atomic_sub(p, val)          p[0] -= val
#define atomic_inc(p)               (p)[0]
#define atomic_dec(p)               (p)[0]
#define atomic_cmpxchg(p, cmp, val) ((p)[0] == cmp ? val : (p)[0])

#define min(x, y)     (x < y ? x : y)
#define max(x, y)     (x > y ? x : y)
#define sin(x)        x
#define cos(x)        x
#define fract(x, ptr) x