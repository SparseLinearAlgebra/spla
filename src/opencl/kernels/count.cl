#include "common_def.cl"

__kernel void count_nz(__global const TYPE* g_vec,
                       __global uint*       g_count,
                       const uint           n) {
    const uint gid   = get_global_id(0);
    const uint gsize = get_global_size(0);
    uint       count = 0;

    for (uint i = gid; i < n; i += gsize) {
        if (g_vec[i] != 0) {
            count += 1;
        }
    }

    atomic_add(g_count, count);
}