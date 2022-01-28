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

#include <algo/matrix/SplaMatrixReduceScalarCOO.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaReduce2.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>


bool spla::MatrixReduceScalarCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMatrixReduceScalar *>(&params);

    return p &&
           p->mask.Is<MatrixCOO>() &&
           p->matrix.Is<MatrixCOO>();
}

void spla::MatrixReduceScalarCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    auto p = dynamic_cast<ParamsMatrixReduceScalar *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    auto &type = p->type;
    auto byteSize = type->GetByteSize();

    auto matrix = p->matrix.Cast<MatrixCOO>();
    auto mask = p->mask.Cast<MatrixCOO>();
    auto reduce = p->reduce;
    auto complementMask = desc->IsParamSet(Descriptor::Param::MaskComplement);
    bool hasMask = p->hasMask;

    CHECK_RAISE_ERROR(!(!p->hasMask && complementMask), InvalidState, "MaskComplement is set to true, but flag hasMask is false");

    // Nothing to do with a matrix that has been emptied by the mask
    if (p->hasMask && !complementMask && mask.IsNull()) {
        p->scalar = nullptr;
        return;
    }

    // Mask is actually absent
    if (p->hasMask && complementMask && mask.IsNull()) {
        hasMask = false;
    }

    // Nothing to do with an empty matrix
    if (matrix.IsNull()) {
        p->scalar = nullptr;
        return;
    }

    auto reduceValues = [&](const compute::vector<unsigned char> &values) {
        return Reduce2(values, byteSize, reduce->GetSource(), queue);
    };

    // Apply finally mask if required
    if (hasMask && mask.IsNotNull()) {
        // TODO: Avoid usage of cols and rows to reduce memory usage

        compute::vector<unsigned int> maskedRows(ctx);
        compute::vector<unsigned int> maskedCols(ctx);
        compute::vector<unsigned char> maskedVals(ctx);

        ApplyMask(mask->GetRows(),
                  mask->GetCols(),
                  matrix->GetRows(), matrix->GetCols(), matrix->GetVals(),
                  maskedRows, maskedCols, maskedVals,
                  byteSize,
                  complementMask,
                  queue);
        std::size_t maskedSize = maskedRows.size();

        if (maskedSize == 0) {
            p->scalar = nullptr;
            return;
        }

        p->scalar = ScalarValue::Make(reduceValues(maskedVals));
    } else {
        p->scalar = ScalarValue::Make(reduceValues(matrix->GetVals()));
    }
}

spla::Algorithm::Type spla::MatrixReduceScalarCOO::GetType() const {
    return spla::Algorithm::Type::MatrixReduceScalar;
}

std::string spla::MatrixReduceScalarCOO::GetName() const {
    return "ReduceScalarCOO";
}
