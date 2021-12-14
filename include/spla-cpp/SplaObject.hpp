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

#ifndef SPLA_SPLAOBJECT_HPP
#define SPLA_SPLAOBJECT_HPP

#include <spla-cpp/SplaRefCnt.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class Object
     *
     * Base class for any spla library object, which can be used in math operations.
     */
    class SPLA_API Object : public RefCnt {
    public:
        ~Object() override = default;

        /** Type name of concrete object. */
        enum class TypeName {
            Type,
            Matrix,
            Vector,
            Scalar,
            Descriptor,
            FunctionUnary,
            FunctionBinary,
            FunctionSelect,
            Expression,
            ExpressionNode,
            DataMatrix,
            DataVector,
            DataScalar,
            Unknown
        };

        explicit Object(TypeName typeName, class Library &library);

        /** @return Object string text label for debugging */
        const std::string &GetLabel() const;

        /** @return Type specification of the object */
        TypeName GetTypeName() const;

        /** @return Library instance that this object is associated with */
        class Library &GetLibrary() const;

    private:
        friend class ExpressionManager;
        virtual RefPtr<Object> CloneEmpty();
        virtual void CopyData(const RefPtr<Object> &object);

    private:
        // User defined text label (for profiling/debugging)
        std::string mLabel;
        // Type of the object
        TypeName mTypeName = TypeName::Unknown;
        // Global library instance
        class Library &mLibrary;
    };

    namespace {
        /** @return Object type string name */
        inline const char *ObjectTypeToStr(Object::TypeName typeName) {
            switch (typeName) {
                case Object::TypeName::Type:
                    return "Type";
                case Object::TypeName::Matrix:
                    return "Matrix";
                case Object::TypeName::Vector:
                    return "Vector";
                case Object::TypeName::Scalar:
                    return "Scalar";
                case Object::TypeName::Descriptor:
                    return "Descriptor";
                case Object::TypeName::FunctionUnary:
                    return "FunctionUnary";
                case Object::TypeName::FunctionBinary:
                    return "FunctionBinary";
                case Object::TypeName::FunctionSelect:
                    return "FunctionSelect";
                case Object::TypeName::Expression:
                    return "Expression";
                case Object::TypeName::ExpressionNode:
                    return "ExpressionNode";
                case Object::TypeName::DataMatrix:
                    return "DataMatrix";
                case Object::TypeName::DataVector:
                    return "DataVector";
                case Object::TypeName::DataScalar:
                    return "DataScalar";

                default:
                    return "Unknown";
            }
        }
    }// namespace

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAOBJECT_HPP
