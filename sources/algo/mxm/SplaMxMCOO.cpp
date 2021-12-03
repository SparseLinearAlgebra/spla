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

#include <boost/compute/algorithm/scatter_if.hpp>

#include <algo/mxm/SplaMxMCOO.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaSortByRowColumn.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

using IndeciesVector = boost::compute::vector<unsigned int>;

namespace spla::detail {
    std::size_t CooSpmmHelper(std::size_t workspaceSize,
                              std::size_t beginRow,
                              std::size_t endRow,
                              std::size_t beginSegment,
                              std::size_t endSegment,
                              const MatrixCOO &a,
                              const MatrixCOO &b,
                              IndeciesVector &wRows,
                              IndeciesVector &wCols,
                              boost::compute::vector<unsigned char> &wVals,
                              std::size_t wValueByteSize,
                              const IndeciesVector &bRowOffsets,
                              const IndeciesVector &segmentLengths,
                              const IndeciesVector &outputPtr,
                              IndeciesVector &aGatherLocations,
                              IndeciesVector &bGatherLocations,
                              IndeciesVector &I,
                              IndeciesVector &J,
                              boost::compute::vector<unsigned char> &V,
                              const FunctionBinary &fMultiply,
                              const FunctionBinary &fAdd,
                              boost::compute::command_queue &queue) {
        using namespace boost;

        aGatherLocations.resize(workspaceSize, queue);
        bGatherLocations.resize(workspaceSize, queue);
        I.resize(workspaceSize, queue);
        J.resize(workspaceSize, queue);
        V.resize(workspaceSize * wValueByteSize, queue);

        // nothing to do
        if (workspaceSize == 0) {
            wRows.resize(0, queue);
            wCols.resize(0, queue);
            wVals.resize(0, queue);
            return 0;
        }

        const auto beginSegmentDiff = static_cast<std::ptrdiff_t>(beginSegment);
        const auto endSegmentDiff = static_cast<std::ptrdiff_t>(endSegment);

        // compute gather locations of intermediate format for 'a'
        compute::fill(aGatherLocations.begin(), aGatherLocations.end(), 0, queue);
        compute::scatter_if(compute::counting_iterator<unsigned int>(beginSegment),
                            compute::counting_iterator<unsigned int>(endSegment),
                            outputPtr.begin() + beginSegmentDiff,
                            segmentLengths.begin() + beginSegmentDiff,
                            aGatherLocations.begin() - (outputPtr.begin() + beginSegmentDiff).read(queue),
                            queue);

        compute::inclusive_scan(aGatherLocations.begin(),
                                aGatherLocations.end(),
                                aGatherLocations.begin(),
                                compute::max<unsigned int>(),
                                queue);

        compute::fill(bGatherLocations.begin(), bGatherLocations.end(), 0, queue);

        // compute gather locations of intermediate format for 'b'
        {
            auto &aCols = a.GetCols();
            BOOST_COMPUTE_CLOSURE(void, smudgeBGatherLoc, (unsigned int i), (beginSegmentDiff, aCols, bRowOffsets, aGatherLocations, bGatherLocations), {
                bGatherLocations[i] = bRowOffsets[aCols[beginSegmentDiff + aGatherLocations[i]]];
            });
            compute::for_each_n(compute::counting_iterator<unsigned int>(0), bGatherLocations.size(), smudgeBGatherLoc, queue);

            BOOST_COMPUTE_CLOSURE(void, computeBGatherLoc, (unsigned int i), (outputPtr, aGatherLocations, bGatherLocations), {
                bGatherLocations[i] += i - outputPtr[aGatherLocations[i]];
            });
            compute::for_each_n(compute::counting_iterator<unsigned int>(0), bGatherLocations.size(), computeBGatherLoc, queue);
        }

        compute::gather(aGatherLocations.begin(), aGatherLocations.end(),
                        a.GetRows().begin(),
                        I.begin(),
                        queue);
        compute::gather(bGatherLocations.begin(), bGatherLocations.end(),
                        b.GetCols().begin(),
                        J.begin(),
                        queue);

        TransformValues(aGatherLocations, bGatherLocations,
                        a.GetVals(), b.GetVals(),
                        V,
                        a.GetValueByteSize(), b.GetValueByteSize(), wValueByteSize,
                        fMultiply.GetSource(),
                        queue);

        // sort (I,J,V) tuples by (I,J)
        SortByRowColumn(I, J, V, wValueByteSize, queue);

        // sum values with the same (i,j)
        return ReduceByPairKey(I, J, V,
                               wRows, wCols, wVals,
                               wValueByteSize,
                               fAdd.GetSource(),
                               queue);
    }
}// namespace spla::detail

