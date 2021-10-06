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

#ifndef SPLA_SPLAMATRIXSTORAGE_HPP
#define SPLA_SPLAMATRIXSTORAGE_HPP

#include <detail/SplaHash.hpp>
#include <mutex>
#include <spla-cpp/SplaLibrary.hpp>
#include <storage/SplaMatrixBlock.hpp>
#include <unordered_map>
#include <vector>

namespace spla {

    class MatrixStorage final : public RefCnt {
    public:
        using Index = std::pair<unsigned int, unsigned int>;
        using Entry = std::pair<Index, RefPtr<MatrixBlock>>;
        using EntryList = std::vector<Entry>;

        ~MatrixStorage() override = default;

        /** Set block at specified block index */
        void SetBlock(const Index &index, const RefPtr<MatrixBlock> &block);

        /** Remove block at specified block index (if present) */
        void RemoveBlock(const Index &index);

        /** Get list of non-null presented blocks in storage */
        void GetBlocks(EntryList &entryList) const;

        /** Get blocks grid (number of blocks in each dimension) */
        void GetBlocksGrid(size_t &rows, size_t &cols) const;

        /** @return Block at specified index; may be null */
        RefPtr<MatrixBlock> GetBlock(const Index &index) const;

        /** @return Number of rows of the storage */
        [[nodiscard]] size_t GetNrows() const noexcept;

        /** @return Number of columns of the storage */
        [[nodiscard]] size_t GetNcols() const noexcept;

        /** @return Number of values in storage */
        [[nodiscard]] size_t GetNvals() const noexcept;

        static RefPtr<MatrixStorage> Make(size_t nrows, size_t ncols, Library &library);

    private:
        MatrixStorage(size_t nrows, size_t ncols, Library &library);

        std::unordered_map<Index, RefPtr<MatrixBlock>, PairHash> mBlocks;
        mutable std::mutex mMutex;
        size_t mNrows;
        size_t mNcols;
        size_t mNvals = 0;
        size_t mNblockRows = 0;
        size_t mNblockCols = 0;
        size_t mBlockSize = 0;

        Library &mLibrary;
    };

}// namespace spla

#endif//SPLA_SPLAMATRIXSTORAGE_HPP
