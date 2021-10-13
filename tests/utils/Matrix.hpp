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

#ifndef SPLA_MATRIX_HPP
#define SPLA_MATRIX_HPP

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>
#include <spla-cpp/Spla.hpp>
#include <utility>
#include <vector>

namespace utils {

    template<typename T>
    class Matrix {
    public:
        using Index = unsigned int;
        static constexpr std::size_t IndexSize = sizeof(Index);
        static constexpr std::size_t ElementSize = sizeof(T);

        Matrix(size_t nrows, size_t ncols, std::vector<Index> rows, std::vector<Index> cols, std::vector<T> vals)
            : mRows(std::move(rows)),
              mCols(std::move(cols)),
              mVals(std::move(vals)),
              mNrows(nrows),
              mNcols(ncols) {
            assert(nrows > 0);
            assert(ncols > 0);
            assert(mRows.size() == mCols.size());
            assert(mCols.size() == mVals.size());
        }

        template<typename Gen>
        void Fill(Gen generator) {
            for (T &value : mVals) {
                value = generator();
            }
        }

        [[nodiscard]] T *GetVals() {
            return mVals.data();
        }

        [[nodiscard]] Index *GetRows() {
            return mRows.data();
        }

        [[nodiscard]] Index *GetCols() {
            return mCols.data();
        }

        [[nodiscard]] const T *GetVals() const {
            return mVals.data();
        }

        [[nodiscard]] const Index *GetRows() const {
            return mRows.data();
        }

        [[nodiscard]] const Index *GetCols() const {
            return mCols.data();
        }

        [[nodiscard]] std::size_t GetNrows() const {
            return mNrows;
        }

        [[nodiscard]] std::size_t GetNcols() const {
            return mNcols;
        }

        [[nodiscard]] std::size_t GetNvals() const {
            return mVals.size();
        }

        [[nodiscard]] bool Equals(const spla::RefPtr<spla::Matrix> &m) const {
            if (m->GetNrows() != GetNrows() || m->GetNcols() != GetNcols()) {
                std::cout << "Size mismatched" << std::endl;
                return false;
            }

            if (m->GetNvals() != GetNvals()) {
                std::cout << "Number of nnz mismatched" << std::endl;
                return false;
            }

            if (ElementSize != m->GetType()->GetByteSize()) {
                std::cout << "Type has incompatible size" << std::endl;
                return false;
            }

            std::vector<Index> spRows(GetNvals());
            std::vector<Index> spCols(GetNvals());
            std::vector<T> spVals(GetNvals());

            auto &library = m->GetLibrary();
            auto data = spla::DataMatrix::Make(library);
            data->SetRows(spRows.data());
            data->SetCols(spCols.data());
            data->SetVals(spVals.data());
            data->SetNvals(GetNvals());

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(m, data);
            library.Submit(readExpr);

            readExpr->Wait();

            if (readExpr->GetState() != spla::Expression::State::Evaluated) {
                std::cout << "Read expression is not evaluated" << std::endl;
                return false;
            }

            if (std::memcmp(GetRows(), spRows.data(), GetNvals() * IndexSize) != 0) {
                std::cout << "Row indices not equal" << std::endl;
                return false;
            }

            if (std::memcmp(GetCols(), spCols.data(), GetNvals() * IndexSize) != 0) {
                std::cout << "Column indices not equal" << std::endl;
                return false;
            }

            if (std::memcmp(mVals.data(), spVals.data(), GetNvals() * ElementSize) != 0) {
                std::cout << "Values not equal" << std::endl;
                return false;
            }

            return true;
        }

        [[nodiscard]] Matrix SortReduceDuplicates() const {
            using Pair = std::pair<Index, Index>;
            using Tuple = std::pair<Pair, size_t>;

            std::vector<Tuple> indices;
            indices.reserve(GetNvals());

            for (size_t i = 0; i < GetNvals(); i++) {
                auto row = mRows[i];
                auto col = mCols[i];
                indices.emplace_back(Pair{row, col}, i);
            }

            std::sort(indices.begin(), indices.end(), [](const Tuple &t1, const Tuple &t2) {
                const auto &a = t1.first;
                const auto &b = t2.first;
                return a.first < b.first ||
                       (a.first == b.first && a.second < b.second) ||
                       (a == b && t1.second < t2.second);
            });

            indices.erase(std::unique(indices.begin(), indices.end(), [](const Tuple &a, const Tuple &b) {
                              return a.first == b.first;
                          }),
                          indices.end());

            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<T> vals;

            rows.reserve(indices.size());
            cols.reserve(indices.size());
            vals.reserve(indices.size());

            for (const auto &index : indices) {
                auto i = index.second;
                rows.push_back(GetRows()[i]);
                cols.push_back(GetCols()[i]);
                vals.push_back(GetVals()[i]);
            }

            return Matrix<T>(GetNrows(), GetNcols(), std::move(rows), std::move(cols), std::move(vals));
        }

        static Matrix Generate(size_t nrows, size_t ncols, size_t nvals, size_t seed = 0, const T &value = T()) {
            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<T> vals(nvals, value);

            rows.reserve(nvals);
            cols.reserve(nvals);

            std::default_random_engine engine(seed);
            auto distRows = std::uniform_int_distribution<Index>(0, nrows > 0 ? nrows - 1 : nrows);
            auto distCols = std::uniform_int_distribution<Index>(0, ncols > 0 ? ncols - 1 : ncols);

            for (size_t i = 0; i < nvals; i++) {
                auto row = distRows(engine);
                auto col = distCols(engine);
                assert(row < nrows);
                assert(col < ncols);
                rows.push_back(row);
                cols.push_back(col);
            }

            return Matrix<T>(nrows, ncols, std::move(rows), std::move(cols), std::move(vals));
        }

    private:
        std::vector<Index> mRows;
        std::vector<Index> mCols;
        std::vector<T> mVals;
        size_t mNrows = 0;
        size_t mNcols = 0;
    };

}// namespace utils

#endif//SPLA_MATRIX_HPP
