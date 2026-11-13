#include <spla.hpp>

#include <opencl/cl_merge.hpp>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

using uint = spla::uint;

static bool ensure_opencl() {
    auto* lib = spla::Library::get();
    if (lib->get_accelerator() != nullptr) {
        return true;
    }
    return lib->set_accelerator(spla::AcceleratorType::OpenCL) == spla::Status::Ok &&
           lib->get_accelerator() != nullptr;
}

static std::mt19937& generator() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

template<typename T>
static cl::Buffer make_padded_buffer(cl::Context& ctx, const std::vector<T>& data, uint offset, T pad) {
    std::vector<T> padded(offset + data.size(), pad);
    std::copy(data.begin(), data.end(), padded.begin() + offset);
    return cl::Buffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(T) * padded.size(), padded.data());
}

static bool check_union(const std::vector<uint>&  a_keys,
                        const std::vector<float>& a_vals,
                        const std::vector<uint>&  b_keys,
                        const std::vector<float>& b_vals,
                        uint                      a_offset = 0,
                        uint                      b_offset = 0,
                        uint                      r_offset = 0) {
    const uint n        = static_cast<uint>(a_keys.size());
    const uint m        = static_cast<uint>(b_keys.size());
    const uint capacity = n + m;

    auto* acc   = spla::get_acc_cl();
    auto& ctx   = acc->get_context();
    auto& queue = acc->get_queue_default();

    cl::Buffer cl_a_keys = make_padded_buffer<uint>(ctx, a_keys, a_offset, 0u);
    cl::Buffer cl_a_vals = make_padded_buffer<float>(ctx, a_vals, a_offset, 0.0f);
    cl::Buffer cl_b_keys = make_padded_buffer<uint>(ctx, b_keys, b_offset, 0u);
    cl::Buffer cl_b_vals = make_padded_buffer<float>(ctx, b_vals, b_offset, 0.0f);
    cl::Buffer cl_r_keys(ctx, CL_MEM_READ_WRITE, sizeof(uint) * (r_offset + capacity));
    cl::Buffer cl_r_vals(ctx, CL_MEM_READ_WRITE, sizeof(float) * (r_offset + capacity));

    auto* tmp_alloc = acc->get_alloc_tmp();
    auto  op        = spla::PLUS_FLOAT.template cast_safe<spla::TOpBinary<float, float, float>>();

    const uint total = spla::cl_merge_union<float>(queue,
                                                   cl_a_keys, cl_a_vals, a_offset, n,
                                                   cl_b_keys, cl_b_vals, b_offset, m,
                                                   cl_r_keys, cl_r_vals, r_offset,
                                                   op, tmp_alloc);

    tmp_alloc->free_all();
    queue.finish();

    std::vector<uint>  r_keys(total);
    std::vector<float> r_vals(total);
    if (total > 0) {
        queue.enqueueReadBuffer(cl_r_keys, CL_TRUE, sizeof(uint) * r_offset, sizeof(uint) * total, r_keys.data());
        queue.enqueueReadBuffer(cl_r_vals, CL_TRUE, sizeof(float) * r_offset, sizeof(float) * total, r_vals.data());
    }

    std::vector<std::pair<uint, float>> got(total);
    for (uint i = 0; i < total; ++i) {
        got[i] = {r_keys[i], r_vals[i]};
    }

    std::vector<std::pair<uint, float>> expected;
    expected.reserve(capacity);

    uint i = 0;
    uint j = 0;
    while (i < n && j < m) {
        if (a_keys[i] < b_keys[j]) {
            expected.emplace_back(a_keys[i], a_vals[i]);
            ++i;
        } else if (b_keys[j] < a_keys[i]) {
            expected.emplace_back(b_keys[j], b_vals[j]);
            ++j;
        } else {
            expected.emplace_back(a_keys[i], a_vals[i] + b_vals[j]);
            ++i;
            ++j;
        }
    }
    while (i < n) {
        expected.emplace_back(a_keys[i], a_vals[i]);
        ++i;
    }
    while (j < m) {
        expected.emplace_back(b_keys[j], b_vals[j]);
        ++j;
    }

    if (total != expected.size() || got != expected) {
        std::printf("UNION MISMATCH n=%u m=%u offsets=(%u,%u,%u): got %u, expected %u\n",
                    n, m, a_offset, b_offset, r_offset, total, static_cast<uint>(expected.size()));
        return false;
    }

    std::printf("UNION OK (n=%u m=%u total=%u offsets=%u/%u/%u)\n", n, m, total, a_offset, b_offset, r_offset);
    return true;
}

