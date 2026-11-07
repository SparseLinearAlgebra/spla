/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_PATH_INDEX_HPP
#define SPLA_PATH_INDEX_HPP

#include <algorithm>
#include <cstdint>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @brief Path index for single-path context-free path querying.
     *
     * `middle` is the intermediate vertex of the concatenated path and `height`
     * is the height of the derivation tree. The additive zero (bottom) is
     * `{middle = 0, height = 0}`.
     */
    struct PathIndex {
        std::uint32_t middle;
        std::uint32_t height;

        PathIndex() : middle(0), height(0) {}
        PathIndex(std::uint32_t m, std::uint32_t h) : middle(m), height(h) {}

        bool operator==(const PathIndex& other) const {
            return middle == other.middle && height == other.height;
        }
        bool operator!=(const PathIndex& other) const {
            return !(*this == other);
        }
    };

    /**
     * @brief Additive semiring operation (min by height, tie by middle).
     *
     * The bottom element `{0, 0}` is skipped.
     */
    inline PathIndex path_index_add(const PathIndex& a, const PathIndex& b) {
        const bool a_bottom = a.middle == 0 && a.height == 0;
        const bool b_bottom = b.middle == 0 && b.height == 0;

        if (a_bottom) return b;
        if (b_bottom) return a;
        if (a.height != b.height) return a.height < b.height ? a : b;
        return a.middle <= b.middle ? a : b;
    }

    /**
     * @brief Multiplicative semiring operation (concatenation through `k`).
     *
     * `middle = k`, `height = max(a.height, b.height) + 1`; bottom propagates.
     */
    inline PathIndex path_index_multiply(const PathIndex& a, std::uint32_t k, const PathIndex& b) {
        const bool a_bottom = a.middle == 0 && a.height == 0;
        const bool b_bottom = b.middle == 0 && b.height == 0;

        if (a_bottom || b_bottom) return PathIndex(0, 0);
        return PathIndex(k, std::max(a.height, b.height) + 1);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_PATH_INDEX_HPP
