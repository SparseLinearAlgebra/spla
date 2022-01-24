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
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <compute/SplaSortByRow.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <storage/block/SplaVectorCOO.hpp>

#include <spla-cpp/SplaUtils.hpp>

#define PRF_S(tm) \
    CpuTimer tm;  \
    tm.Start();

#define PRF_F(tm, msg) \
    queue.finish();    \
    tm.Stop();         \
    std::cout << " ++++ " msg << " " << tm.GetElapsedMs() << std::endl;

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

    PRF_S(timerSM);
    // Allocate vector mask structure
    // Apply mask to define, which indices we are interested in
    compute::vector<Index> selectionMask(N + 1, ctx);
    if (p->hasMask) {
        if (mask.IsNotNull()) {
            if (complementMask) {
                BOOST_COMPUTE_CLOSURE(void, assignZero, (unsigned int i), (selectionMask), { selectionMask[i] = 0u; });
                compute::fill_n(selectionMask.begin(), N, 1u, queue);
                compute::for_each(mask->GetRows().begin(), mask->GetRows().end(), assignZero, queue);
            } else {
                BOOST_COMPUTE_CLOSURE(void, assignOne, (unsigned int i), (selectionMask), { selectionMask[i] = 1u; });
                compute::fill_n(selectionMask.begin(), N, 0u, queue);
                compute::for_each(mask->GetRows().begin(), mask->GetRows().end(), assignOne, queue);
            }
        } else {
            assert(complementMask);
            compute::fill_n(selectionMask.begin(), N, 1u, queue);
        }
    } else {
        compute::fill_n(selectionMask.begin(), N, 1u, queue);
    }
    PRF_F(timerSM, "selectionMask");

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

    PRF_S(timerBS);
    auto &bLengths = *lengthsB;
    std::vector<Index> bucketsHost(7);
    compute::vector<Index> buckets(7, ctx);
    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    BOOST_COMPUTE_CLOSURE(void, countBuckets, (unsigned int rowId), (bLengths, buckets), {
        const length = bLengths[rowId];
        if (length <= 8) {
            atomic_add(&buckets[0], 1u);
        } else if (length <= 16) {
            atomic_add(&buckets[1], 1u);
        } else if (length <= 32) {
            atomic_add(&buckets[2], 1u);
        } else if (length <= 64) {
            atomic_add(&buckets[3], 1u);
        } else if (length <= 128) {
            atomic_add(&buckets[4], 1u);
        } else if (length <= 256) {
            atomic_add(&buckets[5], 1u);
        } else {
            atomic_add(&buckets[6], 1u);
        }
    });
    compute::for_each(a->GetRows().begin(), a->GetRows().end(), countBuckets, queue);

    compute::copy(buckets.begin(), buckets.end(), bucketsHost.begin(), queue);
    compute::vector<Index> group32_4(bucketsHost[0], ctx);
    compute::vector<Index> group32_8(bucketsHost[1], ctx);
    compute::vector<Index> group32_16(bucketsHost[2], ctx);
    compute::vector<Index> group32_32(bucketsHost[3], ctx);
    compute::vector<Index> group64(bucketsHost[4], ctx);
    compute::vector<Index> group128(bucketsHost[5], ctx);
    compute::vector<Index> group256(bucketsHost[6], ctx);

    compute::fill(buckets.begin(), buckets.end(), 0u, queue);
    BOOST_COMPUTE_CLOSURE(void, fillBuckets, (unsigned int rowId), (bLengths, buckets, group32_4, group32_8, group32_16, group32_32, group64, group128, group256), {
        const length = bLengths[rowId];
        if (length <= 8) {
            const uint offset = atomic_add(&buckets[0], 1u);
            group32_4[offset] = rowId;
        } else if (length <= 16) {
            const uint offset = atomic_add(&buckets[1], 1u);
            group32_8[offset] = rowId;
        } else if (length <= 32) {
            const uint offset = atomic_add(&buckets[2], 1u);
            group32_16[offset] = rowId;
        } else if (length <= 64) {
            const uint offset = atomic_add(&buckets[3], 1u);
            group32_32[offset] = rowId;
        } else if (length <= 128) {
            const uint offset = atomic_add(&buckets[4], 1u);
            group64[offset] = rowId;
        } else if (length <= 256) {
            const uint offset = atomic_add(&buckets[5], 1u);
            group128[offset] = rowId;
        } else {
            const uint offset = atomic_add(&buckets[6], 1u);
            group256[offset] = rowId;
        }
    });
    compute::for_each(a->GetRows().begin(), a->GetRows().end(), fillBuckets, queue);
    PRF_F(timerBS, "buckets");

    PRF_S(timerNNZ);
    compute::detail::meta_kernel kernel("__spla_vxm_structured");
    auto argCount = kernel.add_arg<cl_uint>("count");
    auto argVpt = kernel.add_arg<cl_uint>("vpt");
    auto argRpt = kernel.add_arg<cl_uint>("rpt");
    auto argGroup = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "group");
    auto argRowOffsets = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowOffsets");
    auto argColIndices = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colIndices");
    auto argResult = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "result");

    kernel << "const uint gid = get_group_id(0) * rpt + get_local_id(0) / vpt;\n"
           << "const uint lid = get_local_id(0) % vpt;\n"
           << "if (gid < count) {\n"
           << "    const uint rowId = group[gid];\n"
           << "    for (uint k = rowOffsets[rowId] + lid; k < rowOffsets[rowId + 1]; k += vpt) {\n"
           << "        result[colIndices[k]] = 1u;\n"
           << "    }\n"
           << "}\n";

    compute::vector<Index> resultStructure(N, ctx);
    compute::fill_n(resultStructure.begin(), N, 0u, queue);

    auto kernelCompiled = kernel.compile(ctx);
    kernelCompiled.set_arg(argRowOffsets, offsetsB->get_buffer());
    kernelCompiled.set_arg(argColIndices, b->GetCols().get_buffer());
    kernelCompiled.set_arg(argResult, resultStructure.get_buffer());

    struct GroupInfo {
        compute::vector<Index> &group;
        compute::command_queue queue;
        std::size_t vpt;
        std::size_t tpb;
    };

    auto dispatchGroup = [&](compute::command_queue &q, compute::vector<Index> &group, std::size_t vpt, std::size_t tpb) {
        if (!group.empty()) {
            std::size_t rpt = tpb / vpt;
            std::size_t workSize = (group.size() / rpt + (group.size() % rpt ? 1 : 0)) * tpb;
            std::cout << " ++++ group " << tpb << " " << vpt << " " << group.size() << std::endl;
            kernelCompiled.set_arg(argCount, static_cast<cl_uint>(group.size()));
            kernelCompiled.set_arg(argVpt, static_cast<cl_uint>(vpt));
            kernelCompiled.set_arg(argRpt, static_cast<cl_uint>(rpt));
            kernelCompiled.set_arg(argGroup, group.get_buffer());
            q.enqueue_1d_range_kernel(kernelCompiled, 0, workSize, tpb);
        }
    };

    GroupInfo infos[] = {
            {group32_4, compute::command_queue(ctx, device), 4, 32},
            {group32_8, compute::command_queue(ctx, device), 8, 32},
            {group32_16, compute::command_queue(ctx, device), 16, 32},
            {group32_32, compute::command_queue(ctx, device), 32, 32},
            {group64, compute::command_queue(ctx, device), 64, 64},
            {group128, compute::command_queue(ctx, device), 128, 128},
            {group256, compute::command_queue(ctx, device), 256, 256}};

    auto before = queue.enqueue_marker();

    for (auto &info : infos) {
        info.queue.enqueue_barrier(before);
        dispatchGroup(info.queue, info.group, info.vpt, info.tpb);
        queue.enqueue_barrier(info.queue.enqueue_marker());
    }

    PRF_F(timerNNZ, "define nnz");

    // Define final offsets
    compute::vector<Index> offsets(N + 1, ctx);
    BOOST_COMPUTE_CLOSURE(unsigned int, applyMask, (unsigned int i), (resultStructure, selectionMask), { return resultStructure[i] & selectionMask[i]; });

    PRF_S(timerTS);
    compute::transform(compute::counting_iterator<unsigned int>(0), compute::counting_iterator<unsigned int>(N), offsets.begin(), applyMask, queue);
    PRF_F(timerTS, "transform v & m");
    PRF_S(timerESOF);
    compute::exclusive_scan(offsets.begin(), offsets.end(), selectionMask.begin(), queue);
    PRF_F(timerESOF, "exclusive_scan offsets");
    std::swap(offsets, selectionMask);

    // Get nnz
    std::size_t resultNnz = (offsets.end() - 1).read(queue);

    // Allocate buffer and store result
    if (resultNnz > 0) {
        PRF_S(timerCR);
        compute::vector<Index> rows(resultNnz, ctx);
        BOOST_COMPUTE_CLOSURE(void, copyResult, (unsigned int i), (rows, offsets, selectionMask), {
            if (selectionMask[i]) {
                rows[offsets[i]] = i;
            }
        });
        compute::for_each_n(compute::counting_iterator<unsigned int>(0), N, copyResult, queue);
        p->w = VectorCOO::Make(N, resultNnz, std::move(rows), compute::vector<unsigned char>(ctx)).As<VectorBlock>();
        PRF_F(timerCR, "copyResult");
    }
}

spla::Algorithm::Type spla::VxMCOOStructure::GetType() const {
    return Type::VxM;
}

std::string spla::VxMCOOStructure::GetName() const {
    return "VxMCOOStructure";
}