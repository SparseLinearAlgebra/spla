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

#include <algo/mxm/SplaMxMMaskedCSRCSC.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <compute/SplaStableSortByColumn.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <utils/SplaProfiling.hpp>

bool spla::MxMMaskedCSRCSC::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMxM *>(&params);

    // Requires a and bT be in csr format with mask in csr
    return p &&
           p->mask.Is<MatrixCSR>() &&
           p->a.Is<MatrixCSR>() &&
           p->bT.Is<MatrixCSR>() &&
           p->mask.IsNotNull() &&
           p->a.IsNotNull() &&
           p->bT.IsNotNull();
}

void spla::MxMMaskedCSRCSC::Process(spla::AlgorithmParams &algoParams) {
    using namespace boost;

    PF_SCOPE(mxm, "-mxm-m-csr-csc-");

    auto params = dynamic_cast<ParamsMxM *>(&algoParams);
    auto library = params->desc->GetLibrary().GetPrivatePtr();
    auto device = library->GetDeviceManager().GetDevice(params->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    SPDLOG_LOGGER_TRACE(library->GetLogger(), "Select masked csr x csc");
}

spla::Algorithm::Type spla::MxMMaskedCSRCSC::GetType() const {
    return spla::Algorithm::Type::MxM;
}

std::string spla::MxMMaskedCSRCSC::GetName() const {
    return "MxMMaskedCSRCSC";
}
