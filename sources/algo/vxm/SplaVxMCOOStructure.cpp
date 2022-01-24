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
        IndicesToRowOffsets(b->GetRows(), offsetsBuffer, lengthsBuffer, N, queue);
    }

    PRF_S(timerNNZ);
    // Compute number of products for each a[i] x b[i,:]
    compute::vector<unsigned int> segmentLengths(a->GetNvals() + 1, ctx);
    compute::gather(a->GetRows().begin(), a->GetRows().end(), lengthsB->begin(), segmentLengths.begin(), queue);

    // Compute offsets between each a[i] x b[i,:] products
    compute::vector<unsigned int> outputPtr(a->GetNvals() + 1, ctx);
    compute::exclusive_scan(segmentLengths.begin(), segmentLengths.end(), outputPtr.begin(), 0u, queue);

    // Number of structural (atomic) operations to perform
    std::size_t cooNnz = (outputPtr.end() - 1).read(queue);
    PRF_F(timerNNZ, "cooNnz");

    PRF_S(timerAL);
    // For each b value to process define it's a value location
    compute::vector<Index> aLocations(cooNnz, ctx);
    compute::fill(aLocations.begin(), aLocations.end(), 0u, queue);
    compute::scatter_if(compute::counting_iterator<unsigned int>(0),
                        compute::counting_iterator<unsigned int>(a->GetNvals()),
                        outputPtr.begin(),
                        segmentLengths.begin(),
                        aLocations.begin(),
                        queue);
    PRF_F(timerAL, "aLocations scatter_if");
    PRF_S(timerALIS);
    compute::inclusive_scan(aLocations.begin(), aLocations.end(), aLocations.begin(), compute::max<unsigned int>(), queue);
    PRF_F(timerALIS, "aLocations inclusive_scan");

    // Compute structure of result
    PRF_S(timerRS);
    compute::vector<Index> resultStructure(N, ctx);
    compute::fill_n(resultStructure.begin(), N, 0u, queue);
    auto &aRows = a->GetRows();
    auto &bCols = b->GetCols();
    auto &bOffsets = *offsetsB;
    BOOST_COMPUTE_CLOSURE(void, defineNnz, (unsigned int i), (resultStructure, outputPtr, aLocations, aRows, bCols, bOffsets), {
        const uint rowIdxLocation = aLocations[i];
        const uint rowIdx = aRows[rowIdxLocation];
        const uint colIndexOffset = i - outputPtr[rowIdxLocation];
        resultStructure[bCols[bOffsets[rowIdx] + colIndexOffset]] = 1u;
    });
    compute::for_each_n(compute::counting_iterator<unsigned int>(0), cooNnz, defineNnz, queue);
    PRF_F(timerRS, "defineNnz");

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