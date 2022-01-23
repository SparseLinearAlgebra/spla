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

#include <algo/SplaAlgorithmManager.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <expression/prod/SplaVxM.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/SplaVectorStorage.hpp>

namespace spla {
    namespace {
        /** Utility to fetch mask block from entry map */
        inline RefPtr<VectorBlock> GetMaskBlock(VectorStorage::EntryMap &map, const VectorStorage::Index &idx) {
            auto found = map.find(idx);
            return found != map.end() ? found->second : RefPtr<VectorBlock>{};
        }

        /** Used to aggregate non empty results of a[i] x b[i,j] products for each (j) entry */
        class ProductsResults {
        public:
            explicit ProductsResults(std::size_t n) : mBlocks(n) {}

            void AddBlock(std::size_t j, const RefPtr<VectorBlock> &block) {
                std::lock_guard<std::mutex> lockGuard(mMutex);
                mBlocks[j].push_back(block);
            }

            void GetBlocks(std::size_t j, std::vector<RefPtr<VectorBlock>> &blocks) {
                std::lock_guard<std::mutex> lockGuard(mMutex);
                blocks = mBlocks[j];
            }

        private:
            std::vector<std::vector<RefPtr<VectorBlock>>> mBlocks;
            mutable std::mutex mMutex;
        };
    }// namespace
}// namespace spla

bool spla::VxM::Select(std::size_t, const spla::Expression &) {
    return true;
}

