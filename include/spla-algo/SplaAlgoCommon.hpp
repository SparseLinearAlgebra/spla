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

#ifndef SPLA_SPLAALGOCOMMON_HPP
#define SPLA_SPLAALGOCOMMON_HPP

#include <spla-cpp/SplaConfig.hpp>
#include <spla-cpp/SplaData.hpp>

#include <vector>

namespace spla {

    /**
     * @addtogroup Algorithm
     * @{
     */

    /**
     * @class HostVector
     * @brief Represents host vector
     *
     * Host data wrapper for the vector primitive.
     * Used in host (cpu) reference algorithms implementations.
     */
    class HostVector final : public RefCnt {
    public:
        SPLA_API HostVector(Size nrows, std::vector<Index> rows, std::vector<unsigned char> vals);
        SPLA_API ~HostVector() override = default;

        SPLA_API RefPtr<DataVector> GetData(Library &library);

        [[nodiscard]] bool HasValues() const { return !mValues.empty(); }

        [[nodiscard]] Size GetNrows() const { return mNrows; }
        [[nodiscard]] Size GetNnvals() const { return mNnvals; }
        [[nodiscard]] Size GetElementSize() const { return mElementSize; }

        [[nodiscard]] const std::vector<Index> &GetRowIndices() { return mRowIndices; }
        [[nodiscard]] const std::vector<unsigned char> &GetValues() { return mValues; }

    private:
        Size mNrows;
        Size mNnvals;
        Size mElementSize;
        std::vector<Index> mRowIndices;
        std::vector<unsigned char> mValues;
    };

    /**
     * @class HostMatrix
     * @brief Represents host matrix
     *
     * Host data wrapper for the matrix primitive.
     * Used in host (cpu) reference algorithms implementations.
     */
    class HostMatrix final : public RefCnt {
    public:
        SPLA_API HostMatrix(Size nrows, Size ncols, std::vector<Index> rows, std::vector<Index> cols, std::vector<unsigned char> vals);
        SPLA_API ~HostMatrix() override = default;

        SPLA_API RefPtr<DataMatrix> GetData(Library &library);

        [[nodiscard]] bool HasValues() const { return !mValues.empty(); }

        [[nodiscard]] Size GetNrows() const { return mNrows; }
        [[nodiscard]] Size GetNcols() const { return mNcols; }
        [[nodiscard]] Size GetNnvals() const { return mNnvals; }
        [[nodiscard]] Size GetElementSize() const { return mElementSize; }

        [[nodiscard]] const std::vector<Index> &GetRowIndices() { return mRowIndices; }
        [[nodiscard]] const std::vector<Index> &GetColIndices() { return mColIndices; }
        [[nodiscard]] const std::vector<unsigned char> &GetValues() { return mValues; }

    private:
        Size mNrows;
        Size mNcols;
        Size mNnvals;
        Size mElementSize;
        std::vector<Index> mRowIndices;
        std::vector<Index> mColIndices;
        std::vector<unsigned char> mValues;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAALGOCOMMON_HPP
