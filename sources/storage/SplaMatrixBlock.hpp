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

#ifndef SPLA_SPLAMATRIXBLOCK_HPP
#define SPLA_SPLAMATRIXBLOCK_HPP

#include <spla-cpp/SplaRefCnt.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class MatrixBlock
     *
     * Base class for a matrix block.
     * Used in matrix storage to represent block of the matrix with specific sparse storage schema.
     * Common matrix blocks: CSR, COO, dense and etc.
     */
    class MatrixBlock : public RefCnt {
    public:
        /** Sparse matrix formats */
        enum class Format {
            CSR,
            COO,
            Dense
        };

        MatrixBlock(std::size_t nrows, std::size_t ncols, std::size_t nvals, Format format)
            : mNrows(nrows), mNcols(ncols), mNvals(nvals), mFormat(format) {}

        ~MatrixBlock() override = default;

        /** @return Number of rows of the block */
        [[nodiscard]] std::size_t GetNrows() const noexcept {
            return mNrows;
        }

        /** @return Number of columns of the block */
        [[nodiscard]] std::size_t GetNcols() const noexcept {
            return mNcols;
        }

        /** @return Number of values in block */
        [[nodiscard]] std::size_t GetNvals() const noexcept {
            return mNvals;
        }

        /** @return Sparse format name of this block */
        [[nodiscard]] Format GetFormat() const noexcept {
            return mFormat;
        }

        /** Dump block content to provided stream */
        virtual void Dump(std::ostream &stream, unsigned int baseI, unsigned int baseJ) const = 0;

        /** @return Size of the stored value (in bytes) */
        [[nodiscard]] virtual std::size_t GetValueByteSize() const noexcept = 0;

    protected:
        std::size_t mNrows;
        std::size_t mNcols;
        std::size_t mNvals;
        Format mFormat;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAMATRIXBLOCK_HPP