static bool check_intersect(const std::vector<uint>&  a_keys,
                            const std::vector<float>& a_vals,
                            const std::vector<uint>&  b_keys,
                            const std::vector<float>& b_vals,
                            uint                      a_offset = 0,
                            uint                      b_offset = 0,
                            uint                      r_offset = 0) {
    const uint n        = static_cast<uint>(a_keys.size());
    const uint m        = static_cast<uint>(b_keys.size());
    const uint capacity = std::min(n, m);

    auto* acc   = spla::get_acc_cl();
    auto& ctx   = acc->get_context();
    auto& queue = acc->get_queue_default();

    cl::Buffer cl_a_keys = make_padded_buffer<uint>(ctx, a_keys, a_offset, 0u);
    cl::Buffer cl_a_vals = make_padded_buffer<float>(ctx, a_vals, a_offset, 0.0f);
    cl::Buffer cl_b_keys = make_padded_buffer<uint>(ctx, b_keys, b_offset, 0u);
    cl::Buffer cl_b_vals = make_padded_buffer<float>(ctx, b_vals, b_offset, 0.0f);
    cl::Buffer cl_keys_ai(ctx, CL_MEM_READ_WRITE, sizeof(uint) * (r_offset + capacity));
    cl::Buffer cl_keys_ax(ctx, CL_MEM_READ_WRITE, sizeof(uint) * (r_offset + capacity));
    cl::Buffer cl_vals_ai(ctx, CL_MEM_READ_WRITE, sizeof(uint) * (r_offset + capacity));
    cl::Buffer cl_vals_ax(ctx, CL_MEM_READ_WRITE, sizeof(float) * (r_offset + capacity));

    auto* tmp_alloc = acc->get_alloc_tmp();
    auto  op        = spla::PLUS_FLOAT.template cast_safe<spla::TOpBinary<float, float, float>>();

    const uint total = spla::cl_merge_intersect<float>(queue,
                                                       cl_a_keys, cl_a_vals, a_offset, n,
                                                       cl_b_keys, cl_b_vals, b_offset, m,
                                                       cl_keys_ai, cl_keys_ax,
                                                       cl_vals_ai, cl_vals_ax,
                                                       r_offset, op, tmp_alloc);

    tmp_alloc->free_all();
    queue.finish();

    std::vector<uint>  keys_ai(total);
    std::vector<uint>  keys_ax(total);
    std::vector<uint>  vals_ai(total);
    std::vector<float> vals_ax(total);
    if (total > 0) {
        queue.enqueueReadBuffer(cl_keys_ai, CL_TRUE, sizeof(uint) * r_offset, sizeof(uint) * total, keys_ai.data());
        queue.enqueueReadBuffer(cl_keys_ax, CL_TRUE, sizeof(uint) * r_offset, sizeof(uint) * total, keys_ax.data());
        queue.enqueueReadBuffer(cl_vals_ai, CL_TRUE, sizeof(uint) * r_offset, sizeof(uint) * total, vals_ai.data());
        queue.enqueueReadBuffer(cl_vals_ax, CL_TRUE, sizeof(float) * r_offset, sizeof(float) * total, vals_ax.data());
    }

    std::vector<std::pair<uint, float>> expected;
    uint                                i = 0;
    uint                                j = 0;
    while (i < n && j < m) {
        if (a_keys[i] < b_keys[j]) {
            ++i;
        } else if (b_keys[j] < a_keys[i]) {
            ++j;
        } else {
            expected.emplace_back(a_keys[i], a_vals[i] + b_vals[j]);
            ++i;
            ++j;
        }
    }

    bool ok = total == expected.size();
    for (uint k = 0; ok && k < total; ++k) {
        ok = keys_ax[k] == expected[k].first &&
             vals_ax[k] == expected[k].second &&
             keys_ai[k] == r_offset + k &&
             vals_ai[k] == r_offset + k;
    }

    if (!ok) {
        std::printf("INTERSECT MISMATCH n=%u m=%u offsets=(%u,%u,%u): got %u, expected %u\n",
                    n, m, a_offset, b_offset, r_offset, total, static_cast<uint>(expected.size()));
        return false;
    }

    std::printf("INTERSECT OK (n=%u m=%u total=%u offsets=%u/%u/%u)\n", n, m, total, a_offset, b_offset, r_offset);
    return true;
}

