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

#ifndef SPLA_MATRIX_BLOCK_HPP
#define SPLA_MATRIX_BLOCK_HPP

#include <spla/detail/ref.hpp>
#include <spla/types.hpp>

namespace spla::detail {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class MatrixBlock
     * @brief Base class for a block of matrix data inside matrix storage
     *
     * @tparam T Type of stored matrix values
     */
    template<typename T>
    class MatrixBlock : public detail::RefCnt {
    public:
        MatrixBlock(std::size_t nrows, std::size_t ncols, std::size_t nvals)
            : m_nrows(nrows), m_ncols(ncols), m_nvals(nvals) {}

        ~MatrixBlock() override = default;

        std::size_t nrows() const { return m_nrows; }
        std::size_t ncols() const { return m_ncols; }
        std::size_t nvals() const { return m_nvals; }

        std::size_t &nvals() { return m_nvals; }

    protected:
        std::size_t m_nrows;
        std::size_t m_ncols;
        std::size_t m_nvals;
    };

    /**
     * @}
     */

}// namespace spla::detail

#endif//SPLA_MATRIX_BLOCK_HPP
