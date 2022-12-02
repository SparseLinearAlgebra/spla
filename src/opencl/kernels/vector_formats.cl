#include "common.cl"

__kernel void sparse_to_dense(__global const uint* Ai,
                              __global const TYPE* Ax,
                              __global TYPE*       Rx,
                              const uint           n) {
    const uint gid     = get_global_id(0);
    const uint gstrige = get_global_size(0);

    for (uint i = gid; i < n; i += gstrige) {
        Rx[Ai[i]] = Ax[i];
    }
}

__kernel void dense_to_sparse(__global const TYPE* Ax,
                              __global uint*       Ri,
                              __global TYPE*       Rx,
                              __global uint*       count,
                              const uint           n) {
    const uint gid     = get_global_id(0);
    const uint gstrige = get_global_size(0);

    for (uint i = gid; i < n; i += gstrige) {
        if (Ax[i]) {
            const uint offset = atomic_inc(count);
            Ri[offset]        = i;
            Rx[offset]        = Ax[i];
        }
    }
}