void spla::VxM::Process(std::size_t nodeIdx, const spla::Expression &expression, spla::TaskBuilder &builder) {
    auto &nodes = expression.GetNodes();
    auto &node = nodes[nodeIdx];
    auto library = node->GetLibrary().GetPrivatePtr();
    auto logger = library->GetLogger();
    auto &deviceMan = library->GetDeviceManager();

    auto w = node->GetArg(0).Cast<Vector>();
    auto mask = node->GetArg(1).Cast<Vector>();
    auto mult = node->GetArg(2).Cast<FunctionBinary>();
    auto add = node->GetArg(3).Cast<FunctionBinary>();
    auto a = node->GetArg(4).Cast<Vector>();
    auto b = node->GetArg(5).Cast<Matrix>();
    auto desc = node->GetDescriptor();

    assert(w.IsNotNull());
    assert(a.IsNotNull());
    assert(b.IsNotNull());
    assert(desc.IsNotNull());

    // Clear w, so by default empty result returned
    w->GetStorage()->Clear();

    auto ta = a->GetType();
    auto tb = b->GetType();
    auto tw = w->GetType();
    auto hasMask = mask.IsNotNull();
    auto &aStorage = a->GetStorage();
    auto &bStorage = b->GetStorage();

    // Query block storage dimensions for vectors and matrix
    std::size_t nBlockM = aStorage->GetNblockRows(),
                nBlockN = bStorage->GetNblockCols();

    using IndexV = VectorStorage::Index;
    using IndexM = MatrixStorage::Index;
    struct ToProcess {
        IndexV a;
        IndexM b;
    };

    // Fetch blocks and store locally
    VectorStorage::EntryMap aBlocks;
    MatrixStorage::EntryMap bBlocks;
    VectorStorage::EntryMap maskBlocks;
    aStorage->GetBlocks(aBlocks);
    bStorage->GetBlocks(bBlocks);

    if (hasMask)
        // If mask empty => does not apply mask at all
        mask->GetStorage()->GetBlocks(maskBlocks);

    // Determine number of block products and product pairs for each result block
    std::size_t totalProducts = 0;
    std::size_t totalBlocks = 0;
    std::vector<std::vector<ToProcess>> blockProducts(nBlockN);
    for (std::size_t j = 0; j < nBlockN; j++) {
        auto &toProcess = blockProducts[j];
        for (std::size_t i = 0; i < nBlockM; i++) {
            IndexV aIndex{static_cast<unsigned int>(i)};
            IndexM bIndex{static_cast<unsigned int>(i), static_cast<unsigned int>(j)};
            auto aBlock = aBlocks.find(aIndex);
            auto bBlock = bBlocks.find(bIndex);

            // If has something to multiply in both a[i,k] and b[k,j] blocks
            if (aBlock != aBlocks.end() && bBlock != bBlocks.end()) {
                toProcess.push_back(ToProcess{aIndex, bIndex});
                totalProducts += 1;
            }
        }
        // Count number of potentially not empty final w[j] blocks
        totalBlocks += toProcess.empty() ? 0 : 1;
    }

    // Edge case: if no products, return empty result
    if (totalProducts == 0) {
        return;
    }

    // Shared thread-safe storage to aggregate results of products
    auto products = std::make_shared<ProductsResults>(nBlockN);

    // Query required number of devices (strategy: device per product)
    auto devicesForProducts = deviceMan.FetchDevices(totalProducts, node);

    // Dispatch tasks to compute a.block[i] x b.block[i,j] products
    std::size_t deviceToFetch = 0;
    std::vector<std::vector<tf::Task>> blockProductsTasks(nBlockN);
    for (std::size_t j = 0; j < nBlockN; j++) {
        auto &tasks = blockProductsTasks[j];
        for (auto &toProcess : blockProducts[j]) {
            auto deviceId = devicesForProducts[deviceToFetch];
            auto aIdx = toProcess.a;
            auto bIdx = toProcess.b;
            auto aBlock = aBlocks.find(aIdx)->second;
            auto bBlock = bBlocks.find(bIdx)->second;
            auto maskBlock = GetMaskBlock(maskBlocks, IndexV{bIdx.second});
            auto task = builder.Emplace([=]() {
                assert(aBlock->GetNrows() == bBlock->GetNrows());
                ParamsVxM params;
                params.desc = desc;
                params.deviceId = deviceId;
                params.hasMask = hasMask;
                params.mask = maskBlock;
                params.mult = mult;
                params.add = add;
                params.a = aBlock;
                params.b = bBlock;
                params.ta = ta;
                params.tb = tb;
                params.tw = tw;
                library->GetAlgoManager()->Dispatch(Algorithm::Type::VxM, params);

                if (params.w.IsNotNull()) {
                    // If has not empty result, store it to sum later
                    products->AddBlock(j, params.w);
                    SPDLOG_LOGGER_TRACE(logger, "Blocks product ({})x({},{}) nnz={}",
                                        aIdx, bIdx.first, bIdx.second, params.w->GetNvals());
                }
            });
            deviceToFetch += 1;
            tasks.push_back(std::move(task));
        }
    }

    // Query required number of devices (strategy: device per not empty w[j])
    auto devicesForFinalMerge = deviceMan.FetchDevices(totalBlocks, node);

    // Finally, for each block w[i,j] we must aggregate intermediate
    // blocks multiplications results as a series of element-wise additions of blocks
    deviceToFetch = 0;
    for (std::size_t j = 0; j < nBlockN; j++) {
        auto &toProcess = blockProducts[j];
        if (!toProcess.empty()) {
            auto deviceId = devicesForFinalMerge[deviceToFetch];
            auto task = builder.Emplace([=]() {
                std::vector<RefPtr<VectorBlock>> blocks;
                products->GetBlocks(j, blocks);

                // Nothing to do, w[j] is empty
                if (blocks.empty())
                    return;

                // Start to merge n blocks, number of merges n - 1
                auto block = blocks[0];
                for (std::size_t k = 1; k < blocks.size(); k++) {
                    assert(block->GetNrows() == blocks[k]->GetNrows());
                    ParamsVectorEWiseAdd params;
                    params.desc = desc;
                    params.deviceId = deviceId;
                    params.op = add;
                    params.a = block;
                    params.b = blocks[k];
                    params.type = tw;
                    library->GetAlgoManager()->Dispatch(Algorithm::Type::VectorEWiseAdd, params);

                    // Store result for next iteration
                    block = params.w;
                }

                // Store final result
                IndexV index{static_cast<unsigned int>(j)};
                w->GetStorage()->SetBlock(index, block);
            });

            // Setup dependencies
            // Start aggregation as soon as all sums are computed for w[j]
            auto &deps = blockProductsTasks[j];
            for (auto &parent : deps)
                parent.precede(task);

            deviceToFetch += 1;
        }
    }
}

spla::ExpressionNode::Operation spla::VxM::GetOperationType() const {
    return ExpressionNode::Operation::VxM;
}
