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
#include <random>
#include <spla-cpp/Spla.hpp>
#include <utility>
#include <vector>

namespace utils {

    class Matrix {
    public:
        std::vector<unsigned int> rows;
        std::vector<unsigned int> cols;
        std::vector<char> vals;

        size_t nrows = 0;
        size_t ncols = 0;
        size_t nvals = 0;
        size_t elementSize = 0;

        void FillFloatData(size_t seed = 0) {
            assert(elementSize == 0);
            assert(vals.empty());

            elementSize = sizeof(float);
            vals.resize(nvals * elementSize);

            std::default_random_engine engine(seed);
            auto dist = std::uniform_real_distribution<float>();

            size_t offset = 0;
            for (size_t k = 0; k < nvals; k++) {
                auto value = dist(engine);
                std::memcpy(vals.data() + offset, &value, elementSize);
                offset += elementSize;
            }
        }

        [[nodiscard]] bool Equals(const spla::RefPtr<spla::Matrix> &m) const {
            if (m->GetNrows() != nrows || m->GetNcols() != ncols || m->GetNvals() != nvals)
                return false;

            auto type = m->GetType();
            auto size = type->GetByteSize();

            if (elementSize != size)
                return false;

            std::vector<unsigned int> spRows(nvals);
            std::vector<unsigned int> spCols(nvals);
            std::vector<char> spVals(nvals * elementSize);

            auto &library = m->GetLibrary();
            auto data = spla::DataMatrix::Make(library);
            data->SetRows(spRows.data());
            data->SetCols(spCols.data());
            data->SetVals(spVals.data());
            data->SetNvals(nvals);

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(m, data);
            library.Submit(readExpr);

            if (readExpr->GetState() != spla::Expression::State::Evaluated)
                return false;

            if (!std::memcmp(rows.data(), spRows.data(), nvals * sizeof(unsigned int)))
                return false;
            if (!std::memcmp(cols.data(), spCols.data(), nvals * sizeof(unsigned int)))
                return false;
            if (elementSize && !std::memcmp(vals.data(), spVals.data(), nvals * elementSize))
                return false;

            return true;
        }

        [[nodiscard]] Matrix SortReduceDuplicates() const {
            using Pair = std::pair<unsigned int, unsigned int>;
            std::vector<Pair> indices;
            indices.reserve(nvals);

            for (size_t i = 0; i < nvals; i++) {
                auto row = rows[i];
                auto col = cols[i];
                indices.emplace_back(row, col);
            }

            std::sort(indices.begin(), indices.end(), [](const Pair &a, const Pair &b) {
                return a.first < b.first || (a.first == b.first && a.second < b.second);
            });

            std::vector<size_t> toCopy;

            Pair prev{static_cast<unsigned int>(nrows), static_cast<unsigned int>(ncols)};
            for (size_t i = 0; i < indices.size(); i++) {
                auto &p = indices[i];

                if (p != prev) {
                    toCopy.push_back(i);
                    prev = p;
                }
            }

            Matrix r;
            r.nrows = nrows;
            r.ncols = ncols;
            r.nvals = toCopy.size();

            r.rows.reserve(r.nvals);
            r.cols.reserve(r.nvals);

            for (auto idx : toCopy) {
                auto &p = indices[idx];
                r.rows.push_back(p.first);
                r.rows.push_back(p.second);
            }

            if (!vals.empty()) {
                assert(elementSize * nvals == vals.size());

                r.elementSize = elementSize;
                r.vals.resize(r.nvals * r.elementSize);

                size_t offset = 0;

                for (auto idx : toCopy) {
                    std::memcpy(r.vals.data() + offset, vals.data() + elementSize * idx, elementSize);
                    offset += elementSize;
                }
            }

            return r;
        }

        static Matrix Generate(size_t nrows, size_t ncols, size_t nvals, size_t seed = 0) {
            Matrix m;
            m.nrows = nrows;
            m.ncols = ncols;

            std::default_random_engine engine(seed);
            auto distRows = std::uniform_int_distribution<unsigned int>(0, nrows);
            auto distCols = std::uniform_int_distribution<unsigned int>(0, ncols);

            for (size_t i = 0; i < nvals; i++) {
                auto row = distRows(engine);
                auto col = distCols(engine);

                if (row < nrows && col < ncols) {
                    m.rows.push_back(row);
                    m.cols.push_back(col);
                    m.nvals += 1;
                }
            }

            return m;
        }

        static Matrix GenerateStDp(size_t nrows, size_t ncols, size_t nvals, size_t seed = 0) {
            Matrix m = Generate(nrows, ncols, nvals, seed);
            return m.SortReduceDuplicates();
        }
    };

}// namespace utils

#endif//SPLA_MATRIX_HPP
