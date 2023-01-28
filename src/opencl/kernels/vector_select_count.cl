#include "common_def.cl"

__kernel void select_count(__global const TYPE* g_v,
                           __global int*        g_count,
                           const uint           n) {
    const uint gid     = get_global_id(0);
    const uint gstride = get_global_size(0);

    int count = 0;

    for (uint i = gid; i < n; i += gstride) {
        if (OP_SELECT(g_v[i])) {
            count += 1;
        }
    }

    if (count > 0) {
        atomic_add(g_count, count);
    }
}