#include "common.cl"

void reduction_group(uint          block_size,
                     uint          lid,
                     __local TYPE* s_sum) {
    if (BLOCK_SIZE >= block_size) {
        if (lid < (block_size / 2)) {
            s_sum[lid] = OP_BINARY(s_sum[lid], s_sum[lid + (block_size / 2)]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

__kernel void reduce(__global const TYPE* g_vec,
                     __global TYPE*       g_sum,
                     const uint           n) {
    const uint gid   = get_global_id(0);
    const uint lid   = get_local_id(0);
    const uint gsize = get_global_size(0);

    uint i = gid;

    __local TYPE s_sum[BLOCK_SIZE];
    TYPE         sum = g_sum[0];

    while (i < n) {
        sum = OP_BINARY(sum, g_vec[i]);
        i += gsize;
    }

    s_sum[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    reduction_group(512, lid, s_sum);
    reduction_group(256, lid, s_sum);
    reduction_group(128, lid, s_sum);
    reduction_group(64, lid, s_sum);
    reduction_group(32, lid, s_sum);
    reduction_group(16, lid, s_sum);
    reduction_group(8, lid, s_sum);
    reduction_group(4, lid, s_sum);
    reduction_group(2, lid, s_sum);

    if (gid == 0) {
        g_sum[0] = s_sum[0];
    }
}