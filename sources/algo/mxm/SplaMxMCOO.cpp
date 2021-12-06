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
#include <boost/compute/algorithm/transform.hpp>

#include <algo/mxm/SplaMxMCOO.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaSortByRowColumn.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>

using IndeciesVector = boost::compute::vector<unsigned int>;

namespace spla::detail {
    std::size_t CooSpmmHelper(std::size_t workspaceSize,
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
                              boost::compute::command_queue &queue,
                              const std::shared_ptr<spdlog::logger> &) {
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
        assert(beginSegmentDiff < outputPtr.size());
        const auto startShift = (outputPtr.begin() + beginSegmentDiff).read(queue);

        // compute gather locations of intermediate format for 'a'
        compute::fill(aGatherLocations.begin(), aGatherLocations.end(), 0, queue);
        {
            BOOST_COMPUTE_CLOSURE(void, calcAGatherLoc, (unsigned int i), (aGatherLocations, outputPtr, startShift, segmentLengths), {
                if (segmentLengths[i] != 0) {
                    aGatherLocations[outputPtr[i] - startShift] = i;
                }
            });
            assert(beginSegment <= outputPtr.size());
            assert(endSegment <= outputPtr.size());
            compute::for_each_n(compute::counting_iterator(beginSegment), endSegment, calcAGatherLoc, queue);
        }
        compute::inclusive_scan(aGatherLocations.begin(),
                                aGatherLocations.end(),
                                aGatherLocations.begin(),
                                compute::max<unsigned int>(),
                                queue);

        // compute gather locations of intermediate format for 'b'
        {
            const auto &aCols = a.GetCols();
            BOOST_COMPUTE_CLOSURE(void, computeBGatherLoc, (unsigned int i),
                                  (aCols, startShift, outputPtr, bRowOffsets, aGatherLocations, bGatherLocations),
                                  {
                                      bGatherLocations[i] = bRowOffsets[aCols[aGatherLocations[i]]] + i - (outputPtr[aGatherLocations[i]] - startShift);
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

    auto &logger = library->GetLogger();

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
        const std::size_t free = 1 << 15;
        const std::size_t maxWorkspaceCapacity = free / (4 * sizeof(unsigned int) + valueByteSize);

        // use at most one third of the remaining capacity
        workspaceCapacity = std::min(maxWorkspaceCapacity / 3, workspaceCapacity);
    }

    compute::vector<unsigned int> aGatherLocations(ctx), bGatherLocations(ctx);
    compute::vector<unsigned int> I(ctx), J(ctx);
    compute::vector<unsigned char> V(ctx);
    compute::vector<unsigned int> wTmpRows(ctx), wTmpCols(ctx);
    compute::vector<unsigned char> wTmpVals(ctx);

    std::size_t wTmpNnz = 0;

    //    if (cooNumNonZeros <= workspaceCapacity) {
    if (false) {
        // compute W = A * B in one step
        std::size_t beginSegment = 0;
        std::size_t endSegment = a.GetNvals();
        std::size_t workspaceSize = cooNumNonZeros;

        wTmpNnz = detail::CooSpmmHelper(workspaceSize,
                                        beginSegment, endSegment,
                                        a, b, wTmpRows, wTmpCols, wTmpVals, wValueByteSize,
                                        bRowOffsets,
                                        segmentLengths, outputPtr,
                                        aGatherLocations, bGatherLocations,
                                        I, J, V,
                                        *params->mult, *params->add,
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
        compute::vector<unsigned int> aRowOffsets(a.GetNrows() + 1, ctx);
        IndicesToRowOffsets(a.GetRows(), aRowOffsets, a.GetNrows(), queue);

        // compute workspace requirements for each row
        compute::vector<unsigned int> cumulativeRowWorkspace(a.GetNrows(), ctx);

        compute::gather(aRowOffsets.begin() + 1, aRowOffsets.end(),
                        outputPtr.begin(),
                        cumulativeRowWorkspace.begin(),
                        queue);

        std::ptrdiff_t beginRow = 0;
        std::size_t totalWork = 0;
        bool goodBefore = true;
        {
            compute::vector<unsigned int> eq(aRowOffsets.size() - 1, ctx);
            using compute::lambda::_1;
            using compute::lambda::_2;
            compute::transform(aRowOffsets.begin(), aRowOffsets.end() - 1, aRowOffsets.begin() + 1, eq.begin(), _1 > _2, queue);
            auto n = compute::accumulate(eq.begin(), eq.end(), 0, queue);
            CHECK_RAISE_CRITICAL_ERROR(n == 0, MemOpFailed, (std::string{"IT MUST ZERO!!! "} + std::to_string(n)));
            goodBefore = true;
        }

        while (static_cast<std::size_t>(beginRow) < a.GetNrows()) {
            // find the largest endRow such that the capacity of [beginRow, endRow) fits in the workspaceCapacity
            std::ptrdiff_t endRow = compute::upper_bound(cumulativeRowWorkspace.begin() + beginRow,
                                                         cumulativeRowWorkspace.end(),
                                                         totalWork + workspaceCapacity,
                                                         queue) -
                                    cumulativeRowWorkspace.begin();
            CHECK_RAISE_CRITICAL_ERROR(beginRow < endRow, MemOpFailed, "Workspace size isn't large enough to perform MxM");

            {
                compute::vector<unsigned int> eq(aRowOffsets.size() - 1, ctx);
                using compute::lambda::_1;
                using compute::lambda::_2;
                compute::transform(aRowOffsets.begin(), aRowOffsets.end() - 1, aRowOffsets.begin() + 1, eq.begin(), _1 > _2, queue);
                auto n = compute::accumulate(eq.begin(), eq.end(), 0, queue);
                CHECK_RAISE_CRITICAL_ERROR(n == 0, MemOpFailed, (std::string{"It was good before, but.. not now( IT MUST ZERO!!! "} + std::to_string(n)));
            }
            unsigned int beginSegment = (aRowOffsets.begin() + beginRow).read(queue);
            unsigned int endSegment = (aRowOffsets.begin() + endRow).read(queue);
            CHECK_RAISE_CRITICAL_ERROR(beginSegment < endSegment, MemOpFailed, "beginSegment >= endSegment :( " + std::to_string(beginSegment) + ' ' + std::to_string(endSegment));

            assert(endSegment < outputPtr.size());
            assert(beginSegment < outputPtr.size());
            {
                auto beginOffset = (outputPtr.begin() + beginSegment).read(queue);
                auto endOffset = (outputPtr.begin() + endSegment).read(queue);

                CHECK_RAISE_CRITICAL_ERROR(beginOffset < endOffset, MemOpFailed, (std::string{"No!!!!! "} + std::to_string(beginOffset) + ' ' + std::to_string(endOffset) + ' ' + std::to_string(beginSegment) + ' ' + std::to_string(endSegment)));
            }
            std::size_t workspaceSize = (outputPtr.begin() + endSegment).read(queue) - (outputPtr.begin() + beginSegment).read(queue);
            totalWork += workspaceSize;
            CHECK_RAISE_CRITICAL_ERROR(workspaceSize <= workspaceCapacity, MemOpFailed, (std::string{"Oh, I am so dead "} + std::to_string(workspaceSize) + ' ' + std::to_string(workspaceCapacity)));
            assert(workspaceSize <= workspaceCapacity);

            compute::vector<unsigned int> wSliceRows(ctx), wSliceCols(ctx);
            compute::vector<unsigned char> wSliceVals(ctx);
            wTmpNnz += detail::CooSpmmHelper(workspaceSize,
                                             beginSegment, endSegment,
                                             a, b, wSliceRows, wSliceCols, wSliceVals, wValueByteSize,
                                             bRowOffsets,
                                             segmentLengths, outputPtr,
                                             aGatherLocations, bGatherLocations,
                                             I, J, V,
                                             *params->mult, *params->add,
                                             queue, logger);
            slices.emplace_back(std::move(wSliceRows),
                                std::move(wSliceCols),
                                std::move(wSliceVals));
            beginRow = endRow;
        }

        // resize output
        wTmpRows.resize(wTmpNnz, queue);
        wTmpCols.resize(wTmpNnz, queue);
        wTmpVals.resize(wTmpNnz * wValueByteSize, queue);

        // copy slices into output
        std::ptrdiff_t base = 0;
        for (auto it = slices.begin(); it < slices.end(); ++it) {
            compute::copy(it->rows.begin(), it->rows.end(), wTmpRows.begin() + base, queue);
            compute::copy(it->cols.begin(), it->cols.end(), wTmpCols.begin() + base, queue);
            compute::copy(it->vals.begin(), it->vals.end(), wTmpVals.begin() + base * static_cast<std::ptrdiff_t>(wValueByteSize), queue);
            base += static_cast<std::ptrdiff_t>(it->GetNvals());
        }
        assert(static_cast<std::size_t>(base) == wTmpNnz);
    }

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
                  params->desc->IsParamSet(Descriptor::Param::MaskComplement),
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
}

spla::Algorithm::Type spla::MxMCOO::GetType() const {
    return Type::MxM;
}

std::string spla::MxMCOO::GetName() const {
    return "MxMCOO";
}
