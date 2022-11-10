#include "common.cl"

__kernel void select_count(__global const TYPE* g_v,
                           __global int*        g_count,
                           const uint           n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        if (OP_SELECT(g_v[gid])) {
            atomic_inc(g_count);
        }
    }
}