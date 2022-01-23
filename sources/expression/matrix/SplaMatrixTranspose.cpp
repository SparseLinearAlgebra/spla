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

#include <algo/SplaAlgorithmManager.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <expression/matrix/SplaMatrixTranspose.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/SplaVectorStorage.hpp>
#include <utils/SplaAlgo.hpp>


bool spla::MatrixTranspose::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::MatrixTranspose::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto w = node->GetArg(0).Cast<Matrix>();
    auto mask = node->GetArg(1).Cast<Matrix>();
    auto accum = node->GetArg(2).Cast<FunctionBinary>();
    auto a = node->GetArg(3).Cast<Matrix>();
    auto desc = node->GetDescriptor();

    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(desc.IsNotNull());

    /** Handle case if new to accum(w, s) */
    auto tmp = w;
    auto applyAccum = desc->IsParamSet(Descriptor::Param::AccumResult);
    // Create temporary vector for assignment result
    if (applyAccum) tmp = Matrix::Make(w->GetNrows(), w->GetNcols(), w->GetType(), w->GetLibrary());
    // If no accum, clear result first
    if (!applyAccum) w->GetStorage()->Clear();
    // If assign is null, make default to keep new entries
    if (applyAccum && accum.IsNull()) accum = utils::MakeFunctionChooseSecond(w->GetType());

    std::size_t requiredDeviceCount = w->GetStorage()->GetNblockRows() * w->GetStorage()->GetNblockCols();
    auto deviceIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < w->GetStorage()->GetNblockRows(); i++) {
        for (std::size_t j = 0; j < w->GetStorage()->GetNblockCols(); j++) {
            auto deviceId = deviceIds[i * w->GetStorage()->GetNblockCols() + j];
            auto taskTranspose = builder.Emplace([=]() {
                MatrixStorage::Index aIndex{static_cast<unsigned int>(j), static_cast<unsigned int>(i)};
                MatrixStorage::Index wIndex{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};

                ParamsTranspose params;
                params.desc = desc;
                params.deviceId = deviceId;
                params.hasMask = mask.IsNotNull();
                params.mask = mask.IsNotNull() ? mask->GetStorage()->GetBlock(wIndex) : RefPtr<MatrixBlock>{};
                params.a = a->GetStorage()->GetBlock(aIndex);
                params.type = w->GetType();
                library->GetAlgoManager()->Dispatch(Algorithm::Type::Transpose, params);

                if (params.w.IsNotNull()) {
                    tmp->GetStorage()->SetBlock(wIndex, params.w);
                    SPDLOG_LOGGER_TRACE(logger, "Transpose block=({},{})",
                                        aIndex.first, aIndex.second);
                }
            });

            if (applyAccum) {
                auto taskAccum = builder.Emplace([=]() {
                    MatrixStorage::Index wIndex{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};

                    ParamsMatrixEWiseAdd params;
                    params.desc = desc;
                    params.deviceId = deviceId;
                    params.a = w->GetStorage()->GetBlock(wIndex);
                    params.b = tmp->GetStorage()->GetBlock(wIndex);
                    params.op = accum;
                    params.type = w->GetType();
                    library->GetAlgoManager()->Dispatch(Algorithm::Type::MatrixEWiseAdd, params);

                    if (params.w.IsNotNull()) {
                        w->GetStorage()->SetBlock(wIndex, params.w);
                        SPDLOG_LOGGER_TRACE(logger, "Accum block=({},{}) nnz={}",
                                            wIndex.first, wIndex.second, params.w->GetNvals());
                    }
                });

                // Transpose matrix and then accum results
                taskTranspose.precede(taskAccum);
            }
        }
    }
}

spla::ExpressionNode::Operation spla::MatrixTranspose::GetOperationType() const {
    return spla::ExpressionNode::Operation::Transpose;
}
