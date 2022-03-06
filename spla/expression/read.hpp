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

#ifndef SPLA_READ_HPP
#define SPLA_READ_HPP

#include <algorithm>
#include <vector>

#include <spla/backend.hpp>
#include <spla/descriptor.hpp>
#include <spla/types.hpp>

#include <spla/expression/expression_node.hpp>
#include <spla/storage/storage_lock.hpp>
#include <spla/storage/storage_utils.hpp>

namespace spla::expression {

    template<typename T, typename Callback>
    class ReadVector final : public ExpressionNode {
    public:
        ReadVector(Vector<T> vector, Callback callback, const Descriptor &desc, std::size_t id, Expression &expression)
            : ExpressionNode(desc, id, expression), m_vector(std::move(vector)), m_callback(std::move(callback)) {}

        ~ReadVector() override = default;

        std::string type() const override {
            return "read-vector";
        }

    private:
        void prepare() override {}
        void finalize() override {}
        void execute(SubtaskBuilder &builder) override {
            auto storage = m_vector.storage();
            auto nvals = storage->nvals();
            auto nblocks = storage->block_count_rows();
            auto host_rows = std::make_shared<std::vector<Index>>(nvals);
            auto host_values = std::make_shared<std::vector<T>>(type_has_values<T>() ? nvals : 0);

            std::vector<std::size_t> offsets(nblocks);
            auto blocks = storage->blocks_sparse();
            for (std::size_t i = 0; i < nblocks; i++) offsets[i] = blocks[i]->nvals();
            std::exclusive_scan(offsets.begin(), offsets.end(), offsets.begin(), std::size_t{0});

            auto notify = builder.emplace("read-notify", [host_rows, host_values, this]() { m_callback(*host_rows, *host_values); });
            for (std::size_t i = 0; i < storage->block_count_rows(); i++) {
                builder.emplace("read-block", [storage, host_rows, host_values, i, offsets, this]() {
                           auto blockSize = storage->block_size();
                           backend::ReadParams readParams{storage::block_offset(blockSize, i), offsets[i]};
                           backend::read(storage->block_sparse(i), *host_rows, *host_values, desc(), readParams, i);
                       })
                        .precede(notify);
            }
        }

    private:
        Vector<T> m_vector;
        Callback m_callback;
    };

}// namespace spla::expression

#endif//SPLA_READ_HPP
