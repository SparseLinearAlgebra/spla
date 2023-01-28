#include "common_def.cl"

__kernel void vxm_prepare(__global TYPE* g_rx,
                          const TYPE     init,
                          const uint     n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        g_rx[gid] = init;
    }
}

__kernel void vxm_atomic_vector(__global const TYPE* g_vx,
                                __global const uint* g_Ap,
                                __global const uint* g_Aj,
                                __global const TYPE* g_Ax,
                                __global const TYPE* g_mask,
                                __global TYPE*       g_rx,
                                const uint           n) {
    const uint lid     = get_local_id(1);   // thread id in a row
    const uint lsize   = get_local_size(1); // num threads to process row
    const uint gid     = get_global_id(0);  // id of row to touch
    const uint gstride = get_global_size(0);// step between row ids

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

__kernel void vxm_atomic_scalar(__global const TYPE* g_vx,
                                __global const uint* g_Ap,
                                __global const uint* g_Aj,
                                __global const TYPE* g_Ax,
                                __global const TYPE* g_mask,
                                __global TYPE*       g_rx,
                                const uint           n,
                                const uint           early_exit) {
    const uint gid     = get_global_id(0);  // id of row to touch
    const uint gstride = get_global_size(0);// step between row ids

    for (int row_id = gid; row_id < n; row_id += gstride) {
        const TYPE vx = g_vx[row_id];

        if (vx) {
            const uint start = g_Ap[row_id];
            const uint end   = g_Ap[row_id + 1];

            for (uint i = start; i < end; i += 1) {
                const uint col_id = g_Aj[i];
                const TYPE prod   = OP_BINARY1(vx, g_Ax[i]);

                if (OP_SELECT(g_mask[col_id])) {
                    bool success = false;
                    TYPE old     = g_rx[col_id];

                    while (!success) {
                        if (early_exit && old) break;

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

__kernel void vxm_config(__global const TYPE* g_vx,
                         __global TYPE*       g_rx,
                         __global uint*       g_config,
                         __global uint*       g_config_size,
                         const TYPE           init,
                         const uint           n,
                         const uint           m) {
    const uint gid     = get_global_id(0);
    const uint gstride = get_global_size(0);

    const uint w = min(n, m);
    uint       i = gid;

    while (i < w) {
        if (g_vx[i]) {
            const uint id = atomic_inc(g_config_size);
            g_config[id]  = i;
        }

        g_rx[i] = init;
        i += gstride;
    }

    while (i < n) {
        if (g_vx[i]) {
            const uint id = atomic_inc(g_config_size);
            g_config[id]  = i;
        }
        i += gstride;
    }

    while (i < m) {
        g_rx[i] = init;
        i += gstride;
    }
}

__kernel void vxm_config_atomic_scalar(__global const TYPE* g_vx,
                                       __global const uint* g_Ap,
                                       __global const uint* g_Aj,
                                       __global const TYPE* g_Ax,
                                       __global const TYPE* g_mask,
                                       __global const uint* g_config,
                                       __global TYPE*       g_rx,
                                       const uint           n,
                                       const uint           early_exit) {
    const uint gid     = get_global_id(0);  // id of row to touch
    const uint gstride = get_global_size(0);// step between row ids

    for (uint cid = gid; cid < n; cid += gstride) {
        const uint row_id = g_config[cid];
        const uint start  = g_Ap[row_id];
        const uint end    = g_Ap[row_id + 1];

        const TYPE vx = g_vx[row_id];

        for (uint i = start; i < end; i += 1) {
            const uint col_id = g_Aj[i];
            const TYPE prod   = OP_BINARY1(vx, g_Ax[i]);

            if (OP_SELECT(g_mask[col_id])) {
                bool success = false;
                TYPE old     = g_rx[col_id];

                while (!success) {
                    if (early_exit && old) break;

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

__kernel void vxm_sparse_count(__global const uint* g_vi,
                               __global const TYPE* g_vx,
                               __global const uint* g_Ap,
                               __global const uint* g_Aj,
                               __global const TYPE* g_mask,
                               __global uint*       g_size,
                               const uint           n) {
    const uint gid     = get_global_id(0);  // id of v entry to touch
    const uint gstride = get_global_size(0);// step between v entries

    for (int idx = gid; idx < n; idx += gstride) {
        const uint vi = g_vi[idx];
        const TYPE vx = g_vx[idx];

        if (vx) {
            const uint start = g_Ap[vi];
            const uint end   = g_Ap[vi + 1];

            uint count = 0;

            for (uint i = start; i < end; i++) {
                if (OP_SELECT(g_mask[g_Aj[i]])) count += 1;
            }

            atomic_add(g_size, count);
        }
    }
}

__kernel void vxm_sparse_collect(__global const uint* g_vi,
                                 __global const TYPE* g_vx,
                                 __global const uint* g_Ap,
                                 __global const uint* g_Aj,
                                 __global const TYPE* g_Ax,
                                 __global const TYPE* g_mask,
                                 __global uint*       g_ri,
                                 __global TYPE*       g_rx,
                                 __global uint*       g_roffset,
                                 const uint           n) {
    const uint gid     = get_global_id(0);  // id of v entry to touch
    const uint gstride = get_global_size(0);// step between v entries

    for (int idx = gid; idx < n; idx += gstride) {
        const uint vi = g_vi[idx];
        const TYPE vx = g_vx[idx];

        if (vx) {
            const uint start = g_Ap[vi];
            const uint end   = g_Ap[vi + 1];

            uint count = 0;

            for (uint i = start; i < end; i++) {
                if (OP_SELECT(g_mask[g_Aj[i]])) count += 1;
            }

            uint offset = atomic_add(g_roffset, count);

            for (uint i = start; i < end; i++) {
                const uint col_id = g_Aj[i];

                if (OP_SELECT(g_mask[col_id])) {
                    g_ri[offset] = col_id;
                    g_rx[offset] = OP_BINARY1(vx, g_Ax[i]);
                    offset += 1;
                }
            }
        }
    }
}

__kernel void vxm_sparse_reduce(__global uint* g_ri,
                                __global TYPE* g_rx,
                                __global uint* g_rsize,
                                const uint     n) {
    const uint gid = get_global_id(0);

    if (gid == 0) {
        uint write_offset = 0;
        uint read_offset  = 1;

        while (read_offset < n) {
            if (g_ri[write_offset] == g_ri[read_offset]) {
                g_rx[write_offset] = OP_BINARY2(g_rx[write_offset], g_rx[read_offset]);
            } else {
                write_offset += 1;
                g_ri[write_offset] = g_ri[read_offset];
                g_rx[write_offset] = g_rx[read_offset];
            }

            read_offset += 1;
        }

        g_rsize[0] = n > 0 ? write_offset + 1 : 0;
    }
}