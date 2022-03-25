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

#ifndef SPLA_SORT_HPP
#define SPLA_SORT_HPP

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include <spla/types.hpp>

namespace spla::backend {

    /**
     * @addtogroup shared
     * @{
     */

    template<typename T>
    inline void sort(std::vector<Index> &rows,
                     std::vector<T> &values) {
        assert(rows.size() == values.size() || (!type_has_values<T>() && values.empty()));

        typedef std::pair<Index, T> Entry;

        auto count = rows.size();

        if constexpr (type_has_values<T>()) {
            std::vector<Entry> entries;
            entries.reserve(count);

            for (std::size_t i = 0; i < count; i++)
                entries.emplace_back(rows[i], values[i]);

            std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
                return a.first < b.first;
            });

            for (std::size_t i = 0; i < count; i++) {
                const auto &entry = entries[i];
                rows[i] = entry.first;
                values[i] = entry.second;
            }
        } else
            std::sort(rows.begin(), rows.end(), std::less<>());
    }

    template<typename T>
    inline void sort(std::vector<Index> &rows,
                     std::vector<Index> &cols,
                     std::vector<T> &values) {
        assert(rows.size() == cols.size());
        assert(rows.size() == values.size() || (!type_has_values<T>() && values.empty()));

        typedef std::pair<Index, Index> Idx;
        typedef std::pair<Idx, T> Entry;

        auto count = rows.size();

        if constexpr (type_has_values<T>()) {
            std::vector<Entry> entries;
            entries.reserve(count);

            for (std::size_t i = 0; i < count; i++)
                entries.push_back(Entry(Idx(rows[i], cols[i]), values[i]));

            std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
                const auto &a_idx = a.first;
                const auto &b_idx = b.first;
                return a_idx.first < b_idx.first || (a_idx.first == b_idx.first && a_idx.second < b_idx.second);
            });

            for (std::size_t i = 0; i < count; i++) {
                const auto &entry = entries[i];
                const auto &idx = entry.first;
                rows[i] = idx.first;
                cols[i] = idx.second;
                values[i] = entry.second;
            }
        } else {
            std::vector<Idx> entries;
            entries.reserve(count);

            for (std::size_t i = 0; i < count; i++)
                entries.emplace_back(rows[i], cols[i]);

            std::sort(entries.begin(), entries.end(), [](const auto &a_idx, const auto &b_idx) {
                return a_idx.first < b_idx.first || (a_idx.first == b_idx.first && a_idx.second < b_idx.second);
            });

            for (std::size_t i = 0; i < count; i++) {
                const auto &idx = entries[i];
                rows[i] = idx.first;
                cols[i] = idx.second;
            }
        }
    }

    /**
     * @}
     */

}// namespace spla::backend

#endif//SPLA_SORT_HPP
