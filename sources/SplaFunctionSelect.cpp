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

#include <spla-cpp/SplaFunctionSelect.hpp>

const spla::RefPtr<spla::Type> &spla::FunctionSelect::GetA() const {
    return mA;
}

const std::string &spla::FunctionSelect::GetSource() const {
    return mSource;
}

bool spla::FunctionSelect::CanApply(const spla::TypedObject &a) const {
    return a.GetType() == GetA();
}

spla::RefPtr<spla::FunctionSelect> spla::FunctionSelect::Make(spla::RefPtr<spla::Type> a, std::string source, spla::Library &library) {
    return RefPtr<FunctionSelect>(new FunctionSelect(std::move(a), std::move(source), library));
}

spla::FunctionSelect::FunctionSelect(spla::RefPtr<spla::Type> a, std::string source, spla::Library &library)
    : Object(TypeName::FunctionSelect, library), mA(std::move(a)), mSource(std::move(source)) {
}
