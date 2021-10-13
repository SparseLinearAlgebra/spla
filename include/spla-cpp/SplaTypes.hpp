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

#ifndef SPLA_SPLATYPES_HPP
#define SPLA_SPLATYPES_HPP

#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaType.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class SplaTypes
     * @brief Provider of built-in library types.
     * Allows to access built-in int, float, bool types by Library instance.
     */
    class SPLA_API Types {
    public:
        /**
         * Get built-in library Void type.
         * @note Void type has 0 size (no actual values).
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Void(Library &library);

        /**
         * Get built-in library Bool type.
         * @note C++ `bool` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Bool(Library &library);

        /**
         * Get built-in library Int8 type.
         * @note C++ `int8_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Int8(Library &library);

        /**
         * Get built-in library Int16 type.
         * @note C++ `int16_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Int16(Library &library);

        /**
         * Get built-in library Int32 type.
         * @note C++ `int32_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Int32(Library &library);

        /**
         * Get built-in library Int64 type.
         * @note C++ `int64_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Int64(Library &library);

        /**
         * Get built-in library UInt8 type.
         * @note C++ `uint8_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> UInt8(Library &library);

        /**
         * Get built-in library UInt16 type.
         * @note C++ `uint16_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> UInt16(Library &library);

        /**
         * Get built-in library UInt32 type.
         * @note C++ `uint32_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> UInt32(Library &library);

        /**
         * Get built-in library UInt64 type.
         * @note C++ `uint64_t` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> UInt64(Library &library);

        /**
         * Get built-in library Float32 type.
         * @note C++ `float` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Float32(Library &library);

        /**
         * Get built-in library Float64 type.
         * @note C++ `double` type.
         *
         * @param library Library global instance
         * @return Built-in type instance
         */
        static RefPtr<Type> Float64(Library &library);

    private:
        friend class Library;
        static void RegisterTypes(Library &library);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLATYPES_HPP