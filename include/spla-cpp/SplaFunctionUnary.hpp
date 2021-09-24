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

#ifndef SPLA_SPLAFUNCTIONUNARY_HPP
#define SPLA_SPLAFUNCTIONUNARY_HPP

#include <bitset>
#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaType.hpp>
#include <string>

namespace spla {

    class SPLA_API FunctionUnary final : public Object {
    public:
        ~FunctionUnary() override = default;

        enum class Flags {
            Max = 1
        };

        const RefPtr<Type> &GetArg1() const {
            return mArg1;
        }

        const RefPtr<Type> &GetResult() const {
            return mResult;
        }

        const std::string &GetSource() const {
            return mSource;
        }

        bool IsApplicable(const TypedObject &arg1, const TypedObject &result) const {
            return arg1.GetType() == mArg1 && result.GetType() == mResult;
        }

    private:
        RefPtr<Type> mArg1;
        RefPtr<Type> mResult;
        std::string mSource;
        std::bitset<static_cast<size_t>(Flags::Max)> mFlags;
    };

}// namespace spla

#endif//SPLA_SPLAFUNCTIONUNARY_HPP
