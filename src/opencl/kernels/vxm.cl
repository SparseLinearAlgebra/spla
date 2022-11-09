#include "common.cl"

__kernel void vxm_prepare(__global const TYPE* g_vx,
                          __global const uint* g_Ap,
                          __global const TYPE* g_init,
                          __global TYPE*       g_rx,
                          __global uint*       g_config_size,
                          __global uint*       g_config,
                          const uint           n,
                          const uint           m) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        if (g_vx[gid] && (g_Ap[gid + 1] - g_Ap[gid]) > 0) {
            const uint id = atomic_inc(g_config_size);
            g_config[id]  = gid;
        }
    }
    if (gid < m) {
        g_rx[gid] = g_init[0];
    }
}

__kernel void vxm_exec(__global const TYPE* g_vx,
                       __global const uint* g_Ap,
                       __global const uint* g_Aj,
                       __global const TYPE* g_Ax,
                       __global const TYPE* g_mask,
                       __global const uint* g_config,
                       __global TYPE*       g_rx,
                       const uint           config_size,
                       const uint           m) {
    const uint lid     = get_local_id(1);   // thread id in a row
    const uint lsize   = get_local_size(1); // num threads to process row
    const uint gid     = get_global_id(0);  // id of config to touch
    const uint gstride = get_global_size(0);// step between config ids
    const uint pid     = get_group_id(1);   // id of out part to process
    const uint psize   = get_num_groups(1); // total parts to process

    const uint index_range_start = (m / psize) * pid;
    const uint index_range_end   = (pid + 1) == psize ? m : index_range_start + m / psize;

    for (int config_id = gid; config_id < config_size; config_id += gstride) {
        const uint row_id = g_config[config_id];
        const uint start  = g_Ap[row_id];
        const uint end    = g_Ap[row_id + 1];

        const TYPE vx = g_vx[row_id];

        for (uint i = start + lid; i < end; i += lsize) {
            const uint col_id = g_Aj[i];

            if (index_range_start <= col_id && col_id < index_range_end && OP_SELECT(g_mask[col_id])) {
                g_rx[col_id] = OP_BINARY2(g_rx[col_id], OP_BINARY1(vx, g_Ax[i]));
            }
        }
    }
}