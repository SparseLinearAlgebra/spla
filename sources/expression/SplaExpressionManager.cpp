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

#include <expression/SplaExpressionManager.hpp>
#include <detail/SplaError.hpp>

spla::ExpressionManager::ExpressionManager(spla::Library &library) : mLibrary(library) {

}

void spla::ExpressionManager::Submit(const spla::RefPtr<spla::Expression> &expression) {

}

void spla::ExpressionManager::Register(const spla::RefPtr<spla::NodeProcessor> &processor) {
    CHECK_RAISE_ERROR(processor.IsNotNull(), InvalidArgument, L"Passed null processor");

    ExpressionNode::Operation op = processor->GetOperationType();
    auto list = mProcessors.find(op);

    if (list == mProcessors.end())
        list = mProcessors.emplace(op, ProcessorList()).first;

    list->second.push_back(processor);
}

spla::RefPtr<spla::NodeProcessor>
spla::ExpressionManager::SelectProcessor(const spla::RefPtr<spla::ExpressionNode> &node) {
    return spla::RefPtr<spla::NodeProcessor>();
}
