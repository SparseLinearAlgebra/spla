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
#include <expression/prod/SplaMxM.hpp>
#include <storage/SplaMatrixStorage.hpp>

bool spla::MxM::Select(std::size_t nodeIdx, const spla::Expression &expression) {
    return true;
}

void spla::MxM::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto w = node->GetArg(0).Cast<Matrix>();
    auto mask = node->GetArg(1).Cast<Matrix>();
    auto mult = node->GetArg(2).Cast<Matrix>();
    auto add = node->GetArg(3).Cast<Matrix>();
    auto a = node->GetArg(4).Cast<Matrix>();
    auto b = node->GetArg(5).Cast<Matrix>();
    auto desc = node->GetDescriptor();

    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(b.IsNotNull());
    assert(desc.IsNotNull());

    std::size_t nBlockRows, nBlockCols;
    w->GetStorage()->GetBlocksGrid(nBlockRows, nBlockCols);
}

spla::ExpressionNode::Operation spla::MxM::GetOperationType() const {
    return ExpressionNode::Operation::MxM;
}
