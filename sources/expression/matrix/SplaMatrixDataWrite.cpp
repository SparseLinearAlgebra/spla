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

#include <detail/SplaLibraryPrivate.hpp>
#include <expression/matrix/SplaMatrixDataWrite.hpp>

bool spla::MatrixDataWrite::Select(size_t nodeIdx, spla::ExpressionContext &context) {
    return true;
}

void spla::MatrixDataWrite::Process(size_t nodeIdx, spla::ExpressionContext &context) {
    // todo: impl me!
    auto &taskflow = context.nodesTaskflow[nodeIdx];
    auto logger = context.expression->GetLibrary().GetPrivate().GetLogger();

    // todo: remove it!
    taskflow.emplace([=]() {
        SPDLOG_LOGGER_TRACE(logger, "Process node {}", nodeIdx);
    });
}

spla::ExpressionNode::Operation spla::MatrixDataWrite::GetOperationType() const {
    return ExpressionNode::Operation::MatrixDataWrite;
}
