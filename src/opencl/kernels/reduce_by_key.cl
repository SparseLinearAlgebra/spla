#include "common.cl"

// find first element in a sorted array such x <= element
uint lower_bound(const uint           x,
                 uint                 first,
                 uint                 size,
                 __global const uint* array) {
    while (size > 0) {
        int step = size / 2;

        if (array[first + step] < x) {
            first = first + step + 1;
            size -= step + 1;
        } else {
            size = step;
        }
    }
    return first;
}

// generate uint offsets for unique keys to store result
__kernel void reduce_by_key_generate_offsets(__global const uint* g_keys,
                                             __global uint*       g_offsets,
                                             const uint           n) {
    const uint gid = get_global_id(0);

    if (gid < n) {
        bool is_neq    = gid + 1 < n && g_keys[gid] != g_keys[gid + 1];
        g_offsets[gid] = is_neq ? 1 : 0;
    }
}

// scalar reduction for each group of keys
__kernel void reduce_by_key_naive(__global const TYPE* g_keys,
                                  __global const TYPE* g_values,
                                  __global const uint* g_offsets,
                                  __global uint*       g_unique_keys,
                                  __global TYPE*       g_reduce_values,
                                  const uint           n_keys,
                                  const uint           n_groups) {
    const uint gid = get_global_id(0);

    if (gid < n_groups) {
        const uint start_idx = lower_bound(gid, 0, n_keys, g_offsets);
        TYPE       value     = g_values[start_idx];

        for (uint i = start_idx + 1; i < n_keys && gid == g_offsets[i]; i += 1) {
            value = OP_BINARY(value, g_values[i]);
        }

        g_unique_keys[gid]   = g_keys[start_idx];
        g_reduce_values[gid] = value;
    }
}
