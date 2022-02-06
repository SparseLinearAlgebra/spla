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
#include <expression/matrix/SplaMatrixTria.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/SplaVectorStorage.hpp>

bool spla::MatrixTria::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::MatrixTria::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();

    auto w = node->GetArg(0).Cast<Matrix>();
    auto a = node->GetArg(1).Cast<Matrix>();
    auto desc = node->GetDescriptor();

    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(desc.IsNotNull());

    w->GetStorage()->Clear();
    auto blockSize = a->GetStorage()->GetBlockSize();

    std::size_t requiredDeviceCount = w->GetStorage()->GetNblockRows() * w->GetStorage()->GetNblockCols();
    auto deviceIds = library->GetDeviceManager().FetchDevices(requiredDeviceCount, node);

    for (std::size_t i = 0; i < w->GetStorage()->GetNblockRows(); i++) {
        for (std::size_t j = 0; j < w->GetStorage()->GetNblockCols(); j++) {
            auto deviceId = deviceIds[i * w->GetStorage()->GetNblockCols() + j];
            builder.Emplace("tria", [=]() {
                MatrixStorage::Index index{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};

                ParamsTria params;
                params.desc = desc;
                params.deviceId = deviceId;
                params.firstI = blockSize * i;
                params.firstJ = blockSize * j;
                params.a = a->GetStorage()->GetBlock(index);
                params.type = w->GetType();
                library->GetAlgoManager()->Dispatch(Algorithm::Type::Tria, params);

                if (params.w.IsNotNull()) {
                    w->GetStorage()->SetBlock(index, params.w);
                    SPDLOG_LOGGER_TRACE(library->GetLogger(), "Tria block=({},{})", index.first, index.second);
                }
            });
        }
    }
}

spla::ExpressionNode::Operation spla::MatrixTria::GetOperationType() const {
    return spla::ExpressionNode::Operation::Tria;
}
