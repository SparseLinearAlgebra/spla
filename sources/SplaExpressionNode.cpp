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

#include <spla-cpp/SplaExpressionNode.hpp>

const std::vector<spla::RefPtr<spla::Object>> &spla::ExpressionNode::GetArgs() const {
    return mArgs;
}

const spla::RefPtr<spla::Object> &spla::ExpressionNode::GetArg(unsigned int idx) const {
    assert(idx < mArgs.size());
    return mArgs[idx];
}

const spla::RefPtr<spla::Descriptor> &spla::ExpressionNode::GetDescriptor() const {
    return mDescriptor;
}

spla::ExpressionNode::Operation spla::ExpressionNode::GetNodeOp() const {
    return mNodeOp;
}

std::size_t spla::ExpressionNode::GetIdx() const {
    return mIdx;
}

spla::ExpressionNode::ExpressionNode(spla::ExpressionNode::Operation operation, spla::Expression &expression, spla::Library &library)
    : Object(Object::TypeName::ExpressionNode, library),
      mNodeOp(operation),
      mParent(expression) {
}

void spla::ExpressionNode::Link(spla::ExpressionNode *next) {
    next->mPrev.push_back(this);
    mNext.push_back(next);
}

void spla::ExpressionNode::SetIdx(std::size_t idx) {
    mIdx = idx;
}

bool spla::ExpressionNode::Belongs(class Expression &expression) const {
    return &expression == &mParent;
}

void spla::ExpressionNode::SetArgs(std::vector<RefPtr<Object>> &&args) {
    mArgs = std::move(args);
}

void spla::ExpressionNode::SetDescriptor(const spla::RefPtr<spla::Descriptor> &desc) {
    mDescriptor = desc;
}

std::vector<spla::RefPtr<spla::Object>> &spla::ExpressionNode::GetArgs() {
    return mArgs;
}

const std::vector<spla::ExpressionNode *> &spla::ExpressionNode::GetPrev() const {
    return mPrev;
}

const std::vector<spla::ExpressionNode *> &spla::ExpressionNode::GetNext() const {
    return mNext;
}