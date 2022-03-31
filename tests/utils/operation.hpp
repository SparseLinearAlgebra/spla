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

#ifndef SPLA_TEST_UTILS_OPERATION_HPP
#define SPLA_TEST_UTILS_OPERATION_HPP

#include <vector>

#include <utils/matrix.hpp>
#include <utils/vector.hpp>

#include <spla/spla.hpp>

namespace testing {

    template<typename T>
    bool equals(const spla::Vector<T> &a, const spla::Vector<T> &b) {
        if (a.nvals() != b.nvals())
            return false;

        std::vector<spla::Index> a_rows;
        std::vector<spla::Index> a_values;

        std::vector<spla::Index> b_rows;
        std::vector<spla::Index> b_values;

        spla::Expression read_data;
        read_data.read(a, [&](auto &r, auto &v) {a_rows=std::move(r); a_values=std::move(a_values); });
        read_data.read(b, [&](auto &r, auto &v) {b_rows=std::move(r); b_values=std::move(a_values); });
        read_data.submit().wait();

        auto size = a.nvals();

        for (std::size_t i = 0; i < size; i++) {
            if (a_rows[i] != b_rows[i])
                return false;
        }

        if (!a_values.empty() && !b_values.empty()) {
            for (std::size_t i = 0; i < size; i++) {
                if (a_values[i] != b_values[i])
                    return false;
            }
        }

        return true;
    }

}// namespace testing

#endif//SPLA_TEST_UTILS_OPERATION_HPP
