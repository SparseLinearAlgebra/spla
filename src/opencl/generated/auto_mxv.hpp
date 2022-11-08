////////////////////////////////////////////////////////////////////
// Copyright (c) 2021 - 2022 JetBrains-Research
// Autogenerated file, do not modify
////////////////////////////////////////////////////////////////////

static const char source_mxv[] = R"(
void reduction_group(uint                   block_size,
                     uint                   lid,
                     volatile __local TYPE* s_sum) {
    if (BLOCK_SIZE >= block_size) {
        if (lid < (block_size / 2)) {
            s_sum[lid] = OP_BINARY2(s_sum[lid], s_sum[lid + (block_size / 2)]);
        }
        if (block_size > WARP_SIZE) {
            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }
}

__kernel void mxv_pull_prepare(__global const TYPE* g_mask,
                               __global const TYPE* g_init,
                               __global TYPE*       g_result,
                               __global uint*       g_config_size,
                               __global uint*       g_config,
                               const uint           n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        g_result[gid] = g_init[0];

        if (OP_SELECT(g_mask[gid])) {
            uint id      = atomic_inc(g_config_size);
            g_config[id] = gid;
        }
    }
}

__kernel void mxv_pull_exec(__global const uint* g_Ap,
                            __global const uint* g_Aj,
                            __global const TYPE* g_Ax,
                            __global const TYPE* g_vx,
                            __global const TYPE* g_init,
                            __global const uint* g_config,
                            __global TYPE*       g_result,
                            const uint           config_size) {
    const uint lid     = get_local_id(1);   // thread id in a row
    const uint lsize   = get_local_size(1); // num threads to process row
    const uint lgroup  = get_local_id(0);   // num of rows inside a group
    const uint gid     = get_global_id(0);  // id of config to touch
    const uint gstride = get_global_size(0);// step between config ids

    __local TYPE s_sum[BLOCK_COUNT][BLOCK_SIZE];

    for (int config_id = gid; config_id < config_size; config_id += gstride) {
        TYPE sum = g_init[0];

        const uint row_id = g_config[config_id];
        const uint start  = g_Ap[row_id];
        const uint end    = g_Ap[row_id + 1];

        for (uint i = start + lid; i < end; i += lsize) {
            const uint col_id = g_Aj[i];
            sum               = OP_BINARY2(sum, OP_BINARY1(g_Ax[i], g_vx[col_id]));
        }

        s_sum[lgroup][lid] = sum;
        barrier(CLK_LOCAL_MEM_FENCE);

        reduction_group(64, lid, s_sum[lgroup]);
        reduction_group(32, lid, s_sum[lgroup]);
        reduction_group(16, lid, s_sum[lgroup]);
        reduction_group(8, lid, s_sum[lgroup]);
        reduction_group(4, lid, s_sum[lgroup]);
        reduction_group(2, lid, s_sum[lgroup]);

        if (lid == 0) {
            g_result[row_id] = s_sum[lgroup][0];
        }
    }
}
)";