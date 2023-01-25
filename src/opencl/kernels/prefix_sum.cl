#include "common_incl.cl"
#include "defines.cl"

// gpu parallel prefix scan (blelloch)
// - https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda
// - http://users.umiacs.umd.edu/~ramani/cmsc828e_gpusci/ScanTalk.pdf

void prefix_sum_sweep_up(const uint             lid,
                         const uint             d,
                         const uint             offset,
                         volatile __local TYPE* s_values) {
    if (offset <= BLOCK_SIZE) {
        if (d * 2 > WARP_SIZE) {
            barrier(CLK_LOCAL_MEM_FENCE);
        }

        if (lid < d) {
            const int ai = LM_ADDR(offset * (2 * lid + 1) - 1);
            const int bi = LM_ADDR(offset * (2 * lid + 2) - 1);
            s_values[bi] = OP_BINARY(s_values[bi], s_values[ai]);
        }
    }
}

void prefix_sum_sweep_down(const uint             lid,
                           const uint             d,
                           const uint             offset,
                           volatile __local TYPE* s_values) {
    if (offset <= BLOCK_SIZE) {
        if (d > WARP_SIZE) {
            barrier(CLK_LOCAL_MEM_FENCE);
        }

        if (lid < d) {
            const int ai = LM_ADDR(offset * (2 * lid + 1) - 1);
            const int bi = LM_ADDR(offset * (2 * lid + 2) - 1);

            TYPE tmp     = s_values[ai];
            s_values[ai] = s_values[bi];
            s_values[bi] = OP_BINARY(s_values[bi], tmp);
        }
    }
}

__kernel void prefix_sum_prescan_unroll(__global TYPE* g_values,
                                        __global TYPE* g_carry,
                                        const uint     n) {
    const uint gid    = get_group_id(0);
    const uint lid    = get_local_id(0);
    const uint gstart = gid * BLOCK_SIZE * 2;

    __local TYPE s_values[LM_SIZE(BLOCK_SIZE * 2)];

    s_values[LM_ADDR(lid + BLOCK_SIZE * 0)] = (gstart + lid + BLOCK_SIZE * 0) < n ? g_values[gstart + lid + BLOCK_SIZE * 0] : 0;
    s_values[LM_ADDR(lid + BLOCK_SIZE * 1)] = (gstart + lid + BLOCK_SIZE * 1) < n ? g_values[gstart + lid + BLOCK_SIZE * 1] : 0;

    prefix_sum_sweep_up(lid, BLOCK_SIZE / 1, 1, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 2, 2, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 4, 4, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 8, 8, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 16, 16, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 32, 32, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 64, 64, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 128, 128, s_values);
    prefix_sum_sweep_up(lid, BLOCK_SIZE / 256, 256, s_values);

    if (lid == 0) {
        const uint ai = LM_ADDR(BLOCK_SIZE * 2 - 1);
        g_carry[gid]  = s_values[ai];
        s_values[ai]  = 0;
    }

    prefix_sum_sweep_down(lid, BLOCK_SIZE / 256, 256, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 128, 128, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 64, 64, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 32, 32, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 16, 16, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 8, 8, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 4, 4, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 2, 2, s_values);
    prefix_sum_sweep_down(lid, BLOCK_SIZE / 1, 1, s_values);

    barrier(CLK_LOCAL_MEM_FENCE);

    if ((gstart + lid + BLOCK_SIZE * 0) < n) g_values[gstart + lid + BLOCK_SIZE * 0] = s_values[LM_ADDR(lid + BLOCK_SIZE * 0)];
    if ((gstart + lid + BLOCK_SIZE * 1) < n) g_values[gstart + lid + BLOCK_SIZE * 1] = s_values[LM_ADDR(lid + BLOCK_SIZE * 1)];
}

__kernel void prefix_sum_prescan(__global TYPE* g_values,
                                 __global TYPE* g_carry,
                                 const uint     n) {
    const uint gid    = get_group_id(0);
    const uint lid    = get_local_id(0);
    const uint gstart = gid * BLOCK_SIZE * 2;

    __local TYPE s_values[LM_SIZE(BLOCK_SIZE * 2)];

    s_values[LM_ADDR(lid + BLOCK_SIZE * 0)] = (gstart + lid + BLOCK_SIZE * 0) < n ? g_values[gstart + lid + BLOCK_SIZE * 0] : 0;
    s_values[LM_ADDR(lid + BLOCK_SIZE * 1)] = (gstart + lid + BLOCK_SIZE * 1) < n ? g_values[gstart + lid + BLOCK_SIZE * 1] : 0;

    int offset = 1;

    for (uint d = BLOCK_SIZE; d > 0; d /= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);

        if (lid < d) {
            const int ai = LM_ADDR(offset * (2 * lid + 1) - 1);
            const int bi = LM_ADDR(offset * (2 * lid + 2) - 1);
            s_values[bi] = OP_BINARY(s_values[bi], s_values[ai]);
        }

        offset *= 2;
    }

    if (lid == 0) {
        const uint ai = LM_ADDR(BLOCK_SIZE * 2 - 1);
        g_carry[gid]  = s_values[ai];
        s_values[ai]  = 0;
    }

    for (uint d = 1; d <= BLOCK_SIZE; d *= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);

        offset /= 2;

        if (lid < d) {
            const int ai = LM_ADDR(offset * (2 * lid + 1) - 1);
            const int bi = LM_ADDR(offset * (2 * lid + 2) - 1);

            TYPE tmp     = s_values[ai];
            s_values[ai] = s_values[bi];
            s_values[bi] = OP_BINARY(s_values[bi], tmp);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if ((gstart + lid + BLOCK_SIZE * 0) < n) g_values[gstart + lid + BLOCK_SIZE * 0] = s_values[LM_ADDR(lid + BLOCK_SIZE * 0)];
    if ((gstart + lid + BLOCK_SIZE * 1) < n) g_values[gstart + lid + BLOCK_SIZE * 1] = s_values[LM_ADDR(lid + BLOCK_SIZE * 1)];
}

__kernel void prefix_sum_propagate(__global TYPE*       g_values,
                                   __global const TYPE* g_carry,
                                   const uint           n) {
    const uint gid = get_global_id(0) + 2 * BLOCK_SIZE;
    const uint cid = gid / (2 * BLOCK_SIZE);

    if (gid < n) {
        g_values[gid] = OP_BINARY(g_values[gid], g_carry[cid]);
    }
}