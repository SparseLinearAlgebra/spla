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

#ifndef SPLA_ROWS_PTR_HPP
#define SPLA_ROWS_PTR_HPP

#include <numeric>
#include <vector>

#include <spla/types.hpp>

namespace spla::backend {

    /**
     * @addtogroup shared
     * @{
     */

    inline void rows_offsets(std::size_t nrows, const std::vector<Index> &rows, std::vector<Index> &rows_ptr) {
        std::vector<Index> lengths(nrows + 1, 0);
        std::vector<Index> offsets(nrows + 1);

        for (auto i : rows) lengths[i] += 1;
        std::exclusive_scan(lengths.begin(), lengths.end(), offsets.begin(), Index{0});

        std::swap(rows_ptr, offsets);
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_ROWS_PTR_HPP
