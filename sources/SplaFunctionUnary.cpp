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

#include <spla-cpp/SplaFunctionUnary.hpp>

const spla::RefPtr<spla::Type> &spla::FunctionUnary::GetA() const {
    return mA;
}

const spla::RefPtr<spla::Type> &spla::FunctionUnary::GetB() const {
    return mB;
}

const std::string &spla::FunctionUnary::GetSource() const {
    return mSource;
}

bool spla::FunctionUnary::CanApply(const spla::TypedObject &a, const spla::TypedObject &b) const {
    return a.GetType() == GetA() &&
           b.GetType() == GetB();
}

spla::RefPtr<spla::FunctionUnary> spla::FunctionUnary::Make(spla::RefPtr<spla::Type> a, spla::RefPtr<spla::Type> b, std::string source, spla::Library &library) {
    return RefPtr<FunctionUnary>(new FunctionUnary(std::move(a), std::move(b), std::move(source), library));
}

spla::FunctionUnary::FunctionUnary(spla::RefPtr<spla::Type> a, spla::RefPtr<spla::Type> b, std::string source, spla::Library &library)
        : Object(TypeName::FunctionUnary, library), mA(std::move(a)), mB(std::move(b)), mSource(std::move(source)) {
}