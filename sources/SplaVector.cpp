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

#include <spla-cpp/SplaVector.hpp>
#include <storage/SplaVectorStorage.hpp>

std::size_t spla::Vector::GetNrows() const {
    return mStorage->GetNrows();
}

std::size_t spla::Vector::GetNvals() const {
    return mStorage->GetNvals();
}

spla::RefPtr<spla::Vector> spla::Vector::Make(std::size_t nrows,
                                              const RefPtr<Type> &type,
                                              spla::Library &library) {
    return {new Vector(nrows, type, library)};
}

spla::Vector::Vector(std::size_t nrows,
                     const RefPtr<Type> &type,
                     spla::Library &library) : TypedObject(type, Object::TypeName::Vector, library) {
    mStorage = VectorStorage::Make(nrows, GetLibrary());
}

const spla::RefPtr<spla::VectorStorage> &spla::Vector::GetStorage() const {
    return mStorage;
}

void spla::Vector::Dump(std::ostream &stream) const {
    mStorage->Dump(stream);
}

spla::Vector::~Vector() = default;