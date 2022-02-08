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
#include <spla-cpp/SplaExpression.hpp>


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

void spla::Expression::Submit() {
    CHECK_RAISE_ERROR(GetState() == State::Default, InvalidState, "Expression must be in default state before submission");
    GetLibrary().GetPrivate().GetExprManager()->Submit(this);
}

void spla::Expression::Wait() {
    CHECK_RAISE_ERROR(GetState() != State::Default, InvalidState, "Expression must be submitted for the execution");

    // NOTE: Wait only if submitted
    if (GetState() == State::Submitted)
        mFuture->Get().wait();
}

void spla::Expression::SubmitWait() {
    Submit();
    Wait();
}

void spla::Expression::Dependency(const spla::RefPtr<spla::ExpressionNode> &pred,
                                  const spla::RefPtr<spla::ExpressionNode> &succ) {
    CHECK_RAISE_ERROR(GetState() == State::Default, InvalidState, "Expression must be in default state");

    CHECK_RAISE_ERROR(pred.IsNotNull(), NullPointer, "Passed null arg");
    CHECK_RAISE_ERROR(succ.IsNotNull(), NullPointer, "Passed null arg");

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
    CHECK_RAISE_ERROR(matrix.IsNotNull(), NullPointer, "matrix can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");

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
    CHECK_RAISE_ERROR(vector.IsNotNull(), NullPointer, "vector can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");

    std::vector<RefPtr<Object>> args = {
            vector.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorDataWrite,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataWrite(const RefPtr<Scalar> &scalar,
                                const RefPtr<DataScalar> &data,
                                const RefPtr<Descriptor> &desc) {
    CHECK_RAISE_ERROR(scalar.IsNotNull(), NullPointer, "scalar can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");
    CHECK_RAISE_ERROR(scalar->GetType()->HasValues(), InvalidState, "can't write type with no value");

    std::vector<RefPtr<Object>> args = {
            scalar.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::ScalarDataWrite,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataRead(const spla::RefPtr<spla::Matrix> &matrix,
                               const spla::RefPtr<spla::DataMatrix> &data,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(matrix.IsNotNull(), NullPointer, "matrix can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");

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
    CHECK_RAISE_ERROR(vector.IsNotNull(), NullPointer, "vector can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");

    std::vector<RefPtr<Object>> args = {
            vector.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorDataRead,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeDataRead(const RefPtr<Scalar> &scalar,
                               const RefPtr<DataScalar> &data,
                               const RefPtr<Descriptor> &desc) {
    CHECK_RAISE_ERROR(scalar.IsNotNull(), NullPointer, "scalar can't be null");
    CHECK_RAISE_ERROR(data.IsNotNull(), NullPointer, "data can't be null");
    CHECK_RAISE_ERROR(scalar->GetType()->HasValues(), InvalidState, "can't read type with no value");

    std::vector<RefPtr<Object>> args = {
            scalar.As<Object>(),
            data.As<Object>()};

    return MakeNode(ExpressionNode::Operation::ScalarDataRead,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeToDense(const RefPtr<Vector> &w,
                              const RefPtr<Vector> &v,
                              const RefPtr<Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(v.IsNotNull(), NullPointer, "v can't be null");
    CHECK_RAISE_ERROR(w->GetType() == v->GetType(), InvalidType, "Type must be compatible");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            v.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorToDense,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeAssign(const spla::RefPtr<spla::Vector> &w,
                             const spla::RefPtr<spla::Vector> &mask,
                             const spla::RefPtr<FunctionBinary> &accum,
                             const spla::RefPtr<spla::Scalar> &s,
                             const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(s.IsNull() || w->IsCompatible(*s), InvalidType, "s and w must have the same type");
    CHECK_RAISE_ERROR(s.IsNotNull() || !w->GetType()->HasValues(), InvalidState, "s can be null only if w has no values");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetNrows() == mask->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(accum.IsNull() || (s.IsNotNull() && accum->CanApply(*w, *s, *w)), InvalidType, "cannot apply provided accum function");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            accum.As<Object>(),
            s.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorAssign,
                    std::move(args),
                    desc);
}


spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeEWiseAdd(const spla::RefPtr<spla::Matrix> &w,
                               const spla::RefPtr<spla::Matrix> &mask,
                               const spla::RefPtr<spla::FunctionBinary> &op,
                               const spla::RefPtr<spla::Matrix> &a,
                               const spla::RefPtr<spla::Matrix> &b,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(b.IsNotNull(), NullPointer, "b can't be null");
    CHECK_RAISE_ERROR(w->IsCompatible(*a) && w->IsCompatible(*b), InvalidType, "w, a, b must have the same type");
    CHECK_RAISE_ERROR(!w->GetType()->HasValues() || op.IsNotNull(), NullPointer, "If type has values then `op` must be provided");
    CHECK_RAISE_ERROR(w->GetType()->HasValues() || op.IsNull(), InvalidArgument, "If type has no values then `op` must be null");
    CHECK_RAISE_ERROR(op.IsNull() || op->CanApply(*a, *b, *w), InvalidType, "Can't apply provided op");
    CHECK_RAISE_ERROR(w->GetDim() == a->GetDim(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(w->GetDim() == b->GetDim(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetDim() == mask->GetDim(), DimensionMismatch, "Incompatible size");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            op.As<Object>(),
            a.As<Object>(),
            b.As<Object>()};

    return MakeNode(ExpressionNode::Operation::MatrixEWiseAdd,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeEWiseAdd(const spla::RefPtr<spla::Vector> &w,
                               const spla::RefPtr<spla::Vector> &mask,
                               const spla::RefPtr<spla::FunctionBinary> &op,
                               const spla::RefPtr<spla::Vector> &a,
                               const spla::RefPtr<spla::Vector> &b,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(b.IsNotNull(), NullPointer, "b can't be null");
    CHECK_RAISE_ERROR(w->IsCompatible(*a) && w->IsCompatible(*b), InvalidType, "w, a, b must have the same type");
    CHECK_RAISE_ERROR(!w->GetType()->HasValues() || op.IsNotNull(), NullPointer, "If type has values then `op` must be provided");
    CHECK_RAISE_ERROR(w->GetType()->HasValues() || op.IsNull(), InvalidArgument, "If type has no values then `op` must be null");
    CHECK_RAISE_ERROR(op.IsNull() || op->CanApply(*a, *b, *w), InvalidType, "Can't apply provided op");
    CHECK_RAISE_ERROR(w->GetNrows() == a->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(w->GetNrows() == b->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetNrows() == mask->GetNrows(), DimensionMismatch, "Incompatible size");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            op.As<Object>(),
            a.As<Object>(),
            b.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorEWiseAdd,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeReduce(const spla::RefPtr<spla::Scalar> &s,
                             const spla::RefPtr<spla::FunctionBinary> &op,
                             const spla::RefPtr<spla::Vector> &v,
                             const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(s.IsNotNull(), NullPointer, "s op can't be null");
    CHECK_RAISE_ERROR(op.IsNotNull(), NullPointer, "op can't be null");
    CHECK_RAISE_ERROR(v.IsNotNull(), NullPointer, "v can't be null");
    CHECK_RAISE_ERROR(op->CanApply(*v, *v, *s), InvalidType, "Can't apply reduce op to provided objects");

    std::vector<RefPtr<Object>> args = {
            s.As<Object>(),
            op.As<Object>(),
            v.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VectorReduce,
                    std::move(args),
                    desc);
}


spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeReduceScalar(const spla::RefPtr<spla::Scalar> &w,
                                   const spla::RefPtr<spla::Matrix> &mask,
                                   const spla::RefPtr<spla::FunctionBinary> &accum,
                                   const spla::RefPtr<spla::FunctionBinary> &op,
                                   const spla::RefPtr<spla::Matrix> &a,
                                   const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "scalar w can't be null");
    CHECK_RAISE_ERROR(op.IsNotNull(), NullPointer, "reduce op can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "matrix a can't be null");
    CHECK_RAISE_ERROR(op->CanApply(*a, *a, *w), InvalidType, "Can't apply op to provided objects");
    if (accum.IsNotNull()) {
        CHECK_RAISE_ERROR(accum->CanApply(*w, *w, *w), InvalidType, "Can't apply accum to provided objects");
    }
    if (mask.IsNotNull()) {
        CHECK_RAISE_ERROR(mask->GetDim() == a->GetDim(), DimensionMismatch, "Mask has incompatible size");
    }

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            accum.As<Object>(),
            op.As<Object>(),
            a.As<Object>()};

    return MakeNode(ExpressionNode::Operation::MatrixReduceScalar,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeEWiseAdd(const spla::RefPtr<spla::Scalar> &w,
                               const spla::RefPtr<spla::FunctionBinary> &op,
                               const spla::RefPtr<spla::Scalar> &a,
                               const spla::RefPtr<spla::Scalar> &b,
                               const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "Scalar w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "Scalar a can't be null");
    CHECK_RAISE_ERROR(b.IsNotNull(), NullPointer, "Scalar b can't be null");
    CHECK_RAISE_ERROR(op.IsNotNull(), NullPointer, "Function op can't be null");
    CHECK_RAISE_ERROR(op->CanApply(*a, *b, *w), InvalidType, "Can't apply op ot provided objects");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            op.As<Object>(),
            a.As<Object>(),
            b.As<Object>()};

    return MakeNode(ExpressionNode::Operation::ScalarEWiseAdd,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeMxM(const spla::RefPtr<spla::Matrix> &w,
                          const spla::RefPtr<spla::Matrix> &mask,
                          const spla::RefPtr<spla::FunctionBinary> &mult,
                          const spla::RefPtr<spla::FunctionBinary> &add,
                          const spla::RefPtr<spla::Matrix> &a,
                          const spla::RefPtr<spla::Matrix> &b,
                          const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(b.IsNotNull(), NullPointer, "b can't be null");
    CHECK_RAISE_ERROR(w->GetNrows() == a->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(w->GetNcols() == b->GetNcols(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(a->GetNcols() == b->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetDim() == mask->GetDim(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mult.IsNull() || mult->CanApply(*a, *b, *w), InvalidType, "Cannot apply `mult` op to provided objects");
    CHECK_RAISE_ERROR(add.IsNull() || add->CanApply(*w, *w, *w), InvalidType, "Cannot apply `add` op to provided objects");
    CHECK_RAISE_ERROR(!w->GetType()->HasValues() || (mult.IsNotNull() && add.IsNotNull()), NullPointer, "If type has values, `mult` and `add` op must be provided");
    CHECK_RAISE_ERROR(w->GetType()->HasValues() || (mult.IsNull() && add.IsNull()), InvalidArgument, "If type has no values then `mult` and `add` must be null");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            mult.As<Object>(),
            add.As<Object>(),
            a.As<Object>(),
            b.As<Object>()};

    return MakeNode(ExpressionNode::Operation::MxM,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeVxM(const spla::RefPtr<spla::Vector> &w,
                          const spla::RefPtr<spla::Vector> &mask,
                          const spla::RefPtr<spla::FunctionBinary> &mult,
                          const spla::RefPtr<spla::FunctionBinary> &add,
                          const spla::RefPtr<spla::Vector> &a,
                          const spla::RefPtr<spla::Matrix> &b,
                          const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(b.IsNotNull(), NullPointer, "b can't be null");
    CHECK_RAISE_ERROR(w->GetNrows() == b->GetNcols(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(a->GetNrows() == b->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetNrows() == mask->GetNrows(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mult.IsNull() || mult->CanApply(*a, *b, *w), InvalidType, "Cannot apply `mult` op to provided objects");
    CHECK_RAISE_ERROR(add.IsNull() || add->CanApply(*w, *w, *w), InvalidType, "Cannot apply `add` op to provided objects");
    CHECK_RAISE_ERROR(!w->GetType()->HasValues() || (mult.IsNotNull() && add.IsNotNull()), NullPointer, "If type has values, `mult` and `add` op must be provided");
    CHECK_RAISE_ERROR(w->GetType()->HasValues() || (mult.IsNull() && add.IsNull()), InvalidArgument, "If type has no values then `mult` and `add` must be null");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            mult.As<Object>(),
            add.As<Object>(),
            a.As<Object>(),
            b.As<Object>()};

    return MakeNode(ExpressionNode::Operation::VxM,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeTranspose(const spla::RefPtr<spla::Matrix> &w,
                                const spla::RefPtr<spla::Matrix> &mask,
                                const spla::RefPtr<spla::FunctionBinary> &accum,
                                const spla::RefPtr<spla::Matrix> &a,
                                const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(w->IsCompatible(*a), InvalidType, "w and a must have the same type");
    CHECK_RAISE_ERROR(w->GetNrows() == a->GetNcols(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(a->GetNrows() == w->GetNcols(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(mask.IsNull() || w->GetDim() == mask->GetDim(), DimensionMismatch, "Incompatible size");
    CHECK_RAISE_ERROR(accum.IsNull() || accum->CanApply(*w, *a, *w), InvalidType, "Cannot apply `add` op to provided objects");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            mask.As<Object>(),
            accum.As<Object>(),
            a.As<Object>()};

    return MakeNode(ExpressionNode::Operation::Transpose,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeTril(const spla::RefPtr<spla::Matrix> &w,
                           const spla::RefPtr<spla::Matrix> &a,
                           const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(w->IsCompatible(*a), InvalidType, "w and a must have the same type");
    CHECK_RAISE_ERROR(w->GetDim() == a->GetDim(), DimensionMismatch, "Incompatible size");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            a.As<Object>()};

    return MakeNode(ExpressionNode::Operation::Tril,
                    std::move(args),
                    desc);
}

spla::RefPtr<spla::ExpressionNode>
spla::Expression::MakeTriu(const spla::RefPtr<spla::Matrix> &w,
                           const spla::RefPtr<spla::Matrix> &a,
                           const spla::RefPtr<spla::Descriptor> &desc) {
    CHECK_RAISE_ERROR(w.IsNotNull(), NullPointer, "w can't be null");
    CHECK_RAISE_ERROR(a.IsNotNull(), NullPointer, "a can't be null");
    CHECK_RAISE_ERROR(w->IsCompatible(*a), InvalidType, "w and a must have the same type");
    CHECK_RAISE_ERROR(w->GetDim() == a->GetDim(), DimensionMismatch, "Incompatible size");

    std::vector<RefPtr<Object>> args = {
            w.As<Object>(),
            a.As<Object>()};

    return MakeNode(ExpressionNode::Operation::Triu,
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