////////////////////////////////////////////////////////////////////
// Copyright (c) 2021 - 2023 SparseLinearAlgebra
// Autogenerated file, do not modify
////////////////////////////////////////////////////////////////////

#pragma once

static const char source_vector_eadd[] = R"(


__kernel void sparse_to_dense(__global TYPE*       g_rx,
                              __global const uint* g_vi,
                              __global const TYPE* g_vx,
                              __global uint*       g_fdbi,
                              __global TYPE*       g_fdbx,
                              __global uint*       g_fdb_size,
                              const uint           n) {
    const uint gid   = get_global_id(0);
    const uint gsize = get_global_size(0);

    for (uint k = gid; k < n; k += gsize) {
        const uint i    = g_vi[k];
        const TYPE prev = g_rx[i];

        g_rx[i] = OP_BINARY(prev, g_vx[k]);

        if (prev != g_rx[i]) {
            const uint offset = atomic_inc(g_fdb_size);
            g_fdbi[offset]    = i;
            g_fdbx[offset]    = g_rx[i];
        }
    }
}
)";