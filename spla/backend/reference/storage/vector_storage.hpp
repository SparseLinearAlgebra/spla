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

#ifndef SPLA_REFERENCE_VECTOR_STORAGE_HPP
#define SPLA_REFERENCE_VECTOR_STORAGE_HPP

#include <cassert>
#include <cstddef>
#include <mutex>

#include <spla/backend/reference/storage/vector_coo.hpp>
#include <spla/backend/reference/storage/vector_dense.hpp>
#include <spla/storage/storage_schema.hpp>

namespace spla::backend {

    template<typename T>
    class VectorStorage final : public RefCnt {
    public:
        explicit VectorStorage(std::size_t nrows) : m_nrows(nrows) {}

        ~VectorStorage() override = default;

        std::size_t nrows() const { return m_nrows; }

        std::size_t nvals() const {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            return m_nvals;
        }

    private:
        std::size_t m_nrows;
        std::size_t m_nvals = 0;

        std::unordered_map<std::size_t, Ref<VectorBlock<T>>> m_sparse;
        std::unordered_map<std::size_t, Ref<VectorBlock<T>>> m_dense;

        mutable std::mutex m_mutex;
    };

}// namespace spla::backend

#endif//SPLA_REFERENCE_VECTOR_STORAGE_HPP
