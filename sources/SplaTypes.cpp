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

#include <spla-cpp/SplaTypes.hpp>

namespace spla {
    static const char *NAME_VOID = "__spla_void";
    static const char *NAME_BOOL = "__spla_bool";
    static const char *NAME_INT8 = "__spla_int8";
    static const char *NAME_INT16 = "__spla_int16";
    static const char *NAME_INT32 = "__spla_int32";
    static const char *NAME_INT64 = "__spla_int64";
    static const char *NAME_UINT8 = "__spla_uint8";
    static const char *NAME_UINT16 = "__spla_uint16";
    static const char *NAME_UINT32 = "__spla_uint32";
    static const char *NAME_UINT64 = "__spla_uint64";
    static const char *NAME_FLOAT32 = "__spla_float32";
    static const char *NAME_FLOAT64 = "__spla_float64";
}// namespace spla

spla::RefPtr<spla::Type> spla::Types::Void(Library &library) {
    return Type::Find(NAME_VOID, library);
}

spla::RefPtr<spla::Type> spla::Types::Bool(Library &library) {
    return Type::Find(NAME_BOOL, library);
}

spla::RefPtr<spla::Type> spla::Types::Int8(Library &library) {
    return Type::Find(NAME_INT8, library);
}

spla::RefPtr<spla::Type> spla::Types::Int16(Library &library) {
    return Type::Find(NAME_INT16, library);
}

spla::RefPtr<spla::Type> spla::Types::Int32(Library &library) {
    return Type::Find(NAME_INT32, library);
}

spla::RefPtr<spla::Type> spla::Types::Int64(Library &library) {
    return Type::Find(NAME_INT64, library);
}

spla::RefPtr<spla::Type> spla::Types::UInt8(Library &library) {
    return Type::Find(NAME_UINT8, library);
}

spla::RefPtr<spla::Type> spla::Types::UInt16(Library &library) {
    return Type::Find(NAME_UINT16, library);
}

spla::RefPtr<spla::Type> spla::Types::UInt32(Library &library) {
    return Type::Find(NAME_UINT32, library);
}

spla::RefPtr<spla::Type> spla::Types::UInt64(Library &library) {
    return Type::Find(NAME_UINT64, library);
}

spla::RefPtr<spla::Type> spla::Types::Float32(Library &library) {
    return Type::Find(NAME_FLOAT32, library);
}

spla::RefPtr<spla::Type> spla::Types::Float64(Library &library) {
    return Type::Find(NAME_FLOAT64, library);
}

void spla::Types::RegisterTypes(Library &library) {
    Type::Make(NAME_VOID, 0, true, library);
    Type::Make(NAME_BOOL, sizeof(bool), true, library);
    Type::Make(NAME_INT8, sizeof(std::int8_t), true, library);
    Type::Make(NAME_INT16, sizeof(std::int16_t), true, library);
    Type::Make(NAME_INT32, sizeof(std::int32_t), true, library);
    Type::Make(NAME_INT64, sizeof(std::int64_t), true, library);
    Type::Make(NAME_UINT8, sizeof(std::uint8_t), true, library);
    Type::Make(NAME_UINT16, sizeof(std::uint16_t), true, library);
    Type::Make(NAME_UINT32, sizeof(std::uint32_t), true, library);
    Type::Make(NAME_UINT64, sizeof(std::uint64_t), true, library);
    Type::Make(NAME_FLOAT32, sizeof(float), true, library);
    Type::Make(NAME_FLOAT64, sizeof(double), true, library);
}