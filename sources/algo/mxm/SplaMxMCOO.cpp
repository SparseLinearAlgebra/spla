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

#include <algo/mxm/SplaMxMCOO.hpp>
#include <boost/compute/algorithm/scatter_if.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <compute/SplaSortByRowColumn.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

using IndeciesVector = boost::compute::vector<unsigned int>;
using ValuesVector = boost::compute::vector<unsigned char>;

namespace spla::detail {
    std::size_t CooSpmmHelper(std::size_t workspaceSize,
                              std::size_t beginSegment,
                              std::size_t endSegment,
                              const MatrixCOO &a,
                              const MatrixCOO &b,
                              IndeciesVector &wRows,
                              IndeciesVector &wCols,
                              ValuesVector &wVals,
                              std::size_t wValueByteSize,
                              const IndeciesVector &bRowOffsets,
                              const IndeciesVector &segmentLengths,
                              const IndeciesVector &outputPtr,
                              IndeciesVector &aGatherLocations,
                              IndeciesVector &bGatherLocations,
                              IndeciesVector &I,
                              IndeciesVector &J,
                              ValuesVector &V,
                              const RefPtr<FunctionBinary> &fMultiply,
                              const RefPtr<FunctionBinary> &fAdd,
                              boost::compute::command_queue &queue,
                              const std::shared_ptr<spdlog::logger> &) {
        using namespace boost;
        const bool typeHasValues = wValueByteSize != 0;

        aGatherLocations.resize(workspaceSize, queue);
        bGatherLocations.resize(workspaceSize, queue);
        I.resize(workspaceSize, queue);
        J.resize(workspaceSize, queue);

        if (typeHasValues) {
            V.resize(workspaceSize * wValueByteSize, queue);
        }

        // nothing to do
        if (workspaceSize == 0) {
            wRows.resize(0, queue);
            wCols.resize(0, queue);
            if (typeHasValues) {
                wVals.resize(0, queue);
            }
            return 0;
        }

        const auto beginSegmentDiff = static_cast<std::ptrdiff_t>(beginSegment);
        const auto startShift = (outputPtr.begin() + beginSegmentDiff).read(queue);

        // compute gather locations of intermediate format for 'a'
        compute::fill(aGatherLocations.begin(), aGatherLocations.end(), 0, queue);// On resize (if enlarge), new entries can store rubbish
        BOOST_COMPUTE_CLOSURE(void, calcAGatherLoc, (unsigned int i), (aGatherLocations, outputPtr, startShift, segmentLengths), {
            if (segmentLengths[i] != 0) {
                aGatherLocations[outputPtr[i] - startShift] = i;
            }
        });
        compute::for_each_n(compute::counting_iterator<unsigned int>(beginSegment), endSegment - beginSegment, calcAGatherLoc, queue);
        compute::inclusive_scan(aGatherLocations.begin(), aGatherLocations.end(), aGatherLocations.begin(), compute::max<unsigned int>(), queue);

        // compute gather locations of intermediate format for 'b'
        const auto &aCols = a.GetCols();
        BOOST_COMPUTE_CLOSURE(void, calcBGatherLoc, (unsigned int i), (aCols, startShift, outputPtr, bRowOffsets, aGatherLocations, bGatherLocations), {
            uint locationOfColIndex = aGatherLocations[i];
            uint col = aCols[locationOfColIndex];
            uint rowBaseOffset = bRowOffsets[col];
            uint offsetOfRowSegment = outputPtr[locationOfColIndex];
            bGatherLocations[i] = bRowOffsets[aCols[aGatherLocations[i]]] + i - (outputPtr[aGatherLocations[i]] - startShift);
        });
        compute::for_each_n(compute::counting_iterator<unsigned int>(0), bGatherLocations.size(), calcBGatherLoc, queue);

        compute::gather(aGatherLocations.begin(), aGatherLocations.end(),
                        a.GetRows().begin(),
                        I.begin(),
                        queue);
        compute::gather(bGatherLocations.begin(), bGatherLocations.end(),
                        b.GetCols().begin(),
                        J.begin(),
                        queue);

        if (typeHasValues) {
            TransformValues(aGatherLocations, bGatherLocations,
                            a.GetVals(), b.GetVals(),
                            V,
                            a.GetValueByteSize(), b.GetValueByteSize(), wValueByteSize,
                            fMultiply->GetSource(),
                            queue);

            // sort (I,J,V) tuples by (I,J)
            SortByRowColumn(I, J, V, wValueByteSize, queue);

            return ReduceByPairKey(I, J, V,
                                   wRows, wCols, wVals,
                                   wValueByteSize,
                                   fAdd->GetSource(),
                                   queue);
        } else {
            // sort (I,J,V) tuples by (I,J)
            SortByRowColumn(I, J, V, wValueByteSize, queue);

            // Only reduce duplicated indices, no values sum
            return ReduceDuplicates(I, J, wRows, wCols, queue);
        };
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
    const bool maskIsComplement = params->desc->IsParamSet(Descriptor::Param::MaskComplement);
    const bool maskIsNull = params->mask.Cast<MatrixCOO>().IsNull();
    const bool hasMask = params->hasMask;

    if (hasMask && maskIsNull && !maskIsComplement) {
        /**
         * Covered mask cases:
         *
         * has mask: 1, complement: 0, mask is null: 1
         */
        return;
    }

    auto &logger = library->GetLogger();

    const auto &typeA = params->ta;
    const auto &typeB = params->tb;
    const auto &typeW = params->tw;
    const bool hasValues = typeW->HasValues();
    const std::size_t valueByteSize = typeW->GetByteSize();
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
                            0u,
                            queue);

