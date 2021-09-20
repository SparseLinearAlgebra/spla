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

#ifndef SPLA_SPLATYPE_HPP
#define SPLA_SPLATYPE_HPP

#include <spla-cpp/SplaObject.hpp>

namespace spla {

    /**
     * @brief Math values type.
     *
     * Represents predefined of user-defined type description.
     * In the library values types is effectively a unique type string name,
     * type annotations, size in bytes and etc. Value itself is bytes memory
     * region, interpreted in the way defined by the user.
     */
    class Type final: public Object {
    public:
        ~Type() override = default;

        const std::wstring& GetTypeName() const {
            return mTypeName;
        }

        size_t GetByteSize() const {
            return mByteSize;
        }

        bool IsBuiltIn() const {
            return mBuiltIn;
        }

        /**
         * Makes new user-defined type.
         *
         * @param typeName Unique name of the type
         * @param typeSize Size of the type values in bytes
         * @param library Library global instance
         *
         * @return New type instance
         */
        static RefPtr<Type> MakeType(std::wstring typeName, size_t typeSize, class Library& library);

    private:
        Type(std::wstring typeName, size_t typeSize, bool builtIn, class Library& library);

        // Unique type name
        // Unix: utf-32, Windows: utf-16
        std::wstring mTypeName;

        // Size of the type value in bytes
        // Note: if size is 0 => Object with this type have no values
        size_t mByteSize = 0;

        // True for built-in (predefined) types
        bool mBuiltIn = false;
    };

    /** Inherit from this class if your object must have type info */
    class SPLA_API TypedObject {
    public:

        /** @return True if this and other typed object has compatible types */
        bool IsCompatible(const TypedObject& other) const {
            return mType == other.mType;
        }

        /** @return Type info of this object */
        const RefPtr<Type> &GetType() const {
            return mType;
        }

    protected:

        void SetType(const RefPtr<Type> &type) {
            mType = type;
        }

    private:
        RefPtr<Type> mType;
    };

}

#endif //SPLA_SPLATYPE_HPP
