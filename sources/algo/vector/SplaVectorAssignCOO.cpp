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

#include <algo/vector/SplaVectorAssignCOO.hpp>
#include <compute/SplaGather.hpp>
#include <compute/SplaMaskByKey.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <storage/block/SplaVectorCOO.hpp>

bool spla::VectorAssignCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVectorAssign *>(&params);

    return p &&
           p->mask.Is<VectorCOO>();
}

void spla::VectorAssignCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    auto p = dynamic_cast<ParamsVectorAssign *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto s = p->s;
    auto size = p->size;
    auto type = p->type;
    auto mask = p->mask.Cast<VectorCOO>();
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);

    // Indices and values of the result
    compute::vector<unsigned int> rows(ctx);
    compute::vector<unsigned char> vals(ctx);

    // Assign full result
    if (!p->hasMask || (mask.IsNull() && complementMask)) {
        rows.resize(size, queue);
        compute::copy_n(compute::counting_iterator<unsigned int>(0), size, rows.begin(), queue);
    }
    // Has mask, must filter
    else {
        // Nothing to do
        if (mask.IsNull())
            return;

        // Apply direct mask
        if (!complementMask) {
            auto maskSize = mask->GetNvals();
            rows.resize(maskSize, queue);
            compute::copy_n(mask->GetRows().begin(), maskSize, rows.begin(), queue);
        }
        // Apply complement mask
        else {
            // Tmp indices to filter
            compute::vector<unsigned int> indices(size, ctx);
            compute::copy_n(compute::counting_iterator<unsigned int>(0), size, indices.begin(), queue);

            // Filter indices and save in rows result
            MaskKeys(mask->GetRows(), indices, rows, complementMask, queue);
        }
    }

    auto nvals = rows.size();

    // If type has values, gather it (for each nnz value assign scalar value)
    if (nvals > 0 && type->HasValues()) {
        vals.resize(nvals * type->GetByteSize(), queue);
        auto mapIdBegin = compute::constant_iterator<unsigned int>(0, 0);
        auto mapIdEnd = compute::constant_iterator<unsigned int>(0, nvals);
        Gather(mapIdBegin, mapIdEnd, s->GetVal().begin(), vals.begin(), type->GetByteSize(), queue);
    }

    // If after masking has values, store new block
    if (nvals)
        p->w = VectorCOO::Make(size, nvals, std::move(rows), std::move(vals)).As<VectorBlock>();
}

spla::Algorithm::Type spla::VectorAssignCOO::GetType() const {
    return spla::Algorithm::Type::VectorAssign;
}

std::string spla::VectorAssignCOO::GetName() const {
    return "VectorAssignCOO";
}