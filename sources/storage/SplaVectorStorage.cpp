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

#include <detail/SplaLibraryPrivate.hpp>
#include <detail/SplaMath.hpp>
#include <storage/SplaVectorStorage.hpp>

void spla::VectorStorage::SetBlock(const spla::VectorStorage::Index &index, const spla::RefPtr<spla::VectorBlock> &block) {
    assert(index < mNblockRows);
    assert(block.IsNotNull());

    std::lock_guard<std::mutex> lock(mMutex);
    auto &prev = mBlocks[index];

    if (prev.IsNotNull())
        mNvals -= prev->GetNvals();

    prev = block;
    mNvals += block->GetNvals();
}

void spla::VectorStorage::GetBlocks(spla::VectorStorage::EntryList &entryList) const {
    std::lock_guard<std::mutex> lock(mMutex);
    entryList.clear();
    entryList.insert(entryList.begin(), mBlocks.begin(), mBlocks.end());
}

void spla::VectorStorage::GetBlocksGrid(std::size_t &rows) const {
    rows = mNblockRows;
}

spla::RefPtr<spla::VectorBlock> spla::VectorStorage::GetBlock(const spla::VectorStorage::Index &index) const {
    assert(index < mNblockRows);
    std::lock_guard<std::mutex> lock(mMutex);
    auto entry = mBlocks.find(index);
    return entry != mBlocks.end() ? entry->second : nullptr;
}

std::size_t spla::VectorStorage::GetNrows() const noexcept {
    return mNrows;
}

std::size_t spla::VectorStorage::GetNvals() const noexcept {
    std::lock_guard<std::mutex> lock(mMutex);
    return mNvals;
}

spla::RefPtr<spla::VectorStorage> spla::VectorStorage::Make(std::size_t nrows, spla::Library &library) {
    return {new VectorStorage(nrows, library)};
}

spla::VectorStorage::VectorStorage(std::size_t nrows, spla::Library &library)
    : mNrows(nrows), mLibrary(library) {
    mBlockSize = mLibrary.GetPrivate().GetBlockSize();
    mNblockRows = math::GetBlocksCount(nrows, mBlockSize);
}

void spla::VectorStorage::RemoveBlock(const spla::VectorStorage::Index &index) {
    assert(index < mNblockRows);

    std::lock_guard<std::mutex> lock(mMutex);
    mBlocks.erase(index);
}

std::size_t spla::VectorStorage::GetNblockRows() const noexcept {
    return mNblockRows;
}
