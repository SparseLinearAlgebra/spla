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

#ifndef SPLA_VECTOR_HPP
#define SPLA_VECTOR_HPP

#include <spla/backend.hpp>
#include <spla/config.hpp>
#include <spla/library.hpp>

#include <spla/storage/vector_storage.hpp>

namespace spla {

    /**
     * @class Vector
     * @brief Vector object to represent a mathematical dim M vector with values of specified Type.
     *
     * Uses blocked storage schema internally.
     * Can be used as mask (only indices without values) if Type has zero no values.
     * Can be updated from the host using data write expression node.
     * Vector content can be accessed from host using data read expression node.
     *
     * @details
     *  Uses explicit values storage schema, so actual values of the vector has
     *  mathematical type `Maybe Type`, where non-zero values stored as is (`Just Value`),
     *  and null values are not stored (`Nothing`). In expressions actual operations
     *  are applied only to values `Just Value`. If provided binary function, it
     *  is applied only if both of arguments are `Just Arg1` and `Just Arg2`.
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class Vector {
    public:
        explicit Vector(std::size_t nrows) : m_storage(new detail::VectorStorage<T>(nrows)) {}

        /** Clear vector (makes vector empty) */
        void clear() { storage()->clear(); }

        /** @return True if vector empty (has no values inside) */
        [[nodiscard]] bool empty() const { return nvals() == 0; }

        /** @return Storage size */
        [[nodiscard]] std::size_t nrows() const { return m_storage->nrows(); }

        /** @return Storage number of non-zero values */
        [[nodiscard]] std::size_t nvals() const { return m_storage->nvals(); }

        /** @return Backend vector storage */
        [[nodiscard]] const detail::Ref<detail::VectorStorage<T>> &storage() { return m_storage; }

    private:
        detail::Ref<detail::VectorStorage<T>> m_storage;
    };

}// namespace spla

#endif//SPLA_VECTOR_HPP
