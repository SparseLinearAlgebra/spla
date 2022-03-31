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

#ifndef SPLA_TEST_UTILS_MATRIX_HPP
#define SPLA_TEST_UTILS_MATRIX_HPP

#include <algorithm>
#include <random>
#include <utility>
#include <vector>


#include <spla/spla.hpp>

namespace testing {

    template<typename T>
    struct Matrix {
        using Index = spla::Index;

        std::size_t nrows{};
        std::size_t ncols{};
        std::vector<Index> rows;
        std::vector<Index> cols;
        std::vector<T> values;

        [[nodiscard]] std::size_t nvals() const { return rows.size(); }

        Matrix sort_reduce_duplicates() const {
            using Pair = std::pair<Index, Index>;
            using Tuple = std::pair<Pair, std::size_t>;

            std::vector<Tuple> indices;
            indices.reserve(nvals());

            for (size_t i = 0; i < nvals(); i++) {
                auto row = rows[i];
                auto col = cols[i];
                indices.emplace_back(Pair{row, col}, i);
            }

            std::sort(indices.begin(), indices.end(), [](const Tuple &t1, const Tuple &t2) {
                const auto &a = t1.first;
                const auto &b = t2.first;
                return a.first < b.first ||
                       (a.first == b.first && a.second < b.second) ||
                       (a == b && t1.second < t2.second);
            });

            indices.erase(std::unique(indices.begin(), indices.end(), [](const Tuple &a, const Tuple &b) { return a.first == b.first; }),
                          indices.end());

            std::vector<Index> new_rows;
            std::vector<Index> new_cols;
            std::vector<T> new_vals;

            new_rows.reserve(indices.size());
            new_cols.reserve(indices.size());
            new_vals.reserve(indices.size());

            for (const auto &index : indices) {
                auto i = index.second;
                new_rows.push_back(rows[i]);
                new_cols.push_back(cols[i]);
                new_vals.push_back(values[i]);
            }

            return Matrix<T>(nrows, ncols, std::move(new_rows), std::move(new_cols), std::move(new_vals));
        }

        static Matrix generate(std::size_t M, std::size_t N, std::size_t nvals, std::size_t seed = 0, const T &value = T()) {
            std::vector<spla::Index> rows;
            std::vector<spla::Index> cols;
            std::vector<T> values(nvals, value);

            rows.reserve(nvals);
            cols.reserve(nvals);

            std::default_random_engine engine(seed);
            auto distRows = std::uniform_int_distribution<Index>(0, M > 0 ? M - 1 : M);
            auto distCols = std::uniform_int_distribution<Index>(0, N > 0 ? N - 1 : N);

            for (size_t i = 0; i < nvals; i++) {
                auto row = distRows(engine);
                auto col = distCols(engine);
                assert(row < M);
                assert(col < N);
                rows.push_back(row);
                cols.push_back(col);
            }

            return Matrix<T>{M, N, std::move(rows), std::move(cols), std::move(values)};
        }
    };

}// namespace testing

#endif//SPLA_TEST_UTILS_MATRIX_HPP
