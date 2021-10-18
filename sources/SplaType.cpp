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
#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaType.hpp>

spla::RefPtr<spla::Type> spla::Type::Make(std::string id, size_t typeSize, spla::Library &library) {
    return Make(std::move(id), typeSize, false, library);
}

spla::RefPtr<spla::Type> spla::Type::Make(std::string id, size_t typeSize, bool builtIn, spla::Library &library) {
    auto &libraryPrivate = library.GetPrivate();
    auto &typeCache = libraryPrivate.GetTypeCache();

    auto entry = typeCache.find(id);
    CHECK_RAISE_ERROR(entry == typeCache.end(), Error, "An attempt to create new type with the same id");

    RefPtr<Type> type{new Type(std::move(id), typeSize, builtIn, library)};
    typeCache.emplace(type->GetId(), type);

    return type;
}

spla::RefPtr<spla::Type> spla::Type::Find(std::string id, spla::Library &library) {
    auto &libraryPrivate = library.GetPrivate();
    auto &typeCache = libraryPrivate.GetTypeCache();

    auto entry = typeCache.find(id);
    return entry != typeCache.end() ? entry->second : nullptr;
}

spla::Type::Type(std::string id,
                 size_t typeSize,
                 bool builtIn,
                 spla::Library &library)
    : Object(Object::TypeName::Type, library),
      mId(std::move(id)),
      mByteSize(typeSize),
      mBuiltIn(builtIn) {
}

spla::TypedObject::TypedObject(spla::RefPtr<spla::Type> type, spla::Object::TypeName typeName, spla::Library &library)
    : Object(typeName, library), mType(std::move(type)) {
}
