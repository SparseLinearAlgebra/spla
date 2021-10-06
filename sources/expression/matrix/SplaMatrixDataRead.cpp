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

#include <boost/compute.hpp>
#include <detail/SplaError.hpp>
#include <detail/SplaLibraryPrivate.hpp>
#include <detail/SplaMath.hpp>
#include <expression/matrix/SplaMatrixDataRead.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <vector>

bool spla::MatrixDataRead::Select(size_t nodeIdx, spla::ExpressionContext &context) {
    return true;
}

void spla::MatrixDataRead::Process(size_t nodeIdx, spla::ExpressionContext &context) {
    auto &taskflow = context.nodesTaskflow[nodeIdx];
    auto &nodes = context.expression->GetNodes();
    auto node = nodes[nodeIdx];
    auto library = context.expression->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();

    auto matrix = node->GetArg(0).Cast<Matrix>();
    auto matrixData = node->GetArg(1).Cast<DataMatrix>();
    auto desc = node->GetDescriptor();

    assert(matrix.IsNotNull());
    assert(matrixData.IsNotNull());
    assert(desc.IsNotNull());

    // 1. For each block compute nnz per row
    // 2. For each row of blocks in storage compute offsets
    // 3. Copy values of each block to final positions

    struct Shared {
        using Index = MatrixStorage::Index;
        using EntryList = MatrixStorage::EntryList;
        using NnzBuffer = boost::compute::vector<unsigned int>;

        mutable std::mutex mutex;
        EntryList blocks;
        std::vector<size_t> blocksNnz;
        std::unordered_map<Index, NnzBuffer, PairHash> blocksNnzPerRow;
    };

    auto storage = matrix->GetStorage();

    CHECK_RAISE_ERROR(matrixData->GetNvals() >= storage->GetNvals(), InvalidArgument,
                      "Provided data arrays do not have enough space to store matrix data");

    auto shared = std::make_shared<Shared>();
    storage->GetBlocks(shared->blocks);

    tf::Taskflow stage1;

    for (auto &entry : shared->blocks) {
        stage1.emplace([=]() {
            using namespace boost;

            auto index = entry.first;
            auto block = entry.second.Cast<MatrixCOO>();
            CHECK_RAISE_ERROR(block.IsNotNull(), NotImplemented, "Supported only COO format blocks");

            // todo: gpu and device queue management
            compute::device gpu = library->GetDevices()[0];
            compute::context ctx = library->GetContext();
            compute::command_queue queue(ctx, gpu);

            // Allocate buffer with counter for each row (initially is zero)
            compute::vector<unsigned int> elementsPerRow(block->GetNrows(), ctx);
            compute::fill(elementsPerRow.begin(), elementsPerRow.end(), 0, queue);

            const auto &rows = block->GetRows();

            // Compute number of nnz per row
            BOOST_COMPUTE_CLOSURE(void, countElements, (unsigned int i), (rows, elementsPerRow), {
                atomic_inc(&elementsPerRow[rows[i]]);
            });

            // Save results
            {
                std::lock_guard<std::mutex> lock(shared->mutex);
                shared->blocksNnz[index.first] += block->GetNvals();
                shared->blocksNnzPerRow.emplace(index, std::move(elementsPerRow));
            }
        });

        taskflow.composed_of(stage1);

        tf::Taskflow stage2;
    }
}

spla::ExpressionNode::Operation spla::MatrixDataRead::GetOperationType() const {
    return ExpressionNode::Operation::MatrixDataRead;
}
