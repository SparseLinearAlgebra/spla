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
#include <storage/SplaMatrixStorage.hpp>

void spla::MatrixStorage::SetBlock(const spla::MatrixStorage::Index &index, const spla::RefPtr<spla::MatrixBlock> &block) {
    assert(index.first < mNblockRows);
    assert(index.second < mNblockCols);
    assert(block.IsNotNull());

    std::lock_guard<std::mutex> lock(mMutex);
    auto &prev = mBlocks[index];

    if (prev.IsNotNull())
        mNvals -= prev->GetNvals();

    prev = block;
    mNvals += block->GetNvals();
}

void spla::MatrixStorage::GetBlocks(spla::MatrixStorage::EntryList &entryList) const {
    std::lock_guard<std::mutex> lock(mMutex);
    entryList.clear();
    entryList.insert(entryList.begin(), mBlocks.begin(), mBlocks.end());
}

void spla::MatrixStorage::GetBlocksGrid(size_t &rows, size_t &cols) const {
    rows = mNblockRows;
    cols = mNblockCols;
}

spla::RefPtr<spla::MatrixBlock> spla::MatrixStorage::GetBlock(const spla::MatrixStorage::Index &index) const {
    assert(index.first < mNblockRows);
    assert(index.second < mNblockCols);
    std::lock_guard<std::mutex> lock(mMutex);
    auto entry = mBlocks.find(index);
    return entry != mBlocks.end() ? entry->second : nullptr;
}

size_t spla::MatrixStorage::GetNrows() const noexcept {
    return mNrows;
}

size_t spla::MatrixStorage::GetNcols() const noexcept {
    return mNcols;
}

size_t spla::MatrixStorage::GetNvals() const noexcept {
    std::lock_guard<std::mutex> lock(mMutex);
    return mNvals;
}

spla::RefPtr<spla::MatrixStorage> spla::MatrixStorage::Make(size_t nrows, size_t ncols, spla::Library &library) {
    assert(nrows > 0);
    assert(ncols > 0);
    return spla::RefPtr<spla::MatrixStorage>(new MatrixStorage(nrows, ncols, library));
}

spla::MatrixStorage::MatrixStorage(size_t nrows, size_t ncols, spla::Library &library)
    : mNrows(nrows), mNcols(ncols), mLibrary(library) {
    mBlockSize = mLibrary.GetPrivate().GetBlockSize();
    mNblockRows = math::GetBlocksCount(nrows, mBlockSize);
    mNblockCols = math::GetBlocksCount(ncols, mBlockSize);
}
