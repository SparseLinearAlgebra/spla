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

#include <algo/SplaAlgorithmParams.hpp>
#include <algo/vector/SplaVectorEWiseAddCOO.hpp>
#include <compute/SplaCopyUtils.hpp>
#include <compute/SplaGather.hpp>
#include <compute/SplaMaskByKey.hpp>
#include <compute/SplaMergeByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaVectorCOO.hpp>
#include <utils/SplaProfiling.hpp>


bool spla::VectorEWiseAddCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVectorEWiseAdd *>(&params);

    return p &&
           p->mask.Is<VectorCOO>() &&
           p->a.Is<VectorCOO>() &&
           p->b.Is<VectorCOO>();
}

void spla::VectorEWiseAddCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    PF_SCOPE(eadd, "-veadd-coo-");

    auto p = dynamic_cast<ParamsVectorEWiseAdd *>(&params);
    auto w = p->w;
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto &type = p->type;
    auto byteSize = type->GetByteSize();
    auto typeHasValues = type->HasValues();

    auto fillValuesPermutationIndices = [&](RefPtr<VectorCOO> &block, compute::vector<unsigned int> &perm) {
        if (block.IsNotNull() && typeHasValues) {
            auto nnz = block->GetNvals();
            perm.resize(nnz, queue);
            compute::copy_n(compute::make_counting_iterator(0), nnz, perm.begin(), queue);
        }
    };


    PF_SCOPE_MARK(eadd, "setup");

    auto blockA = p->a.Cast<VectorCOO>();
    const compute::vector<unsigned int> *rowsA = nullptr;
    compute::vector<unsigned int> permA(ctx);

    fillValuesPermutationIndices(blockA, permA);

    auto blockB = p->b.Cast<VectorCOO>();
    const compute::vector<unsigned int> *rowsB = nullptr;
    compute::vector<unsigned int> permB(ctx);

    fillValuesPermutationIndices(blockB, permB);

    // If mask is present, apply it to a and b blocks
    compute::vector<unsigned int> tmpRowsA(ctx);
    compute::vector<unsigned int> tmpRowsB(ctx);

    PF_SCOPE_MARK(eadd, "fill perm indices");

    auto maskBlock = p->mask.Cast<VectorCOO>();
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);

    auto applyMask = [&](RefPtr<VectorCOO> &block, compute::vector<unsigned int> &tmpRows, compute::vector<unsigned int> &perm, const compute::vector<unsigned int> *&out) {
        // Nothing to do
        if (block.IsNull())
            return;

        // No mask or must apply inverse mask (but !null = all)
        if (!p->hasMask || (complementMask && maskBlock.IsNull())) {
            out = &block->GetRows();
            return;
        }

        // No inverse and no block - null result
        if (!complementMask && maskBlock.IsNull())
            return;

        if (typeHasValues) {
            compute::vector<unsigned int> tmpPerm(ctx);
            MaskByKeys(maskBlock->GetRows(),
                       block->GetRows(), perm,
                       tmpRows, tmpPerm,
                       complementMask,
                       queue);
            std::swap(perm, tmpPerm);
        } else
            MaskKeys(maskBlock->GetRows(),
                     block->GetRows(),
                     tmpRows,
                     complementMask,
                     queue);

        out = &tmpRows;
    };

    applyMask(blockA, tmpRowsA, permA, rowsA);
    applyMask(blockB, tmpRowsB, permB, rowsB);

    PF_SCOPE_MARK(eadd, "apply mask");

    // If some block is empty (or both, save result as is and finish without merge)
    auto aEmpty = !rowsA || rowsA->empty();
    auto bEmpty = !rowsB || rowsB->empty();

    if (aEmpty || bEmpty) {
        auto setResult = [&](RefPtr<VectorCOO> &block, const compute::vector<unsigned int> *rows, compute::vector<unsigned int> &perm) {
            auto nnz = rows->size();
            compute::vector<unsigned int> newRows(*rows, queue);
            compute::vector<unsigned char> newVals(ctx);

            // Copy masked values if presented
            if (typeHasValues) {
                auto &vals = block->GetVals();
                newVals.resize(nnz * byteSize, queue);
                Gather(perm.begin(), perm.end(), vals.begin(), newVals.begin(), byteSize, queue);
            }

            p->w = VectorCOO::Make(block->GetNrows(), nnz, std::move(newRows), std::move(newVals)).As<VectorBlock>();
        };

        // Copy result of masking a block
        if (!aEmpty)
            setResult(blockA, rowsA, permA);

        // Copy result of masking b block
        if (!bEmpty)
            setResult(blockB, rowsB, permB);

        return;
    }

    // NOTE: offset b perm indices to preserve uniqueness
    if (typeHasValues)
        OffsetIndices(permB, blockA->GetNvals(), queue);

    PF_SCOPE_MARK(eadd, "offset indices");

    // Merge a and b values
    auto mergeCount = rowsA->size() + rowsB->size();
    compute::vector<unsigned int> mergedRows(mergeCount, ctx);
    compute::vector<unsigned int> mergedPerm(ctx);

    if (typeHasValues) {
        mergedPerm.resize(mergeCount, queue);
        MergeByKeys(rowsA->begin(), rowsA->end(), permA.begin(),
                    rowsB->begin(), rowsB->end(), permB.begin(),
                    mergedRows.begin(), mergedPerm.begin(),
                    queue);
    } else
        MergeKeys(rowsA->begin(), rowsA->end(),
                  rowsB->begin(), rowsB->end(),
                  mergedRows.begin(),
                  queue);

    PF_SCOPE_MARK(eadd, "merge");

    // Copy values to single buffer
    compute::vector<unsigned char> mergedValues(ctx);
    if (typeHasValues) {
        mergedValues.resize(mergeCount * byteSize, queue);
        CopyMergedValues(mergedPerm, blockA->GetVals(), blockB->GetVals(), mergedValues, blockA->GetNvals(), byteSize, queue);
    }

    PF_SCOPE_MARK(eadd, "copy merged values");

    // Reduce duplicates
    // NOTE: max 2 duplicated entries for each index
    compute::vector<unsigned int> resultRows(ctx);
    compute::vector<unsigned char> resultVals(ctx);
    std::size_t resultNvals;

    if (typeHasValues)
        resultNvals = ReduceDuplicates(mergedRows, mergedValues,
                                       resultRows, resultVals,
                                       byteSize,
                                       p->op->GetSource(),
                                       queue);
    else
        resultNvals = ReduceDuplicates(mergedRows,
                                       resultRows,
                                       queue);

    PF_SCOPE_MARK(eadd, "reduce duplicates");

    p->w = VectorCOO::Make(blockA->GetNrows(), resultNvals, std::move(resultRows), std::move(resultVals)).As<VectorBlock>();
    SPDLOG_LOGGER_TRACE(library->GetLogger(), "Merge vectors size={} nnz={}", blockA->GetNrows(), resultNvals);
}

spla::Algorithm::Type spla::VectorEWiseAddCOO::GetType() const {
    return Type::VectorEWiseAdd;
}

std::string spla::VectorEWiseAddCOO::GetName() const {
    return "VectorEWiseAddCOO";
}
