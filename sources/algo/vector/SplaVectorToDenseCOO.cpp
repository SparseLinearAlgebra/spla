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

#include <algo/vector/SplaVectorToDenseCOO.hpp>
#include <compute/SplaCopyUtils.hpp>
#include <compute/SplaScatter.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaVectorCOO.hpp>
#include <storage/block/SplaVectorDense.hpp>
#include <utils/SplaProfiling.hpp>

bool spla::VectorToDenseCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVectorToDense *>(&params);

    return p &&
           p->v.Is<VectorCOO>();
}

void spla::VectorToDenseCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    PF_SCOPE(ctd, "-coo2dense-");

    auto p = dynamic_cast<ParamsVectorToDense *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto v = p->v.Cast<VectorCOO>();
    auto nrows = v->GetNrows();
    auto byteSize = p->byteSize;
    auto cooNvals = v->GetNvals();
    auto &cooRows = v->GetRows();
    auto &cooVals = v->GetVals();

    assert(nrows > 0);
    assert(cooNvals > 0);

    PF_SCOPE_MARK(ctd, "setup");

    // Dense storage for all values
    compute::vector<Index> denseMask(nrows, ctx);
    compute::vector<unsigned char> denseVals(ctx);

    BOOST_COMPUTE_CLOSURE(void, applyMask, (unsigned int rowId), (denseMask), { denseMask[rowId] = 1u; });
    compute::fill_n(denseMask.begin(), nrows, 0u, queue);
    compute::for_each(cooRows.begin(), cooRows.end(), applyMask, queue);

    PF_SCOPE_MARK(ctd, "gen mask");

    // Fill existing values
    if (byteSize > 0) {
        denseVals.resize(nrows * byteSize, queue);
        Scatter(cooRows.begin(), cooRows.end(), cooVals.begin(), denseVals.begin(), byteSize, queue);
    }

    PF_SCOPE_MARK(ctd, "scatter coo");

    // Save result as dense block
    p->w = VectorDense::Make(nrows, cooNvals, std::move(denseMask), std::move(denseVals)).As<VectorBlock>();
}

spla::Algorithm::Type spla::VectorToDenseCOO::GetType() const {
    return spla::Algorithm::Type::VectorToDense;
}

std::string spla::VectorToDenseCOO::GetName() const {
    return "SplaVectorToDenseCOO";
}
