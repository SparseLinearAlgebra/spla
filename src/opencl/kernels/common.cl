#define TYPE             int
#define BLOCK_SIZE       32
#define BLOCK_COUNT      1
#define WARP_SIZE        32
#define OP_SELECT(a)     a
#define OP_BINARY(a, b)  a + b
#define OP_BINARY1(a, b) a + b
#define OP_BINARY2(a, b) a + b

#define CLK_LOCAL_MEM_FENCE
#define CLK_GLOBAL_MEM_FENCE

#define __kernel
#define __global
#define __local

#define uint unsigned int

#define barrier(mem_fence)
#define get_group_id(dim)    0
#define get_local_id(dim)    0
#define get_global_id(dim)   0
#define get_local_size(dim)  1
#define get_global_size(dim) 1
#define atomic_add(p, val)   val
#define atomic_sub(p, val)   val
#define atomic_inc(p)        p[0]
#define atomic_dec(p)        p[0]