bool spla::MxMCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMxM *>(&params);

    return p &&
           p->w.Is<MatrixCOO>() &&
           p->mask.Is<MatrixCOO>() &&
           p->a.Is<MatrixCOO>() &&
           p->b.Is<MatrixCOO>();
}

void spla::MxMCOO::Process(spla::AlgorithmParams &algoParams) {
    using namespace boost;

    auto params = dynamic_cast<ParamsMxM *>(&algoParams);
    auto library = params->desc->GetLibrary().GetPrivatePtr();
    auto device = library->GetDeviceManager().GetDevice(params->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    MatrixCOO &a = *params->a.Cast<MatrixCOO>();
    MatrixCOO &b = *params->b.Cast<MatrixCOO>();

    const std::size_t wValueByteSize = params->tw->GetByteSize();

    if (params->hasMask && params->mask.Cast<MatrixCOO>().IsNull()) {
        return;
    }

    const auto &typeA = params->ta;
    const auto &typeB = params->tb;
    const auto &typeW = params->tw;
    auto hasValues = typeW->HasValues();
    auto valueByteSize = typeW->GetByteSize();
    assert(a.GetNcols() == b.GetNrows());

    // compute row offsets and row lengths for B
    compute::vector<unsigned int> bRowOffsets(ctx);
    compute::vector<unsigned int> bRowLengths(ctx);

    IndicesToRowOffsets(b.GetRows(), bRowOffsets, bRowLengths, b.GetNrows(), queue);

    // for each element A(i,j) compute the number of nonzero elements in B(j,:)
    compute::vector<unsigned int> segmentLengths(a.GetNvals() + 1, ctx);
    compute::gather(a.GetCols().begin(), a.GetCols().end(),
                    bRowLengths.begin(),
                    segmentLengths.begin(),
                    queue);

    // output pointer
    compute::vector<unsigned int> outputPtr(a.GetNvals() + 1, ctx);
    compute::exclusive_scan(segmentLengths.begin(), segmentLengths.end(),
                            outputPtr.begin(),
                            0U,
                            queue);

    std::size_t cooNumNonZeros = (outputPtr.end() - 1).read(queue);
    std::size_t workspaceCapacity = cooNumNonZeros;
    {
        // TODO: Find the right constant.
        // It probably should be clGetDeviceInfo(CL_DEVICE_LOCAL_MEM_SIZE)
        //
        // const std::size_t maxAvailableMemory = ctx.get_device().get_info<std::size_t>(CL_DEVICE_LOCAL_MEM_SIZE);

        const std::size_t free = 1 << 30;
        const std::size_t maxWorkspaceCapacity = free / (4 * sizeof(unsigned int) + valueByteSize);

        // use at most one third of the remaining capacity
        workspaceCapacity = std::min(maxWorkspaceCapacity / 3, workspaceCapacity);
    }

    compute::vector<unsigned int> aGatherLocations(ctx), bGatherLocations(ctx);
    compute::vector<unsigned int> I(ctx), J(ctx);
    compute::vector<unsigned char> V(ctx);
    compute::vector<unsigned int> wTmpRows(ctx), wTmpCols(ctx);
    compute::vector<unsigned char> wTmpVals(ctx);

    //    if (cooNumNonZeros <= workspaceCapacity) { TODO: Handle this!
    // compute W = A * B in one step

    assert(cooNumNonZeros <= workspaceCapacity);

    std::size_t beginRow = 0;
    std::size_t endRow = a.GetNrows();
    std::size_t beginSegment = 0;
    std::size_t endSegment = a.GetNvals();
    std::size_t workspaceSize = cooNumNonZeros;

    std::size_t wTmpNnz = detail::CooSpmmHelper(workspaceSize,
                                                beginRow, endRow,
                                                beginSegment, endSegment,
                                                a, b, wTmpRows, wTmpCols, wTmpVals, wValueByteSize,
                                                bRowOffsets,
                                                segmentLengths, outputPtr,
                                                aGatherLocations, bGatherLocations,
                                                I, J, V,
                                                *params->mult, *params->add,
                                                queue);

    if (wTmpNnz == 0) {
        return;
    }

    compute::vector<unsigned int> wRows(ctx);
    compute::vector<unsigned int> wCols(ctx);
    compute::vector<unsigned char> wVals(ctx);
    std::size_t wNnz;

    if (params->hasMask) {
        MatrixCOO &mask = *params->mask.Cast<MatrixCOO>();
        ApplyMask(mask.GetRows(), mask.GetCols(),
                  wTmpRows, wTmpCols, wTmpVals,
                  wRows, wCols, wVals, wValueByteSize,
                  queue);
        wNnz = wRows.size();
    } else {
        wRows = std::move(wTmpRows);
        wCols = std::move(wTmpCols);
        wVals = std::move(wTmpVals);
        wNnz = wTmpNnz;
    }

    if (wNnz == 0) {
        return;
    }

    params->w = MatrixCOO::Make(a.GetNrows(), b.GetNcols(), wNnz,
                                std::move(wRows), std::move(wCols), std::move(wVals))
                        .Cast<MatrixBlock>();

    //    } else {
    //        // decompose C = A * B into several C[slice,:] = A[slice,:] * B operations
    //        typedef typename cusp::coo_matrix<IndexType, ValueType, MemorySpace> Container;
    //        typedef typename std::list<Container> ContainerList;
    //
    //        // storage for C[slice,:] partial results
    //        ContainerList slices;
    //
    //        // compute row offsets for A
    //#if THRUST_VERSION >= 100800
    //        cusp::detail::temporary_array<IndexType, DerivedPolicy> A_row_offsets(exec, A.num_rows + 1);
    //#else
    //        cusp::array1d<IndexType, MemorySpace> A_row_offsets(A.num_rows + 1);
    //#endif
    //
    //        cusp::indices_to_offsets(exec, A.row_indices, A_row_offsets);
    //
    //// compute worspace requirements for each row
    //#if THRUST_VERSION >= 100800
    //        cusp::detail::temporary_array<IndexType, DerivedPolicy> cummulative_row_workspace(exec, A.num_rows);
    //#else
    //        cusp::array1d<IndexType, MemorySpace> cummulative_row_workspace(A.num_rows);
    //#endif
    //
    //        thrust::gather(exec,
    //                       A_row_offsets.begin() + 1, A_row_offsets.end(),
    //                       output_ptr.begin(),
    //                       cummulative_row_workspace.begin());
    //
    //        size_t begin_row = 0;
    //        size_t total_work = 0;
    //
    //        while (begin_row < size_t(A.num_rows)) {
    //            Container C_slice;
    //
    //            // find largest end_row such that the capacity of [begin_row, end_row) fits in the workspace_capacity
    //            size_t end_row = thrust::upper_bound(exec,
    //                                                 cummulative_row_workspace.begin() + begin_row, cummulative_row_workspace.end(),
    //                                                 IndexType(total_work + workspace_capacity)) -
    //                             cummulative_row_workspace.begin();
    //
    //            size_t begin_segment = A_row_offsets[begin_row];
    //            size_t end_segment = A_row_offsets[end_row];
    //
    //            // TODO throw exception signaling that there is insufficient memory (not necessarily bad_alloc)
    //            //if (begin_row == end_row)
    //            //    // workspace wasn't large enough, throw cusp::memory_allocation_failure?
    //
    //            size_t workspace_size = output_ptr[end_segment] - output_ptr[begin_segment];
    //
    //            total_work += workspace_size;
    //
    //            // TODO remove these when an exception is in place
    //            assert(end_row > begin_row);
    //            assert(workspace_size <= workspace_capacity);
    //
    //            coo_spmm_helper(exec,
    //                            workspace_size,
    //                            begin_row, end_row,
    //                            begin_segment, end_segment,
    //                            A, B, C_slice,
    //                            B_row_offsets,
    //                            segment_lengths, output_ptr,
    //                            A_gather_locations, B_gather_locations,
    //                            I, J, V,
    //                            combine, reduce);
    //
    //            slices.push_back(Container());
    //            slices.back().swap(C_slice);
    //
    //            begin_row = end_row;
    //        }

    //        // deallocate workspace
    //        // A_gather_locations.clear();
    //        // A_gather_locations.shrink_to_fit();
    //        // B_gather_locations.clear();
    //        // B_gather_locations.shrink_to_fit();
    //        // I.clear();
    //        // I.shrink_to_fit();
    //        // J.clear();
    //        // J.shrink_to_fit();
    //        // V.clear();
    //        // V.shrink_to_fit();
    //
    //        // compute total output size
    //        size_t C_num_entries = 0;
    //        for (typename ContainerList::iterator iter = slices.begin(); iter != slices.end(); ++iter)
    //            C_num_entries += iter->num_entries;
    //
    //        // resize output
    //        C.resize(A.num_rows, B.num_cols, C_num_entries);
    //
    //        // copy slices into output
    //        size_t base = 0;
    //        for (typename ContainerList::iterator iter = slices.begin(); iter != slices.end(); ++iter) {
    //            thrust::copy(exec, iter->row_indices.begin(), iter->row_indices.end(), C.row_indices.begin() + base);
    //            thrust::copy(exec, iter->column_indices.begin(), iter->column_indices.end(), C.column_indices.begin() + base);
    //            thrust::copy(exec, iter->values.begin(), iter->values.end(), C.values.begin() + base);
    //            base += iter->num_entries;
    //        }
}

spla::Algorithm::Type spla::MxMCOO::GetType() const {
    return Type::MxM;
}

std::string spla::MxMCOO::GetName() const {
    return "MxMCOO";
}
