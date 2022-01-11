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
#ifndef SPLA_SPLASCALARBUFFER_HPP
#define SPLA_SPLASCALARBUFFER_HPP

#include <spla-cpp/SplaFunctionBinary.hpp>
#include <storage/SplaScalarValue.hpp>

namespace spla::detail {

    class ScalarBuffer {
    public:
        void Add(RefPtr<ScalarValue> scalar);

        [[nodiscard]] std::size_t GetNScalars() const noexcept;

        [[nodiscard]] RefPtr<ScalarValue> FirstScalar() const;

        [[nodiscard]] boost::compute::vector<unsigned char> Reduce(const RefPtr<FunctionBinary> &reduce,
                                                                   boost::compute::command_queue &queue);

    private:
        [[nodiscard]] std::size_t BuildSharedBuffer(std::size_t byteSize,
                                                    boost::compute::vector<unsigned char> &buffer,
                                                    boost::compute::command_queue &queue) const;

        mutable std::mutex mMutex;
        std::deque<RefPtr<ScalarValue>> mScalars;
    };

}// namespace spla::detail

#endif//SPLA_SPLASCALARBUFFER_HPP
