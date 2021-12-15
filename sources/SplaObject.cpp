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
#include <spla-cpp/SplaObject.hpp>

spla::Object::Object(spla::Object::TypeName typeName, class Library &library)
    : mTypeName(typeName), mLibrary(library) {
}

const std::string &spla::Object::GetLabel() const {
    return mLabel;
}

spla::Object::TypeName spla::Object::GetTypeName() const {
    return mTypeName;
}

spla::Library &spla::Object::GetLibrary() const {
    return mLibrary;
}

spla::RefPtr<spla::Object> spla::Object::Clone() const {
    assert(false && "Object::Clone() is not implemented");
    RAISE_ERROR(NotImplemented, "Object::Clone() is not implemented");
}

spla::RefPtr<spla::Object> spla::Object::CloneEmpty() {
    assert(false && "Object::CloneEmpty() is not implemented");
    RAISE_ERROR(NotImplemented, "Object::CloneEmpty() is not implemented");
}

void spla::Object::CopyData(const RefPtr<Object> &) {
    assert(false && "Object::CopyData() is not implemented");
    RAISE_ERROR(NotImplemented, "Object::CopyData() is not implemented");
}