    std::size_t cooNumNonZeros = (outputPtr.end() - 1).read(queue);
    std::size_t workspaceCapacity = cooNumNonZeros;
    {
        const auto maxGlobalMem = device.global_memory_size();
        const auto maxAllocSize = device.get_info<cl_ulong>(CL_DEVICE_MAX_MEM_ALLOC_SIZE);

        // See issue #97 for more info https://github.com/JetBrains-Research/spla/issues/97
        // - CL_DEVICE_MAX_MEM_ALLOC_SIZE (max single allocation, nearly max single buffer size, CL_DEVICE_MAX_MEM_ALLOC_SIZE <= CL_DEVICE_GLOBAL_MEM_SIZE)
        // - CL_DEVICE_GLOBAL_MEM_SIZE (total device memory, might be virtualized)
        const std::size_t factor = std::max<std::size_t>(maxGlobalMem / maxAllocSize, 3);
        const std::size_t free = maxGlobalMem;
        const std::size_t maxWorkspaceCapacity = free / (6 * sizeof(unsigned int) + valueByteSize);
        const std::size_t maxWorkspaceCapacityToSelect = maxWorkspaceCapacity / factor;

        // use at most one third of the remaining capacity
        workspaceCapacity = std::min(maxWorkspaceCapacityToSelect, workspaceCapacity);

        // Log for info only
        SPDLOG_LOGGER_TRACE(logger, "Global mem={} KiB alloc={} KiB ({}%) required={} selected={} available={}",
                            maxGlobalMem / 1024, maxAllocSize / 1024,
                            static_cast<double>(maxAllocSize) / static_cast<double>(maxGlobalMem) * 100.0f,
                            cooNumNonZeros, workspaceCapacity, maxWorkspaceCapacityToSelect);
    }

    compute::vector<unsigned int> aGatherLocations(ctx), bGatherLocations(ctx);
    compute::vector<unsigned int> I(ctx), J(ctx);
    compute::vector<unsigned char> V(ctx);
    compute::vector<unsigned int> wRows(ctx), wCols(ctx);
    compute::vector<unsigned char> wVals(ctx);

    std::size_t wTmpNnz = 0;

