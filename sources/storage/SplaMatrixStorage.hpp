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

    /**
     * @class MatrixStorage
     *
     * Blocked matrix storage class. Stores matrix data as set of equally sized blocks,
     * where each block has size `library.blockSize` x `library.blockSize`.
     * Right and bottom blocks has clamped size, so they fit matrix dimension.
     *
     * Blocks are indexed using {i,j} indices, where i in [0..blocks in row)
     * and j in [0..blocks in col). Empty blocks are not stored.
     *
     * Each block can have its own storage format. Common formats are COO, CSR, dense etc.
     * Each block stores indices in a unified way, so values (i,j) within block are in range
     * [0..M) and [0..N), where M and N number of rows and number of columns of the block respectively.
     *
     * Example:
     * @code
     *      // 4x4 matrix
     *      Matrix: [ (0, 0, 0.0f),
     *                (1, 3, 1.0f),
     *                (3, 2, 4.0f),
     *                (3, 3, 7.0f) ]
     *
     *      // 2x2 block storage with block size 2
     *      Storage: { (0, 0) : [ (0, 0, 0.0f) ],
     *                 (0, 1) : [ (1, 1, 1.0f) ],
     *                 (1, 0) : null,
     *                 (1, 1) : [ (1, 0, 4.0f), (1, 1, 7.0f) ] )}
     * @endcode
     *
     * @see MatrixCOO
     * @see MatrixBlock
     *
     * @note Thread-safe
     */
    class MatrixStorage final : public RefCnt {
    public:
        using Index = std::pair<unsigned int, unsigned int>;
        using Entry = std::pair<Index, RefPtr<MatrixBlock>>;
        using EntryList = std::vector<Entry>;
        using EntryRowList = std::vector<std::vector<Entry>>;

        ~MatrixStorage() override = default;

        /** Set block at specified block index */
        void SetBlock(const Index &index, const RefPtr<MatrixBlock> &block);

        /** Remove block at specified block index (if present) */
        void RemoveBlock(const Index &index);

        /** Get list of non-null presented blocks in storage */
        void GetBlocks(EntryList &entryList) const;

        /** Get list of non-null presented blocks in storage per row */
        void GetBlocks(EntryRowList &entryList) const;

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

        /**
         * Make new matrix storage.
         *
         * @param nrows Number of rows in matrix
         * @param ncols Number of column in matrix
         * @param library Library instance
         *
         * @return New empty storage instance
         */
        static RefPtr<MatrixStorage> Make(size_t nrows, size_t ncols, Library &library);

    private:
        MatrixStorage(size_t nrows, size_t ncols, Library &library);

        std::unordered_map<Index, RefPtr<MatrixBlock>, PairHash> mBlocks;
        size_t mNrows;
        size_t mNcols;
        size_t mNvals = 0;
        size_t mNblockRows = 0;
        size_t mNblockCols = 0;
        size_t mBlockSize = 0;

        Library &mLibrary;
        mutable std::mutex mMutex;
    };

}// namespace spla

#endif//SPLA_SPLAMATRIXSTORAGE_HPP
