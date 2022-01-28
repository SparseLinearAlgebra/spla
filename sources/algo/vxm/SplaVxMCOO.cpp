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

#include <algo/vxm/SplaVxMCOO.hpp>
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
#include <utils/SplaProfiling.hpp>

bool spla::VxMCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVxM *>(&params);

    return p &&
           p->w.Is<VectorCOO>() &&
           p->mask.Is<VectorCOO>() &&
           p->a.Is<VectorCOO>() &&
           p->b.Is<MatrixCOO>();
}

void spla::VxMCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    auto p = dynamic_cast<ParamsVxM *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    auto a = p->a.Cast<VectorCOO>();
    auto b = p->b.Cast<MatrixCOO>();
    auto mask = p->mask.Cast<VectorCOO>();
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);

    if (p->hasMask && !complementMask && mask.IsNull())
        return;

    if (a->GetNvals() == 0 || b->GetNvals() == 0)
        return;

    const auto &ta = p->ta;
    const auto &tb = p->tb;
    const auto &tw = p->tw;
    auto hasValues = tw->HasValues();
    auto M = a->GetNrows();
    auto N = b->GetNcols();

    // todo: beautify and avoid pinters usage?
    const compute::vector<Index> *offsets;
    const compute::vector<Index> *lengths;

    compute::vector<Index> offsetsBuffer(ctx);
    compute::vector<Index> lengthsBuffer(ctx);

    if (b.Is<MatrixCSR>()) {
        // Get rows offsets and rows lengths for matrix b
        auto bCSR = b.Cast<MatrixCSR>();
        offsets = &bCSR->GetRowsOffsets();
        lengths = &bCSR->GetRowLengths();
    } else {
        // Compute new rows offsets and rows lengths for matrix b
        offsets = &offsetsBuffer;
        lengths = &lengthsBuffer;
        IndicesToRowOffsets(b->GetRows(), offsetsBuffer, lengthsBuffer, M, queue);
    }

    PF_SCOPE(vxm, "-vxm-");

    // Compute number of products for each a[i] x b[i,:]
    compute::vector<unsigned int> segmentLengths(a->GetNvals() + 1, ctx);
    compute::gather(a->GetRows().begin(), a->GetRows().end(), lengths->begin(), segmentLengths.begin(), queue);

    PF_SCOPE_MARK(vxm, "segment lengths");

    // Compute offsets between each a[i] x b[i,:] products
    compute::vector<unsigned int> outputPtr(a->GetNvals() + 1, ctx);
    compute::exclusive_scan(segmentLengths.begin(), segmentLengths.end(), outputPtr.begin(), 0u, queue);

    PF_SCOPE_MARK(vxm, "output ptr");

    // Number of products to count
    std::size_t cooNnz = (outputPtr.end() - 1).read(queue);

    // nothing to do, no a[i] * b[i,:] product
    if (!cooNnz)
        return;

    // Determine location of a and b values (indices in coo arrays) to copy for products evaluation
    compute::vector<unsigned int> aLocations(cooNnz, ctx);
    compute::vector<unsigned int> bLocations(cooNnz, ctx);

    compute::fill(aLocations.begin(), aLocations.end(), 0u, queue);
    compute::scatter_if(compute::counting_iterator<unsigned int>(0),
                        compute::counting_iterator<unsigned int>(a->GetNvals()),
                        outputPtr.begin(),
                        segmentLengths.begin(),
                        aLocations.begin(),
                        queue);
    PF_SCOPE_MARK(vxm, "a-loc scatter_if");

    compute::inclusive_scan(aLocations.begin(), aLocations.end(), aLocations.begin(), compute::max<unsigned int>(), queue);
    PF_SCOPE_MARK(vxm, "a-loc scan");

    auto &aRows = a->GetRows();
    auto &offsetsRef = *offsets;
    BOOST_COMPUTE_CLOSURE(unsigned int, unfoldSegment, (unsigned int i), (outputPtr, offsetsRef, aRows, aLocations), {
        uint locationOfRowIndex = aLocations[i];
        uint rowIdx = aRows[locationOfRowIndex];
        uint rowBaseOffset = offsetsRef[rowIdx];
        uint offsetOfRowSegment = outputPtr[locationOfRowIndex];
        return rowBaseOffset + (i - offsetOfRowSegment);
    });
    compute::transform(compute::counting_iterator<unsigned int>(0), compute::counting_iterator<unsigned int>(cooNnz), bLocations.begin(), unfoldSegment, queue);
    PF_SCOPE_MARK(vxm, "b-loc");

    // Gather indices j for each product a[i] * b[i,j]
    compute::vector<unsigned int> J(cooNnz, ctx);
    compute::gather(bLocations.begin(), bLocations.end(), b->GetCols().begin(), J.begin(), queue);

    PF_SCOPE_MARK(vxm, "gather J");

    // Store final result here
    compute::vector<unsigned int> rows(ctx);
    compute::vector<unsigned char> vals(ctx);

    // Compute a[i] * b[i, j] for each value i and j
    if (hasValues) {
        compute::vector<unsigned char> V(cooNnz * tw->GetByteSize(), ctx);
        TransformValues(aLocations, bLocations,
                        a->GetVals(), b->GetVals(), V,
                        ta->GetByteSize(),
                        tb->GetByteSize(),
                        tw->GetByteSize(),
                        p->mult->GetSource(),
                        queue);

        PF_SCOPE_MARK(vxm, "transform");

        // Sort a[i] * b[i, j] products, so all j products stored in sequence
        SortByRow(J, V, tw->GetByteSize(), queue);

        PF_SCOPE_MARK(vxm, "sort by row");

        // Reduce all produces a[i] * b[i, j] for j using provided add op
        ReduceByKey(J, V, rows, vals, tw->GetByteSize(), p->add->GetSource(), queue);

        PF_SCOPE_MARK(vxm, "reduce by key");

        // Apply mask if required
        if (p->hasMask && mask.IsNotNull()) {
            compute::vector<unsigned int> tmpRows(ctx);
            compute::vector<unsigned char> tmpVals(ctx);
            ApplyMask(mask->GetRows(), rows, vals, tmpRows, tmpVals, tw->GetByteSize(), complementMask, queue);
            std::swap(rows, tmpRows);
            std::swap(vals, tmpVals);
        }

        PF_SCOPE_MARK(vxm, "apply mask");
    } else {
        // Sort result indices
        compute::sort(J.begin(), J.end(), queue);

        PF_SCOPE_MARK(vxm, "sort");

        // Reduce duplicates (keep only first entry)
        ReduceDuplicates(J, rows, queue);

        PF_SCOPE_MARK(vxm, "reduce duplicates");

        // Apply mask to indices
        if (p->hasMask && mask.IsNotNull()) {
            compute::vector<unsigned int> tmpRows(ctx);
            MaskKeys(mask->GetRows(), rows, tmpRows, complementMask, queue);
            std::swap(rows, tmpRows);
        }

        PF_SCOPE_MARK(vxm, "apply mask");
    }

    // Store result
    // NOTE: can be empty due mask application as the last step
    if (!rows.empty()) {
        auto nvals = rows.size();
        p->w = VectorCOO::Make(N, nvals, std::move(rows), std::move(vals)).As<VectorBlock>();
    }
}

spla::Algorithm::Type spla::VxMCOO::GetType() const {
    return Type::VxM;
}

std::string spla::VxMCOO::GetName() const {
    return "VxMCOO";
}