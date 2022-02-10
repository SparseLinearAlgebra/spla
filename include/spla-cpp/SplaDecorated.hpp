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

#ifndef SPLA_SPLADECORATED_HPP
#define SPLA_SPLADECORATED_HPP

#include <mutex>
#include <vector>

#include <spla-cpp/SplaConfig.hpp>
#include <spla-cpp/SplaObject.hpp>

namespace spla {

    /**
     * @class Decorated
     * @brief Generic object which supports decorations
     *
     * A decoration is additional named property or info, assigned to
     * the object in for of another object. Decorations can be used to
     * cache transposed matrix, transformed into another format vector
     * primitive, or provide alternative implementations for some functions.
     *
     * Decoration can be utilised by the library to automatically select more
     * suitable and performant operation algorithm implementation for faster
     * processing and runtime.
     *
     * @note Decoration has type of Object.
     * @note Decorations can be assigned by the user.
     */
    class SPLA_API Decorated {
    public:
        enum class Decoration {
            /** Transposed matrix decoration */
            TransposedMatrix = 0,
            /** Total decorations count */
            Max = 1
        };

        Decorated();

        /**
         * @brief Sets decoration with specified type
         * Previous decoration with the same type is removed.
         *
         * @param decoration Decoration type
         * @param object Object to set
         */
        void SetDecoration(Decoration decoration, const RefPtr<Object> &object);

        /**
         * @brief Removes decoration with specified type
         *
         * @param decoration Decoration type
         */
        void RemoveDecoration(Decoration decoration);

        /**
         * @brief Check if decoration with specified type is present
         *
         * @param decoration Decoration type
         * @return True if has decoration
         */
        bool HasDecoration(Decoration decoration) const;

        /**
         * @brief Return decoration object with specified type
         *
         * @param decoration Decoration type
         * @return Decoration object; may be null
         */
        RefPtr<Object> GetDecoration(Decoration decoration) const;

    private:
        std::vector<RefPtr<Object>> mDecorations; /** By enum id */
        mutable std::mutex mMutex;                /** Safe multi-thread access */
    };

    namespace {
        /** @return Decoration name as string */
        inline const char *DecorationTypeToStr(Decorated::Decoration decoration) {
            switch (decoration) {
                case Decorated::Decoration::TransposedMatrix:
                    return "TransposedMatrix";

                default:
                    return "Unknown";
            }
        }
    }// namespace

}// namespace spla

#endif//SPLA_SPLADECORATED_HPP
