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

#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <expression/SplaExpressionFuture.hpp>
#include <expression/SplaExpressionTasks.hpp>

spla::Expression::~Expression() {
    // Before destruction we must wait until it is executed
    // NOTE: if was not submitted, nothing to do
    if (GetState() != State::Default)
        Wait();

    mFuture.reset();
    mTasks.reset();
}

spla::RefPtr<spla::Expression> spla::Expression::Make(spla::Library &library) {
    return spla::RefPtr<spla::Expression>(new Expression(library));
}

spla::Expression::Expression(spla::Library &library) : Object(Object::TypeName::Expression, library) {
    SetState(State::Default);
}

void spla::Expression::Wait() {
    CHECK_RAISE_ERROR(GetState() != State::Default, InvalidState, "Expression must be submitted for the execution");

    // NOTE: Wait only if submitted
    if (GetState() == State::Submitted)
        mFuture->Get().wait();
}

void spla::Expression::Dependency(const spla::RefPtr<spla::ExpressionNode> &pred,
                                  const spla::RefPtr<spla::ExpressionNode> &succ) {
    CHECK_RAISE_ERROR(GetState() == State::Default, InvalidState, "Expression must be in default state");

    CHECK_RAISE_ERROR(pred.IsNotNull(), InvalidArgument, "Passed null arg");
    CHECK_RAISE_ERROR(succ.IsNotNull(), InvalidArgument, "Passed null arg");

    CHECK_RAISE_ERROR(pred->Belongs(*this), InvalidArgument, "Node must be part of expression");
    CHECK_RAISE_ERROR(succ->Belongs(*this), InvalidArgument, "Node must be part of expression");

    // NOTE: Cycles check is done later
    pred->Link(succ.Get());
}

spla::Expression::State spla::Expression::GetState() const {
    return static_cast<State>(mState.load());
}

bool spla::Expression::Empty() const {
    return mNodes.empty();
}

const std::vector<spla::RefPtr<spla::ExpressionNode>> &spla::Expression::GetNodes() const {
    return mNodes;
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeNode(spla::ExpressionNode::Operation op,
                           std::vector<RefPtr<Object>> &&args,
                           const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(GetState() == State::Default, InvalidState, "Expression must be in default state");

    for (const auto &arg : args)
        CHECK_RAISE_ERROR(arg.IsNotNull(), InvalidArgument, "Passed null argument to op=" << ExpressionNodeOpToStr(op));

    RefPtr<ExpressionNode> node(new ExpressionNode(op, *this, GetLibrary()));
    node->SetIdx(mNodes.size());
    node->SetArgs(std::move(args));
    node->SetDescriptor(desc.IsNull() ? GetLibrary().GetPrivate().GetDefaultDesc() : desc);
    mNodes.push_back(node);
    return node;
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataWrite(const spla::RefPtr<spla::Matrix> &matrix,
                                const spla::RefPtr<spla::DataMatrix> &data,
                                const spla::RefPtr<spla::Descriptor> &desc) {
    std::vector<RefPtr<Object>> args = {
            matrix.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::MatrixDataWrite,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataWrite(const spla::RefPtr<spla::Vector> &vector,
                                const spla::RefPtr<spla::DataVector> &data,
                                const spla::RefPtr<spla::Descriptor> &desc) {
    std::vector<RefPtr<Object>> args = {
            vector.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorDataWrite,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataRead(const spla::RefPtr<spla::Matrix> &matrix,
                               const spla::RefPtr<spla::DataMatrix> &data,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    std::vector<RefPtr<Object>> args = {
            matrix.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::MatrixDataRead,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataRead(const spla::RefPtr<spla::Vector> &vector,
                               const spla::RefPtr<spla::DataVector> &data,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    std::vector<RefPtr<Object>> args = {
            vector.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorDataRead,
                    std::move(args),
                    desc);
}

void spla::Expression::SetState(State state) {
    mState.store(state);
}

void spla::Expression::SetFuture(std::unique_ptr<class ExpressionFuture> &&future) {
    mFuture = std::move(future);
}

void spla::Expression::SetTasks(std::unique_ptr<class ExpressionTasks> &&tasks) {
    mTasks = std::move(tasks);
}
