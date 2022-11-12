#include "common.cl"

__kernel void assign(__global TYPE*       g_r,
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