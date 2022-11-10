#define TYPE             int
#define BLOCK_SIZE       32
#define BLOCK_COUNT      1
#define WARP_SIZE        32
#define OP_SELECT(a)     a
#define OP_BINARY(a, b)  a + b
#define OP_BINARY1(a, b) a + b
#define OP_BINARY2(a, b) a + b

#define __kernel
#define __global
#define __local
#define __constant
#define __private

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

#define uint unsigned int

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
#define atomic_cmpxchg(p, cmp, val) (p)[0]