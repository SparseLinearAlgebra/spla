#include "common.cl"

// Vector masked assignment of scalar value
// @param g_r result vector to assign
// @param g_mask mask to select slots for assignment
// @param g_init scalar value to assign
// @param n dim of r vector
__kernel void assign(__global TYPE* g_r, __global const TYPE* g_mask, __global const TYPE* g_init, uint n) {
    uint gid  = get_global_id(0);
    TYPE init = g_init[0];

    if (gid < n) {
        if (OP_SELECT(g_mask[gid])) {
            g_r[gid] = OP_BINARY(g_r[gid], init);
        }
    }
}