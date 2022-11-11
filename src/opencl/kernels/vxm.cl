#include "common.cl"

__kernel void vxm_prepare(__global TYPE* g_rx,
                          const TYPE     init,
                          const uint     n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        g_rx[gid] = init;
    }
}

__kernel void vxm_exec_serial(__global const TYPE* g_vx,
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

__kernel void vxm_exec_atomic(__global const TYPE* g_vx,
                              __global const uint* g_Ap,
                              __global const uint* g_Aj,
                              __global const TYPE* g_Ax,
                              __global const TYPE* g_mask,
                              __global TYPE*       g_rx,
                              const uint           n) {
    const uint lid     = get_local_id(1);   // thread id in a row
    const uint lsize   = get_local_size(1); // num threads to process row
    const uint gid     = get_global_id(0);  // id of config to touch
    const uint gstride = get_global_size(0);// step between config ids

    for (int row_id = gid; row_id < n; row_id += gstride) {
        const uint start = g_Ap[row_id];
        const uint end   = g_Ap[row_id + 1];

        const TYPE vx = g_vx[row_id];

        if (vx) {
            for (uint i = start + lid; i < end; i += lsize) {
                const uint col_id = g_Aj[i];
                const TYPE prod   = OP_BINARY1(vx, g_Ax[i]);

                if (OP_SELECT(g_mask[col_id])) {
                    bool success = false;
                    TYPE old     = g_rx[col_id];

                    while (!success) {
                        const TYPE val = OP_BINARY2(old, prod);

                        if (val == old) break;

                        const TYPE res = atomic_cmpxchg(g_rx + col_id, old, val);

                        success = old == res;
                        old     = res;
                    }
                }
            }
        }
    }
}