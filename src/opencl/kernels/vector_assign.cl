#include "defines.cl"

__kernel void assign_dense_to_dense(__global TYPE*       g_r,
                                    __global const TYPE* g_mask,
                                    const TYPE           init,
                                    const uint           n) {
    uint gid     = get_global_id(0);
    uint gstride = get_global_size(0);

    for (uint i = gid; i < n; i += gstride) {
        if (OP_SELECT(g_mask[i])) {
            g_r[i] = OP_BINARY(g_r[i], init);
        }
    }
}

__kernel void assign_sparse_to_dense(__global TYPE*       g_r,
                                     __global const uint* g_maski,
                                     __global const TYPE* g_maskx,
                                     const TYPE           init,
                                     const uint           n) {
    uint gid     = get_global_id(0);
    uint gstride = get_global_size(0);

    for (uint idx = gid; idx < n; idx += gstride) {
        const TYPE mx = g_maskx[idx];
        if (OP_SELECT(mx)) {
            const uint mi = g_maski[idx];
            g_r[mi]       = OP_BINARY(g_r[mi], init);
        }
    }
}