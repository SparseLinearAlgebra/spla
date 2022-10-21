#include "common.cl"

// Reduction inside single work ground
// Suppose, we have a group with size <= 1024 threads maximum
// Note: at the end we insert essential local mem barrier (hits perf on warp reduction)
void reduction_group(uint block_size, uint lid, __local TYPE* s_sum) {
    if (BLOCK_SIZE >= block_size) {
        if (lid < (block_size / 2)) {
            s_sum[lid] = OP1(s_sum[lid], s_sum[lid + (block_size / 2)]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

// Vector reduction to scalar value
// @param g_vec vector to reduce
// @param g_sum sum initial value and for the result
// @param n size of the vector
__kernel void reduce(__global const TYPE* g_vec, __global TYPE* g_sum, uint n) {
    uint gid   = get_global_id(0);
    uint lid   = get_local_id(0);
    uint gsize = get_global_size(0);
    uint lsize = get_local_size(0);

    uint i = gid;

    __local TYPE s_sum[BLOCK_SIZE];
    TYPE         sum = g_sum[0];

    // Count stride sum
    // with step of grid size
    while (i < n) {
        sum = OP1(sum, g_vec[i]);
        i += gsize;
    }

    // Save sum to local mem
    s_sum[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    // Perform group reduction
    // Use unrolled statements (without loop)
    reduction_group(512, lid, s_sum);
    reduction_group(256, lid, s_sum);
    reduction_group(128, lid, s_sum);
    reduction_group(64, lid, s_sum);
    reduction_group(32, lid, s_sum);
    reduction_group(16, lid, s_sum);
    reduction_group(8, lid, s_sum);
    reduction_group(4, lid, s_sum);
    reduction_group(2, lid, s_sum);

    // Save result
    if (gid == 0) {
        g_sum[0] = s_sum[0];
    }
}