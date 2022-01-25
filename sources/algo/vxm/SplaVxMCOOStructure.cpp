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

#include <boost/compute/algorithm.hpp>
#include <boost/compute/algorithm/scatter_if.hpp>

#include <algo/vxm/SplaVxMCOOStructure.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <storage/block/SplaVectorCOO.hpp>
#include <utils/SplaProfiling.hpp>

#include <numeric>

bool spla::VxMCOOStructure::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVxM *>(&params);

    return p &&
           !p->tw->HasValues() &&
           p->w.Is<VectorCOO>() &&
           p->mask.Is<VectorCOO>() &&
           p->a.Is<VectorCOO>() &&
           p->b.Is<MatrixCOO>();
}

void spla::VxMCOOStructure::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    auto p = dynamic_cast<ParamsVxM *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto a = p->a.Cast<VectorCOO>();
    auto b = p->b.Cast<MatrixCOO>();
    auto mask = p->mask.Cast<VectorCOO>();
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);

    if (p->hasMask && !complementMask && mask.IsNull())
        return;

    if (a->GetNvals() == 0 || b->GetNvals() == 0)
        return;

    auto M = a->GetNrows();
    auto N = b->GetNcols();

    assert(!p->tw->HasValues());

    // Get row lengths and offsets for B
    const compute::vector<Index> *offsetsB;
    const compute::vector<Index> *lengthsB;

    compute::vector<Index> offsetsBuffer(ctx);
    compute::vector<Index> lengthsBuffer(ctx);

    if (b.Is<MatrixCSR>()) {
        // Get rows offsets and rows lengths for matrix b
        auto bCSR = b.Cast<MatrixCSR>();
        offsetsB = &bCSR->GetRowsOffsets();
        lengthsB = &bCSR->GetRowLengths();
    } else {
        // Compute new rows offsets and rows lengths for matrix b
        offsetsB = &offsetsBuffer;
        lengthsB = &lengthsBuffer;
        IndicesToRowOffsets(b->GetRows(), offsetsBuffer, lengthsBuffer, M, queue);
    }

    const std::size_t BUCKETS_COUNT = 7;
    compute::vector<Index> buckets(BUCKETS_COUNT, ctx);
    compute::vector<Index> bucketsOffsets(BUCKETS_COUNT, ctx);
    compute::vector<Index> bucketsConfig(ctx);
    std::vector<Index> bucketsHost(BUCKETS_COUNT);

    PF_SCOPE(vxm, "-vxm-");

    auto &bLengths = *lengthsB;
    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    BOOST_COMPUTE_CLOSURE(void, countBuckets, (unsigned int rowId), (bLengths, buckets), {
        const length = bLengths[rowId];
        if (length <= 0) return;
        const uint bucketId = (uint) (clamp(ceil(log2((float) length)) - 3.0f, 0.0f, 6.0f));
        atomic_add(&buckets[bucketId], 1u);
    });
    compute::for_each(a->GetRows().begin(), a->GetRows().end(), countBuckets, queue);
    compute::exclusive_scan(buckets.begin(), buckets.end(), bucketsOffsets.begin(), queue);
    compute::copy(buckets.begin(), buckets.end(), bucketsHost.begin(), queue);

    std::size_t rowsToProcess = std::reduce(bucketsHost.begin(), bucketsHost.end(), 0u);
    bucketsConfig.resize(rowsToProcess);

    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    BOOST_COMPUTE_CLOSURE(void, fillBuckets, (unsigned int rowId), (bLengths, buckets, bucketsOffsets, bucketsConfig), {
        const length = bLengths[rowId];
        if (length <= 0) return;
        const uint bucketId = (uint) (clamp(ceil(log2((float) length)) - 3.0f, 0.0f, 6.0f));
        const uint offset = atomic_add(&buckets[bucketId], 1u);
        bucketsConfig[bucketsOffsets[bucketId] + offset] = rowId;
    });
    compute::for_each(a->GetRows().begin(), a->GetRows().end(), fillBuckets, queue);

    PF_SCOPE_MARK(vxm, "buckets");

    compute::detail::meta_kernel kernel("__spla_vxm_structured");
    auto argCount = kernel.add_arg<cl_uint>("count");
    auto argVpt = kernel.add_arg<cl_uint>("vpt");
    auto argRpt = kernel.add_arg<cl_uint>("rpt");
    auto argRowOffsets = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowOffsets");
    auto argColIndices = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colIndices");
    auto argResult = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "result");
    auto argBucketId = kernel.add_arg<cl_uint>("bucketId");
    auto argBucketOffsets = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "bucketOffsets");
    auto argBucketConfig = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "bucketConfig");

    kernel << "const uint gid = get_group_id(0) * rpt + get_local_id(0) / vpt;\n"
           << "const uint lid = get_local_id(0) % vpt;\n"
           << "if (gid < count) {\n"
           << "    const uint rowId = bucketConfig[bucketOffsets[bucketId] + gid];\n"
           << "    for (uint k = rowOffsets[rowId] + lid; k < rowOffsets[rowId + 1]; k += vpt) {\n"
           << "        result[colIndices[k]] = 1u;\n"
           << "    }\n"
           << "}\n";

    compute::vector<Index> resultStructure(N + 1, ctx);
    compute::fill_n(resultStructure.begin(), N, 0u, queue);

    auto kernelCompiled = kernel.compile(ctx);
    kernelCompiled.set_arg(argRowOffsets, offsetsB->get_buffer());
    kernelCompiled.set_arg(argColIndices, b->GetCols().get_buffer());
    kernelCompiled.set_arg(argResult, resultStructure.get_buffer());
    kernelCompiled.set_arg(argBucketOffsets, bucketsOffsets.get_buffer());
    kernelCompiled.set_arg(argBucketConfig, bucketsConfig.get_buffer());

    auto dispatchGroup = [&](compute::command_queue &q, std::size_t bucketId, std::size_t vpt, std::size_t tpb) {
        std::size_t rowsToProcess = bucketsHost[bucketId];
        if (rowsToProcess > 0) {
            std::size_t rpt = tpb / vpt;
            std::size_t workSize = (rowsToProcess / rpt + (rowsToProcess % rpt ? 1 : 0)) * tpb;
            PF_SCOPE_SHOW(vxm, tpb << " " << vpt << " " << rowsToProcess);
            kernelCompiled.set_arg(argCount, static_cast<cl_uint>(rowsToProcess));
            kernelCompiled.set_arg(argVpt, static_cast<cl_uint>(vpt));
            kernelCompiled.set_arg(argRpt, static_cast<cl_uint>(rpt));
            kernelCompiled.set_arg(argBucketId, static_cast<cl_uint>(bucketId));
            q.enqueue_1d_range_kernel(kernelCompiled, 0, workSize, tpb);
        }
    };

    struct GroupInfo {
        compute::command_queue queue;
        std::size_t vpt;
        std::size_t tpb;
    };

    GroupInfo infos[] = {
            {compute::command_queue(ctx, device), 4, 32},
            {compute::command_queue(ctx, device), 8, 32},
            {compute::command_queue(ctx, device), 16, 32},
            {compute::command_queue(ctx, device), 32, 32},
            {compute::command_queue(ctx, device), 64, 64},
            {compute::command_queue(ctx, device), 128, 128},
            {compute::command_queue(ctx, device), 256, 256}};

    auto before = queue.enqueue_marker();

    for (std::size_t bucketId = 0; bucketId < BUCKETS_COUNT; bucketId++) {
        auto &info = infos[bucketId];
        info.queue.enqueue_barrier(before);
        dispatchGroup(info.queue, bucketId, info.vpt, info.tpb);
        queue.enqueue_barrier(info.queue.enqueue_marker());
    }

    PF_SCOPE_MARK(vxm, "define nnz");

    // Apply mask to define, which indices we are interested in
    compute::vector<Index> offsets(N + 1, ctx);
    if (p->hasMask && mask.IsNotNull()) {
        if (complementMask) {
            BOOST_COMPUTE_CLOSURE(void, assignZero, (unsigned int i), (resultStructure), { resultStructure[i] = 0u; });
            compute::for_each(mask->GetRows().begin(), mask->GetRows().end(), assignZero, queue);
            std::swap(resultStructure, offsets);
        } else {
            BOOST_COMPUTE_CLOSURE(void, assignOne, (unsigned int i), (resultStructure), { offsets[i] = resultStructure[i]; });
            compute::fill(offsets.begin(), offsets.end(), 0u, queue);
            compute::for_each(mask->GetRows().begin(), mask->GetRows().end(), assignOne, queue);
        }
    } else {
        std::swap(resultStructure, offsets);
    }

    PF_SCOPE_MARK(vxm, "apply mask");

    // Define offsets to copy values
    compute::exclusive_scan(offsets.begin(), offsets.end(), resultStructure.begin(), queue);
    std::swap(offsets, resultStructure);

    PF_SCOPE_MARK(vxm, "define offsets");

    // Get nnz
    std::size_t resultNnz = (offsets.end() - 1).read(queue);

    // Allocate buffer and store result
    if (resultNnz > 0) {
        compute::vector<Index> rows(resultNnz, ctx);
        BOOST_COMPUTE_CLOSURE(void, copyResult, (unsigned int i), (rows, offsets, resultStructure), {
            if (resultStructure[i])
                rows[offsets[i]] = i;
        });
        compute::for_each_n(compute::counting_iterator<unsigned int>(0), N, copyResult, queue);
        p->w = VectorCOO::Make(N, resultNnz, std::move(rows), compute::vector<unsigned char>(ctx)).As<VectorBlock>();

        PF_SCOPE_MARK(vxm, "copy result");
    }
}

spla::Algorithm::Type spla::VxMCOOStructure::GetType() const {
    return Type::VxM;
}

std::string spla::VxMCOOStructure::GetName() const {
    return "VxMCOOStructure";
}