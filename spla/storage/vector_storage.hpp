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

#ifndef SPLA_VECTOR_STORAGE_HPP
#define SPLA_VECTOR_STORAGE_HPP

#include <cassert>
#include <cstddef>
#include <mutex>

#include <spla/detail/ref.hpp>
#include <spla/storage/vector_block.hpp>

namespace spla {

    /**
     * @class VectorStorage
     * @brief Storage for vector data
     *
     * @tparam T Type of values
     */
    template<typename T>
    class VectorStorage : public RefCnt {
    public:
        /** @brief Active storage schema of the storage */
        enum class Schema {
            /** Sparse blocks are used */
            Sparse,
            /** Dense blocks are used */
            Dense
        };

        explicit VectorStorage(std::size_t nrows) : m_nrows(nrows) {
            assert(nrows > 0 && "Vector must have non-zero number of rows");
        }

        ~VectorStorage() override = default;

        /** @return Storage size */
        std::size_t get_nrows() const { return m_nrows; }

        /** @return Storage number of non-zero values */
        std::size_t get_nvals() const {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            return m_nvals;
        }

    protected:
        std::size_t m_nrows;
        std::size_t m_nvals = 0;

        mutable std::mutex m_mutex;
    };

}// namespace spla

#endif//SPLA_VECTOR_STORAGE_HPP
