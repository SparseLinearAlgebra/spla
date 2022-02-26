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

#ifndef SPLA_REFERENCE_VECTOR_DENSE_HPP
#define SPLA_REFERENCE_VECTOR_DENSE_HPP

#include <cassert>
#include <vector>

#include <spla/storage/vector_block.hpp>

namespace spla::reference {

    template<typename T>
    class VectorDense : public VectorBlock<T> {
    public:
        VectorDense(std::size_t nrows, std::size_t nvals, std::vector<Index> mask, std::vector<T> values)
            : VectorBlock<T>(nrows, nvals), m_mask(std::move(mask)), m_values(std::move(values)) {
            assert(m_mask.size() == nrows);
            assert(m_values.size() == nrows || !type_has_values<T>());
        }

        ~VectorDense() override = default;

        [[nodiscard]] const std::vector<Index> &mask() const { return m_mask; }
        [[nodiscard]] const std::vector<T> &values() const { return m_values; }

    private:
        std::vector<Index> m_mask;
        std::vector<T> m_values;
    };

}// namespace spla::reference

#endif//SPLA_REFERENCE_VECTOR_DENSE_HPP
