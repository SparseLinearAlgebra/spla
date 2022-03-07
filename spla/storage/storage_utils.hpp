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

#ifndef SPLA_STORAGE_UTILS_HPP
#define SPLA_STORAGE_UTILS_HPP

#include <cmath>
#include <cstddef>
#include <utility>

namespace spla::storage {

    /**
     * @addtogroup internal
     * @{
     */

    inline std::size_t block_size(std::size_t rows, std::size_t factor, std::size_t device_count) {
        std::size_t split = factor * device_count;
        return rows <= 4 * split || split == 1 ? rows : (rows - (rows % split)) / (split - 1);
    }

    inline std::size_t block_size(std::size_t rows, std::size_t cols, std::size_t factor, std::size_t device_count) {
        return std::max(block_size(rows, factor, device_count), block_size(cols, factor, device_count));
    }

    inline std::size_t block_count(std::size_t rows, std::size_t block_size) {
        return rows / block_size + (rows % block_size ? 1 : 0);
    }

    inline std::pair<std::size_t, std::size_t> block_count(std::size_t rows, std::size_t cols, std::size_t block_size) {
        return {block_count(rows, block_size), block_count(cols, block_size)};
    }

    inline std::size_t block_size_at(std::size_t rows, std::size_t block_size, std::size_t i) {
        std::size_t count = block_count(rows, block_size);
        return i + 1 < count ? block_size : rows - (count - 1) * block_size;
    }

    inline std::pair<std::size_t, std::size_t> block_size_at(std::size_t rows, std::size_t cols, std::size_t block_size, std::size_t i, std::size_t j) {
        return {block_size_at(rows, block_size, i), block_size_at(cols, block_size, j)};
    }

    inline std::size_t block_offset(std::size_t block_size, std::size_t i) {
        return block_size * i;
    }

    /**
     * @}
     */

}// namespace spla::storage

#endif//SPLA_STORAGE_UTILS_HPP
