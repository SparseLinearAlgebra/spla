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

#ifndef SPLA_BACKEND_UTILS_HPP
#define SPLA_BACKEND_UTILS_HPP

#include <memory>

#include <spla/backend.hpp>

namespace spla::detail {

    /**
     * @addtogroup internal
     * @{
     */

    template<typename T>
    std::shared_ptr<typename backend::VectorStorage<T>::BlocksSparse> make_vector_blocks_sparse(const detail::Ref<backend::VectorStorage<T>> &storage) {
        return std::make_shared<typename backend::VectorStorage<T>::BlocksSparse>(storage->block_count_rows());
    }

    template<typename T>
    std::shared_ptr<typename backend::VectorStorage<T>::BlocksDense> make_vector_blocks_dense(const detail::Ref<backend::VectorStorage<T>> &storage) {
        return std::make_shared<typename backend::VectorStorage<T>::BlocksDense>(storage->block_count_rows());
    }

    /**
     * @}
     */

}// namespace spla::detail

#endif//SPLA_BACKEND_UTILS_HPP
