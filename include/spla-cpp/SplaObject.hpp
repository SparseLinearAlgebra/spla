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

    /** Type name of concrete object. */
    enum class ObjectType {
        Type,
        Matrix,
        Vector,
        Scalar,
        Descriptor,
        FunctionUnary,
        FunctionBinary,
        Unknown
    };

    /**
     * @brief Math object.
     *
     * Base class for any spla library object, which can be used in math operations.
     */
    class Object: public RefCnt {
    public:
        explicit Object(class Library& library) : mLibrary(library) { }
        ~Object() override = default;

        const std::wstring& GetLabel() const {
            return mLabel;
        }

        ObjectType GetType() const {
            return mType;
        }

        class Library& GetLibrary() const {
            return mLibrary;
        }

    private:
        // User defined text label (for profiling/debugging)
        // Unix: utf-32, Windows: utf-16
        std::wstring mLabel;
        // Type of the object
        ObjectType mType = ObjectType::Unknown;
        // Global library instance
        class Library& mLibrary;
    };

}

#endif //SPLA_SPLAOBJECT_HPP