    if (cooNumNonZeros <= workspaceCapacity) {
        // compute W = A * B in one step
        std::size_t beginSegment = 0;
        std::size_t endSegment = a.GetNvals();
        std::size_t workspaceSize = cooNumNonZeros;

        wTmpNnz = detail::CooSpmmHelper(workspaceSize,
                                        beginSegment, endSegment,
                                        a, b, wRows, wCols, wVals, wValueByteSize,
                                        bRowOffsets,
                                        segmentLengths, outputPtr,
                                        aGatherLocations, bGatherLocations,
                                        I, J, V,
                                        params->mult, params->add,
                                        queue, logger);
    } else {
        // decompose C = A * B into several C[slice,:] = A[slice,:] * B operations

        // storage for C[slice,:] partial results
        class MatrixSlice {
        public:
            MatrixSlice(compute::vector<unsigned int> &&mRows,
                        compute::vector<unsigned int> &&mCols,
                        compute::vector<unsigned char> &&mVals)
                : rows(std::move(mRows)), cols(std::move(mCols)), vals(std::move(mVals)) {}

            const compute::vector<unsigned int> rows;
            const compute::vector<unsigned int> cols;
            const compute::vector<unsigned char> vals;

            [[nodiscard]] std::size_t GetNvals() const {
                return rows.size();
            }
        };
        std::deque<MatrixSlice> slices;

        // compute row offsets for A
        compute::vector<unsigned int> aRowOffsets(ctx);
        IndicesToRowOffsets(a.GetRows(), aRowOffsets, a.GetNrows(), queue);

        // compute workspace requirements for each row
        compute::vector<unsigned int> cumulativeRowWorkspace(a.GetNrows(), ctx);
        compute::gather(aRowOffsets.begin() + 1, aRowOffsets.end(),
                        outputPtr.begin(),
                        cumulativeRowWorkspace.begin(),
                        queue);

        std::ptrdiff_t beginRow = 0;
        std::size_t totalWork = 0;

        while (static_cast<std::size_t>(beginRow) < a.GetNrows()) {
            // find the largest endRow such that the capacity of [beginRow, endRow) fits in the workspaceCapacity
            std::ptrdiff_t endRow = compute::upper_bound(cumulativeRowWorkspace.begin() + beginRow,
                                                         cumulativeRowWorkspace.end(),
                                                         totalWork + workspaceCapacity,
                                                         queue) -
                                    cumulativeRowWorkspace.begin();
            CHECK_RAISE_CRITICAL_ERROR(beginRow < endRow, MemOpFailed, "Workspace size isn't large enough to perform MxM");

            unsigned int beginSegment = (aRowOffsets.begin() + beginRow).read(queue);
            unsigned int endSegment = (aRowOffsets.begin() + endRow).read(queue);
            std::size_t workspaceSize = (outputPtr.begin() + endSegment).read(queue) - (outputPtr.begin() + beginSegment).read(queue);
            totalWork += workspaceSize;

            compute::vector<unsigned int> wSliceRows(ctx), wSliceCols(ctx);
            compute::vector<unsigned char> wSliceVals(ctx);
            wTmpNnz += detail::CooSpmmHelper(workspaceSize,
                                             beginSegment, endSegment,
                                             a, b, wSliceRows, wSliceCols, wSliceVals, wValueByteSize,
                                             bRowOffsets,
                                             segmentLengths, outputPtr,
                                             aGatherLocations, bGatherLocations,
                                             I, J, V,
                                             params->mult, params->add,
                                             queue, logger);
            slices.emplace_back(std::move(wSliceRows),
                                std::move(wSliceCols),
                                std::move(wSliceVals));
            beginRow = endRow;
        }

        // resize output
        wRows.resize(wTmpNnz, queue);
        wCols.resize(wTmpNnz, queue);

        if (hasValues) {
            wVals.resize(wTmpNnz * wValueByteSize, queue);
        }

        // copy slices into output
        std::ptrdiff_t base = 0;
        for (auto it = slices.begin(); it < slices.end(); ++it) {
            compute::copy(it->rows.begin(), it->rows.end(), wRows.begin() + base, queue);
            compute::copy(it->cols.begin(), it->cols.end(), wCols.begin() + base, queue);
            if (hasValues) {
                compute::copy(it->vals.begin(), it->vals.end(), wVals.begin() + base * static_cast<std::ptrdiff_t>(wValueByteSize), queue);
            }
            base += static_cast<std::ptrdiff_t>(it->GetNvals());
        }
    }

    if (wTmpNnz == 0) {
        // Nothing to do
        return;
    }

    std::size_t wNnz = wTmpNnz;

    if (hasMask && !maskIsNull) {
        /**
         * Covered mask cases:
         *
         * has mask: 1, complement: 0, mask is null: 0
         * has mask: 1, complement: 1, mask is null: 0
         */
        compute::vector<unsigned int> wTmpRows(ctx);
        compute::vector<unsigned int> wTmpCols(ctx);
        compute::vector<unsigned char> wTmpVals(ctx);
        MatrixCOO &mask = *params->mask.Cast<MatrixCOO>();
        ApplyMask(mask.GetRows(), mask.GetCols(),
                  wRows, wCols, wVals,
                  wTmpRows, wTmpCols, wTmpVals,
                  wValueByteSize,
                  params->desc->IsParamSet(Descriptor::Param::MaskComplement),
                  queue);
        wNnz = wTmpRows.size();
        std::swap(wTmpRows, wRows);
        std::swap(wTmpCols, wCols);
        std::swap(wTmpVals, wVals);
    }
    /**
     * Else Covered mask cases:
     *
     * has mask: 0, complement: -, mask is null: -
     * has mask: 1, complement: 1, mask is null: 1
     */

    if (wNnz)
        params->w = MatrixCOO::Make(a.GetNrows(), b.GetNcols(), wNnz, std::move(wRows), std::move(wCols), std::move(wVals)).Cast<MatrixBlock>();
}

spla::Algorithm::Type spla::MxMCOO::GetType() const {
    return Type::MxM;
}

std::string spla::MxMCOO::GetName() const {
    return "MxMCOO";
}
