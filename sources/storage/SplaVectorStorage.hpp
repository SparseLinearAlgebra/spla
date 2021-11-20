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

#ifndef SPLA_SPLAVECTORSTORAGE_HPP
#define SPLA_SPLAVECTORSTORAGE_HPP

#include <mutex>
#include <spla-cpp/SplaLibrary.hpp>
#include <storage/SplaVectorBlock.hpp>
#include <unordered_map>
#include <vector>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class VectorStorage
     */
    class VectorStorage final : public RefCnt {
    public:
        using Index = unsigned int;
        using Entry = std::pair<Index, RefPtr<VectorBlock>>;
        using EntryList = std::vector<Entry>;
        using EntryRowList = std::vector<EntryList>;
        using EntryMap = std::unordered_map<Index, RefPtr<VectorBlock>>;

        ~VectorStorage() override = default;

        /** Remove all blocks from storage (empty matrix) */
        void Clear();

        /** Set block at specified block index */
        void SetBlock(const Index &index, const RefPtr<VectorBlock> &block);

        /** Remove block at specified block index (if present) */
        void RemoveBlock(const Index &index);

        /** Get list of non-null presented blocks in storage */
        void GetBlocks(EntryList &entryList) const;

        /** Get map of non-null presented blocks in storage */
        void GetBlocks(EntryMap &entryMap) const;

        /** Get blocks grid (total number of blocks) */
        void GetBlocksGrid(std::size_t &rows) const;

        /** @return Block at specified index; may be null */
        [[nodiscard]] RefPtr<VectorBlock> GetBlock(const Index &index) const;

        /** @return Number of rows of the storage */
        [[nodiscard]] std::size_t GetNrows() const noexcept;

        /** @return Number of rows of blocks */
        [[nodiscard]] std::size_t GetNblockRows() const noexcept;

        /** @return Number of values in storage */
        [[nodiscard]] std::size_t GetNvals() const noexcept;

        /** Dump vector content to provided stream */
        void Dump(std::ostream &stream) const;

        static RefPtr<VectorStorage> Make(std::size_t nrows, Library &library);

    private:
        VectorStorage(std::size_t nrows, Library &library);

        EntryMap mBlocks;
        std::size_t mNrows;
        std::size_t mNvals = 0;
        std::size_t mNblockRows = 0;
        std::size_t mBlockSize = 0;

        Library &mLibrary;
        mutable std::mutex mMutex;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAVECTORSTORAGE_HPP