static std::vector<uint> range_keys(uint from, uint count) {
    std::vector<uint> keys(count);
    for (uint i = 0; i < count; ++i) {
        keys[i] = from + i;
    }
    return keys;
}

static std::vector<uint> random_unique_keys(uint count, uint range) {
    std::vector<uint> pool(range);
    for (uint i = 0; i < range; ++i) {
        pool[i] = i;
    }
    std::shuffle(pool.begin(), pool.end(), generator());

    std::vector<uint> keys(pool.begin(), pool.begin() + count);
    std::sort(keys.begin(), keys.end());
    return keys;
}

int main() {
    if (!ensure_opencl()) {
        std::printf("OpenCL accelerator is not available\n");
        return 1;
    }

    bool ok = true;

    const std::vector<uint>  a_keys_small = {1, 3, 4, 7};
    const std::vector<float> a_vals_small = {1, 1, 1, 1};
    const std::vector<uint>  b_keys_small = {2, 4, 5, 7, 8};
    const std::vector<float> b_vals_small = {2, 2, 2, 2, 2};

    ok &= check_union(a_keys_small, a_vals_small, b_keys_small, b_vals_small);
    ok &= check_union(a_keys_small, a_vals_small, b_keys_small, b_vals_small, 3, 7, 5);
    ok &= check_intersect(a_keys_small, a_vals_small, b_keys_small, b_vals_small);
    ok &= check_intersect(a_keys_small, a_vals_small, b_keys_small, b_vals_small, 2, 5, 3);

    ok &= check_union({0, 1, UINT_MAX}, {1, 1, 1}, {0, 5, UINT_MAX}, {2, 2, 2}, 1, 2, 4);
    ok &= check_intersect({0, 1, UINT_MAX}, {1, 1, 1}, {0, 5, UINT_MAX}, {2, 2, 2}, 1, 2, 4);

    {
        const uint N = 130;
        ok &= check_union(range_keys(0, N), std::vector<float>(N, 1.0f),
                          range_keys(0, N), std::vector<float>(N, 2.0f), 9, 4, 6);
        ok &= check_intersect(range_keys(0, N), std::vector<float>(N, 1.0f),
                              range_keys(0, N), std::vector<float>(N, 2.0f), 9, 4, 6);
    }

    {
        const uint N = 1000000;
        ok &= check_union(range_keys(0, N), std::vector<float>(N, 1.0f),
                          range_keys(0, N), std::vector<float>(N, 2.0f));
        ok &= check_union(range_keys(0, N), std::vector<float>(N, 1.0f),
                          range_keys(N / 2, N), std::vector<float>(N, 2.0f));
        ok &= check_intersect(range_keys(0, N), std::vector<float>(N, 1.0f),
                              range_keys(N / 2, N), std::vector<float>(N, 2.0f));
    }

    {
        const uint                            N      = 20000;
        std::vector<uint>                     a_keys = random_unique_keys(5000, N);
        std::vector<uint>                     b_keys = random_unique_keys(3000, N);
        std::vector<float>                    a_vals(5000);
        std::vector<float>                    b_vals(3000);
        std::uniform_real_distribution<float> val_dist(0.0f, 1000.0f);

        for (auto& v : a_vals) v = val_dist(generator());
        for (auto& v : b_vals) v = val_dist(generator());

        ok &= check_union(a_keys, a_vals, b_keys, b_vals);
        ok &= check_union(a_keys, a_vals, b_keys, b_vals, 11, 13, 17);
        ok &= check_intersect(a_keys, a_vals, b_keys, b_vals, 11, 13, 17);
    }

    spla::Library::get()->finalize();
    return ok ? 0 : 1;
}
