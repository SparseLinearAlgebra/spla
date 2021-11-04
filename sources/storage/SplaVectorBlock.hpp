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

#ifndef SPLA_SPLAVECTORBLOCK_HPP
#define SPLA_SPLAVECTORBLOCK_HPP

#include <spla-cpp/SplaRefCnt.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class VectorBlock
     *
     * Base class for a vector block.
     * Used in vector storage to represent block of the vector with specific sparse storage schema.
     * Common vector blocks: COO and dense.
     */
    class VectorBlock : public RefCnt {
    public:
        /** Sparse vector formats */
        enum class Format {
            COO,
            Dense
        };

        VectorBlock(std::size_t nrows, std::size_t nvals, Format format)
            : mNrows(nrows), mNvals(nvals), mFormat(format) {}

        ~VectorBlock() override = default;

        /** @return Number of rows of the block */
        [[nodiscard]] std::size_t GetNrows() const noexcept {
            return mNrows;
        }

        /** @return Number of values in block */
        [[nodiscard]] std::size_t GetNvals() const noexcept {
            return mNvals;
        }

        /** @return Sparse format name of this block */
        [[nodiscard]] Format GetFormat() const noexcept {
            return mFormat;
        }

        /** Dump vector content to provided stream */
        virtual void Dump(std::ostream &stream, unsigned int baseI) const = 0;

    protected:
        std::size_t mNrows;
        std::size_t mNvals;
        Format mFormat;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAVECTORBLOCK_HPP
