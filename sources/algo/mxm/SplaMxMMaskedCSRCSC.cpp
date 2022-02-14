/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#include <algo/mxm/SplaMxMMaskedCSRCSC.hpp>
#include <compute/metautil/SplaMetaUtil.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <utils/SplaProfiling.hpp>

#define HAS_VALUES_BEGIN if (hasValues) {
#define HAS_VALUES_END }

bool spla::MxMMaskedCSRCSC::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMxM *>(&params);

    // Requires a and bT be in csr format with mask in csr
    // todo: support complement mask (its seems easy,
    //  need to iterate over the row [0, N) and skip (i,j),
    //  which belong to the mask)
    return p &&
           !p->desc->IsParamSet(Descriptor::Param::MaskComplement) &&
           p->mask.Is<MatrixCSR>() &&
           p->a.Is<MatrixCSR>() &&
           p->bT.Is<MatrixCSR>() &&
           p->mask.IsNotNull() &&
           p->a.IsNotNull() &&
           p->bT.IsNotNull();
}

void spla::MxMMaskedCSRCSC::Process(spla::AlgorithmParams &algoParams) {
    using namespace boost;

    PF_SCOPE(mxm, "-mxm-m-csr-csc-");

    auto params = dynamic_cast<ParamsMxM *>(&algoParams);
    auto library = params->desc->GetLibrary().GetPrivatePtr();
    auto device = library->GetDeviceManager().GetDevice(params->deviceId);
    auto hasValues = params->tw->HasValues();

    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    // Algorithm strategy:
    // 1. Allocate temporary storage of size nnz(mask), since mask defines upper bound on result size
    // 2. For each mask value (i,j) evaluate product of A[i,:] x B^T[j,:]
    // 3. Use a warp (32/64 threads) per row
    // 4. Compact temporary storage into final storage (remove holes)

    MatrixCSR &m = *params->mask.Cast<MatrixCSR>();
    MatrixCSR &a = *params->a.Cast<MatrixCSR>();
    MatrixCSR &bT = *params->bT.Cast<MatrixCSR>();

    std::size_t M = a.GetNrows(),
                N = bT.GetNrows();

    auto platform = library->GetPlatform().name();
    auto npos = std::string::npos;

    std::size_t tpb = 32;

    if (platform.find("AMD") != npos ||
        platform.find("Amd") != npos ||
        platform.find("amd") != npos ||
        platform.find("Advanced Micro Devices") != npos)
        tpb = 64;

    std::size_t workSize = M * tpb;

    assert(m.GetNrows() == M);
    assert(m.GetNcols() == N);

    // Auxiliary binary search to find bCol in the bT row by aCol
    std::stringstream binSearch;
    binSearch << "uint binary_search(const uint x, __global const uint* buffer, const uint start, const uint range) {\n"
              << "    int l = start, r = start + range - 1;\n"
              << "    while (l <= r) {\n"
              << "        int k = (l + r) / 2;\n"
              << "        if (buffer[k] < x) l = k + 1;\n"
              << "        else if (x < buffer[k]) r = k - 1;\n"
              << "        else return k;\n"
              << "    }\n"
              << "    return 0xffffffff;\n"
              << "}\n";

    PF_SCOPE_MARK(mxm, "setup");

    const std::size_t BUCKETS_COUNT = 5;
    compute::vector<Index> buckets(BUCKETS_COUNT, ctx);
    compute::vector<Index> bucketsOffsets(BUCKETS_COUNT, ctx);
    compute::vector<Index> bucketsConfig(ctx);
    std::vector<Index> bucketsHost(BUCKETS_COUNT);

    auto &rowsLenM = m.GetRowLengths();
    BOOST_COMPUTE_CLOSURE(void, countBuckets, (unsigned int rid), (rowsLenM, buckets), {
        const uint length = rowsLenM[rid];
        if (length <= 0) return;
        const uint bucketId = (uint) (clamp(ceil(log2((float) length)) - 1.0f, 0.0f, 4.0f));
        atomic_add(&buckets[bucketId], 1u);
    });
    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    compute::for_each_n(compute::counting_iterator<unsigned int>(0), M, countBuckets, queue);
    compute::exclusive_scan(buckets.begin(), buckets.end(), bucketsOffsets.begin(), queue);
    compute::copy(buckets.begin(), buckets.end(), bucketsHost.begin(), queue);

    std::size_t rowsToProcess = std::reduce(bucketsHost.begin(), bucketsHost.end(), 0u);
    bucketsConfig.resize(rowsToProcess, queue);

    BOOST_COMPUTE_CLOSURE(void, fillBuckets, (unsigned int rid), (rowsLenM, buckets, bucketsOffsets, bucketsConfig), {
        const uint length = rowsLenM[rid];
        if (length <= 0) return;
        const uint bucketId = (uint) (clamp(ceil(log2((float) length)) - 1.0f, 0.0f, 4.0f));
        const uint offset = atomic_add(&buckets[bucketId], 1u);
        bucketsConfig[bucketsOffsets[bucketId] + offset] = rid;
    });
    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    compute::for_each_n(compute::counting_iterator<unsigned int>(0), M, fillBuckets, queue);

    PF_SCOPE_MARK(mxm, "buckets");

    // Rows len to evaluate exact size of each row (can be less or equal to the same mask row)
    compute::vector<unsigned int> rowsLenC(M + 1, ctx);
    compute::fill(rowsLenC.begin(), rowsLenC.end(), 0u, queue);

    // Allocate temporary buffers for result (use mask nnz)
    compute::vector<unsigned int> rowsC(m.GetNvals(), ctx);
    compute::vector<unsigned int> colsC(m.GetNvals(), ctx);
    compute::vector<unsigned char> valsC(hasValues ? m.GetNvals() * params->tw->GetByteSize() : 0, ctx);
    {
        PF_SCOPE_MARK(mxm, "allocate tmp");

        compute::detail::meta_kernel kernel("__spla_masked_csr_csc_prod");
        kernel.add_function("binary_search", binSearch.str());

        auto argRowsPtrM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrM");
        auto argColsM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsM");
        auto argRowsPtrA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrA");
        auto argColsA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsA");
        auto argRowsPtrB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrB");
        auto argColsB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsB");
        auto argRowsLenC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "rowsLenC");
        auto argRowsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "rowsC");
        auto argColsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "colsC");

        auto argCount = kernel.add_arg<cl_uint>("count");
        auto argRptb = kernel.add_arg<cl_uint>("rptb");
        auto argBucketId = kernel.add_arg<cl_uint>("bucketId");
        auto argBucketOffsets = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "bucketOffsets");
        auto argBucketConfig = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "bucketConfig");

        std::size_t argValsA;
        std::size_t argValsB;
        std::size_t argValsC;

        HAS_VALUES_BEGIN
        argValsA = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "valsA");
        argValsB = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "valsB");
        argValsC = kernel.add_arg<cl_uchar *>(compute::memory_object::global_memory, "valsC");

        kernel.add_function("mult_op", detail::meta::MakeFunction("mult_op",
                                                                  params->mult->GetSource(),
                                                                  detail::meta::Visibility::Global,
                                                                  detail::meta::Visibility::Global,
                                                                  detail::meta::Visibility::Unspecified));

        kernel.add_function("add_op", detail::meta::MakeFunction("add_op",
                                                                 params->add->GetSource(),
                                                                 detail::meta::Visibility::Unspecified,
                                                                 detail::meta::Visibility::Unspecified,
                                                                 detail::meta::Visibility::Unspecified));

        kernel.add_function("reduce_op", detail::meta::MakeFunction("reduce_op",
                                                                    params->add->GetSource(),
                                                                    detail::meta::Visibility::Local,
                                                                    detail::meta::Visibility::Local,
                                                                    detail::meta::Visibility::Local));
        HAS_VALUES_END

        kernel << "#define TPB " << static_cast<cl_uint>(tpb) << "\n";
        HAS_VALUES_BEGIN
        kernel << "#define BYTE_SIZE_A " << static_cast<cl_uint>(params->ta->GetByteSize()) << "\n"
               << "#define BYTE_SIZE_B " << static_cast<cl_uint>(params->tb->GetByteSize()) << "\n"
               << "#define BYTE_SIZE_C " << static_cast<cl_uint>(params->tw->GetByteSize()) << "\n"
               << "__local ulong buff_reduce[(TPB * BYTE_SIZE_C) /" << static_cast<cl_uint>(sizeof(cl_ulong)) << "];\n"
               << "__local uchar* p_reduce = (__local uchar*)buff_reduce;\n"
               << "__local uint p_reduce_flag[TPB];\n"
               << "uchar p_accum[BYTE_SIZE_C];\n";
        HAS_VALUES_END
        kernel << "const uint gid = get_group_id(0) * rptb;\n"
               << "const uint tid = get_local_id(0);\n"
               << "__local uint hasValue;\n"
               << "for (uint rtp = gid; rtp < count && rtp < gid + rptb; rtp++) {\n"
               << "    const rid = bucketConfig[bucketOffsets[bucketId] + rtp];\n"
               << "    const uint mRowStart = rowsPtrM[rid];\n"
               << "    const uint mRowEnd = rowsPtrM[rid + 1];\n"
               << "    uint writePos = mRowStart;\n"
               << "    for (uint mIdx = mRowStart; mIdx < mRowEnd; mIdx++) {\n"
               << "        if (tid == 0) hasValue = 0;\n"
               << "        barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "        const uint cid = colsM[mIdx];\n"
               << "        const uint aRowStart = rowsPtrA[rid];\n"
               << "        const uint aRowEnd = rowsPtrA[rid + 1];\n"
               << "        const uint bRowStart = rowsPtrB[cid];\n"
               << "        const uint bRowEnd = rowsPtrB[cid + 1];\n"
               << "        const uint bRowLen = bRowEnd - bRowStart;\n"
               << "        if (bRowLen > 0) {\n"
               << "            uint seenValue = 0;\n"
               << "            for (uint aIdx = aRowStart + tid; aIdx < aRowEnd; aIdx += TPB) {\n"
               << "                const uint aCol = colsA[aIdx];\n"
               << "                const uint bIdx = binary_search(aCol, colsB, bRowStart, bRowLen);\n"
               << "                if (bIdx != 0xffffffff) {\n ";
        HAS_VALUES_BEGIN
        kernel << "                    if (seenValue == 0) {\n"
               << "                        mult_op(&valsA[aIdx * BYTE_SIZE_A], &valsB[bIdx * BYTE_SIZE_B], &p_accum[0]);\n"
               << "                    } else {\n"
               << "                        uchar tmp[BYTE_SIZE_C];\n"
               << "                        mult_op(&valsA[aIdx * BYTE_SIZE_A], &valsB[bIdx * BYTE_SIZE_B], &tmp[0]);\n"
               << "                        add_op(&p_accum[0], &tmp[0], &p_accum[0]);\n"
               << "                    }\n";
        HAS_VALUES_END
        kernel << "                    seenValue = 1;\n"
               << "                }\n"
               << "            }\n"
               << "            atomic_or(&hasValue, seenValue);\n"
               << "            barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "            if (hasValue) {\n";
        HAS_VALUES_BEGIN
        kernel << "                p_reduce_flag[tid] = seenValue;\n"
               << "                for (uint i = 0; i < BYTE_SIZE_C; i++)\n"
               << "                    p_reduce[tid * BYTE_SIZE_C + i] = p_accum[i];\n"
               << "                for (uint stride = TPB/2; stride > 0; stride = stride / 2) {\n"
               << "                    barrier(CLK_LOCAL_MEM_FENCE);"
               << "                    if (tid < stride) {\n"
               << "                        if (p_reduce_flag[tid] && p_reduce_flag[tid + stride])\n"
               << "                            reduce_op(&p_reduce[tid * BYTE_SIZE_C], &p_reduce[(tid + stride) * BYTE_SIZE_C], &p_reduce[tid * BYTE_SIZE_C]);\n"
               << "                        else if (p_reduce_flag[tid + stride]) {\n"
               << "                            p_reduce_flag[tid] = 1;\n"
               << "                            for (uint i = 0; i < BYTE_SIZE_C; i++)\n"
               << "                                p_reduce[tid * BYTE_SIZE_C + i] = p_reduce[(tid + stride) * BYTE_SIZE_C + i];\n"
               << "                        }\n"
               << "                    }\n"
               << "                }\n";
        HAS_VALUES_END
        kernel << "                if (tid == 0) {\n"
               << "                    rowsC[writePos] = rid;\n"
               << "                    colsC[writePos] = cid;\n";
        HAS_VALUES_BEGIN
        kernel << "                    for (uint i = 0; i < BYTE_SIZE_C; i++)\n"
               << "                        valsC[writePos * BYTE_SIZE_C + i] = p_reduce[0 * BYTE_SIZE_C + i];\n";
        HAS_VALUES_END
        kernel << "                    writePos += 1;\n"
               << "                    atomic_inc(&rowsLenC[rid]);\n"
               << "                }\n"
               << "            }\n"
               << "        }\n"
               << "    }\n"
               << "}\n";

        auto compiledKernel = kernel.compile(ctx);
        compiledKernel.set_arg(argRowsPtrM, m.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsM, m.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrA, a.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsA, a.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrB, bT.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsB, bT.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsLenC, rowsLenC.get_buffer());
        compiledKernel.set_arg(argRowsC, rowsC.get_buffer());
        compiledKernel.set_arg(argColsC, colsC.get_buffer());

        HAS_VALUES_BEGIN
        compiledKernel.set_arg(argValsA, a.GetVals().get_buffer());
        compiledKernel.set_arg(argValsB, bT.GetVals().get_buffer());
        compiledKernel.set_arg(argValsC, valsC.get_buffer());
        HAS_VALUES_END

        compiledKernel.set_arg(argBucketConfig, bucketsConfig.get_buffer());
        compiledKernel.set_arg(argBucketOffsets, bucketsOffsets.get_buffer());

        auto dispatchGroup = [&](compute::command_queue &q, std::size_t bucketId, std::size_t rptb) {
            std::size_t rowsToProcess = bucketsHost[bucketId];
            if (rowsToProcess > 0) {
                std::size_t workSize = (rowsToProcess / rptb + (rowsToProcess % rptb ? 1 : 0)) * tpb;
                PF_SCOPE_SHOW(mxm, tpb << " " << rptb << " " << rowsToProcess);
                compiledKernel.set_arg(argCount, static_cast<cl_uint>(rowsToProcess));
                compiledKernel.set_arg(argRptb, static_cast<cl_uint>(rptb));
                compiledKernel.set_arg(argBucketId, static_cast<cl_uint>(bucketId));
                q.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
            }
        };

        struct GroupInfo {
            boost::compute::command_queue queue;
            std::size_t rptb;
        };

        GroupInfo infos[] = {
                {compute::command_queue(ctx, device), tpb / 2},
                {compute::command_queue(ctx, device), tpb / 4},
                {compute::command_queue(ctx, device), tpb / 8},
                {compute::command_queue(ctx, device), tpb / 16},
                {compute::command_queue(ctx, device), tpb / 32}};

        auto before = queue.enqueue_marker();

        for (std::size_t bucketId = 0; bucketId < BUCKETS_COUNT; bucketId++) {
            auto &info = infos[bucketId];
            info.queue.enqueue_barrier(before);
            dispatchGroup(info.queue, bucketId, info.rptb);
            queue.enqueue_barrier(info.queue.enqueue_marker());
        }

        PF_SCOPE_MARK(mxm, "evaluate");
    }

    // Evaluate offsets for each row of the result
    compute::vector<unsigned int> rowsPtrC(M + 1, ctx);
    compute::exclusive_scan(rowsLenC.begin(), rowsLenC.end(), rowsPtrC.begin(), 0u, queue);

    PF_SCOPE_MARK(mxm, "offsets");

    // Get nnz of the result
    std::size_t nvalsC = (rowsPtrC.end() - 1).read(queue);

    assert(nvalsC <= m.GetNvals());

    // Early exit, nothing to do
    if (nvalsC <= 0)
        return;

    // Need to compact result
    if (nvalsC < m.GetNvals()) {
        // Allocate final buffers for result (use nvalsC)
        compute::vector<unsigned int> dstRowsC(nvalsC, ctx);
        compute::vector<unsigned int> dstColsC(nvalsC, ctx);
        compute::vector<unsigned char> dstValsC(hasValues ? nvalsC * params->tw->GetByteSize() : 0, ctx);

        PF_SCOPE_MARK(mxm, "allocate final");

        compute::detail::meta_kernel kernel("__spla_masked_csr_csc_compact");

        auto argSrcRowsPtrC = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "srcRowsPtrC");
        auto argSrcRowsC = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "srcRowsC");
        auto argSrcColsC = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "srcColsC");
        auto argDstRowsPtrC = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "dstRowsPtrC");
        auto argDstRowsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "dstRowsC");
        auto argDstColsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "dstColsC");

        std::size_t argSrcValsC;
        std::size_t argDstValsC;

        HAS_VALUES_BEGIN
        argSrcValsC = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "srcValsC");
        argDstValsC = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "dstValsC");
        kernel << "#define BYTE_SIZE " << static_cast<cl_uint>(params->tw->GetByteSize()) << "\n";
        HAS_VALUES_END

        kernel << "#define TPB " << static_cast<cl_uint>(tpb) << "\n"
               << "const uint rid = get_group_id(0);\n"
               << "const uint tid = get_local_id(0);\n"
               << "const uint srcRowStart = srcRowsPtrC[rid];\n"
               << "const uint dstRowStart = dstRowsPtrC[rid];\n"
               << "const uint rowLength = dstRowsPtrC[rid + 1] - dstRowStart;\n"
               << "for (uint idx = tid; idx < rowLength; idx += TPB) {\n"
               << "    dstRowsC[dstRowStart + idx] = srcRowsC[srcRowStart + idx];\n"
               << "    dstColsC[dstRowStart + idx] = srcColsC[srcRowStart + idx];\n";
        HAS_VALUES_BEGIN
        kernel << "    for (uint i = 0; i < BYTE_SIZE; i++)\n"
               << "        dstValsC[(dstRowStart + idx) * BYTE_SIZE + i] = srcValsC[(srcRowStart + idx) * BYTE_SIZE + i];\n";
        HAS_VALUES_END
        kernel << "}\n";

        auto compiledKernel = kernel.compile(ctx);
        compiledKernel.set_arg(argSrcRowsPtrC, m.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argSrcRowsC, rowsC.get_buffer());
        compiledKernel.set_arg(argSrcColsC, colsC.get_buffer());
        compiledKernel.set_arg(argDstRowsPtrC, rowsPtrC.get_buffer());
        compiledKernel.set_arg(argDstRowsC, dstRowsC.get_buffer());
        compiledKernel.set_arg(argDstColsC, dstColsC.get_buffer());

        HAS_VALUES_BEGIN
        compiledKernel.set_arg(argSrcValsC, valsC.get_buffer());
        compiledKernel.set_arg(argDstValsC, dstValsC.get_buffer());
        HAS_VALUES_END

        queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
        PF_SCOPE_MARK(mxm, "compact");

        std::swap(rowsC, dstRowsC);
        std::swap(colsC, dstColsC);
        std::swap(valsC, dstValsC);
    }

    params->w = MatrixCSR::Make(M, N, nvalsC,
                                std::move(rowsC), std::move(colsC), std::move(valsC),
                                std::move(rowsPtrC), std::move(rowsLenC))
                        .As<MatrixBlock>();
}

spla::Algorithm::Type spla::MxMMaskedCSRCSC::GetType() const {
    return spla::Algorithm::Type::MxM;
}

std::string spla::MxMMaskedCSRCSC::GetName() const {
    return "MxMMaskedCSRCSC";
}

#undef HAS_VALUES_BEGIN
#undef HAS_VALUES_END