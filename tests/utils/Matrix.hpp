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
#include <numeric>
#include <random>
#include <spla-algo/SplaAlgo.hpp>
#include <spla-cpp/Spla.hpp>
#include <utility>
#include <utils/Compute.hpp>
#include <utils/Typetraits.hpp>
#include <vector>

namespace utils {

    template<typename T>
    class Matrix {
    public:
        using Index = unsigned int;
        static constexpr std::size_t IndexSize = sizeof(Index);
        static constexpr std::size_t ElementSize = sizeof(T);

        Matrix(std::size_t nrows, std::size_t ncols, std::vector<Index> rows, std::vector<Index> cols, std::vector<T> vals)
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

        [[nodiscard]] const std::vector<T> &GetValsVec() const {
            return mVals;
        }

        [[nodiscard]] const std::vector<Index> &GetRowsVec() const {
            return mRows;
        }

        [[nodiscard]] const std::vector<Index> &GetColsVec() const {
            return mCols;
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

        [[nodiscard]] bool EqualsStructure(const spla::RefPtr<spla::Matrix> &m) const {
            if (m->GetNrows() != GetNrows() || m->GetNcols() != GetNcols()) {
                std::cout << "Size mismatched" << std::endl;
                return false;
            }

            if (m->GetNvals() != GetNvals()) {
                std::cout << "Number of nnz mismatched "
                          << "expected=" << GetNvals() << " "
                          << "actual=" << m->GetNvals() << std::endl;
                return false;
            }

            std::vector<Index> spRows(GetNvals());
            std::vector<Index> spCols(GetNvals());

            auto &library = m->GetLibrary();
            auto data = spla::DataMatrix::Make(library);
            data->SetRows(spRows.data());
            data->SetCols(spCols.data());
            data->SetNvals(GetNvals());

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(m, data);
            readExpr->Submit();
            readExpr->Wait();

            if (readExpr->GetState() != spla::Expression::State::Evaluated) {
                std::cout << "Read expression is not evaluated" << std::endl;
                return false;
            }

            if (std::memcmp(GetRows(), spRows.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetRows()[i] << " actual=" << spRows[i] << " " << (GetRows()[i] == spRows[i] ? "eq" : "neq") << std::endl;

                Dump(m);

                std::cout << "Row indices not equal" << std::endl;
                return false;
            }

            if (std::memcmp(GetCols(), spCols.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetCols()[i] << " actual=" << spCols[i] << " " << (GetCols()[i] == spCols[i] ? "eq" : "neq") << std::endl;

                Dump(m);

                std::cout << "Column indices not equal" << std::endl;
                return false;
            }

            return true;
        }

        [[nodiscard]] bool Equals(const spla::RefPtr<spla::Matrix> &m) const {
            if (m->GetNrows() != GetNrows() || m->GetNcols() != GetNcols()) {
                std::cout << "Size mismatched" << std::endl;
                return false;
            }

            if (m->GetNvals() != GetNvals()) {
                std::cout << "Number of nnz mismatched "
                          << "expected=" << GetNvals() << " "
                          << "actual=" << m->GetNvals() << std::endl;
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
            readExpr->Submit();
            readExpr->Wait();

            if (readExpr->GetState() != spla::Expression::State::Evaluated) {
                std::cout << "Read expression is not evaluated" << std::endl;
                return false;
            }

            if (std::memcmp(GetRows(), spRows.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetRows()[i] << " actual=" << spRows[i] << " " << (GetRows()[i] == spRows[i] ? "eq" : "neq") << std::endl;

                Dump(m);

                std::cout << "Row indices not equal" << std::endl;
                return false;
            }

            if (std::memcmp(GetCols(), spCols.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetCols()[i] << " actual=" << spCols[i] << " " << (GetCols()[i] == spCols[i] ? "eq" : "neq") << std::endl;

                Dump(m);

                std::cout << "Column indices not equal" << std::endl;
                return false;
            }

            for (std::size_t i = 0; i < GetNvals(); i++) {
                auto a = GetVals()[i];
                auto b = spVals[i];

                if (!utils::EqWithError(a, b)) {
                    return false;
                }
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

        [[nodiscard]] Matrix RemoveLoops() const {
            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<T> vals;

            for (std::size_t k = 0; k < GetNvals(); k++) {
                if (GetRows()[k] != GetCols()[k]) {
                    rows.push_back(GetRows()[k]);
                    cols.push_back(GetCols()[k]);
                    vals.push_back(GetVals()[k]);
                }
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

        [[nodiscard]] spla::RefPtr<spla::DataMatrix> GetDataIndices(spla::Library &library) {
            auto data = spla::DataMatrix::Make(library);
            data->SetRows(GetRows());
            data->SetCols(GetCols());
            data->SetNvals(GetNvals());
            return data;
        }

        [[nodiscard]] spla::RefPtr<spla::DataMatrix> GetData(spla::Library &library) {
            auto data = spla::DataMatrix::Make(library);
            data->SetRows(GetRows());
            data->SetCols(GetCols());
            data->SetVals(GetVals());
            data->SetNvals(GetNvals());
            return data;
        }

        template<typename M>
        [[nodiscard]] Matrix Mask(const Matrix<M> &mask, bool complement) const {
            assert(GetNrows() == mask.GetNrows());
            assert(GetNcols() == mask.GetNcols());

            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<T> vals;

            std::size_t a = 0;
            std::size_t b = 0;
            std::size_t endA = GetNvals();
            std::size_t endB = mask.GetNvals();

            if (complement) {
                while (a != endA && b != endB) {
                    auto thisCrd = std::pair{GetRows()[a], GetCols()[a]};
                    auto maskCrd = std::pair{mask.GetRows()[b], mask.GetCols()[b]};

                    if (thisCrd == maskCrd) {
                        a += 1;
                        b += 1;
                    } else if (thisCrd < maskCrd) {
                        rows.push_back(GetRows()[a]);
                        cols.push_back(GetCols()[a]);
                        vals.push_back(GetVals()[a]);
                        a += 1;
                    } else {
                        b += 1;
                    }
                }

                while (a != endA) {
                    rows.push_back(GetRows()[a]);
                    cols.push_back(GetCols()[a]);
                    vals.push_back(GetVals()[a]);
                    a += 1;
                }
            } else {
                while (a != endA && b != endB) {
                    auto thisCrd = std::pair{GetRows()[a], GetCols()[a]};
                    auto maskCrd = std::pair{mask.GetRows()[b], mask.GetCols()[b]};

                    if (thisCrd == maskCrd) {
                        rows.push_back(GetRows()[a]);
                        cols.push_back(GetCols()[a]);
                        vals.push_back(GetVals()[a]);
                        a += 1;
                        b += 1;
                    } else if (thisCrd < maskCrd) {
                        a += 1;
                    } else {
                        b += 1;
                    }
                }
            }

            return Matrix<T>(GetNrows(), GetNcols(), std::move(rows), std::move(cols), std::move(vals));
        }

        [[nodiscard]] bool IsEmpty() const {
            return GetNvals() == 0;
        }

        template<typename BinaryOp>
        [[nodiscard]] Matrix EWiseAdd(const Matrix<T> &other, BinaryOp op) const {
            assert(GetNrows() == other.GetNrows());
            assert(GetNcols() == other.GetNcols());

            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<T> vals;

            std::size_t a = 0;
            std::size_t b = 0;
            std::size_t endA = GetNvals();
            std::size_t endB = other.GetNvals();

            while (a != endA && b != endB) {
                auto thisCrd = std::pair{GetRows()[a], GetCols()[a]};
                auto otherCrd = std::pair{other.GetRows()[b], other.GetCols()[b]};

                if (thisCrd == otherCrd) {
                    rows.push_back(GetRows()[a]);
                    cols.push_back(GetCols()[a]);
                    vals.push_back(op(GetVals()[a], other.GetVals()[b]));
                    a += 1;
                    b += 1;
                } else if (thisCrd < otherCrd) {
                    rows.push_back(GetRows()[a]);
                    cols.push_back(GetCols()[a]);
                    vals.push_back(GetVals()[a]);
                    a += 1;
                } else {
                    rows.push_back(other.GetRows()[b]);
                    cols.push_back(other.GetCols()[b]);
                    vals.push_back(other.GetVals()[b]);
                    b += 1;
                }
            }

            while (a != endA) {
                rows.push_back(GetRows()[a]);
                vals.push_back(GetVals()[a]);
                cols.push_back(GetCols()[a]);
                a += 1;
            }

            while (b != endB) {
                rows.push_back(other.GetRows()[b]);
                vals.push_back(other.GetVals()[b]);
                cols.push_back(other.GetCols()[b]);
                b += 1;
            }

            return Matrix<T>(GetNrows(), GetNcols(), std::move(rows), std::move(cols), std::move(vals));
        }

        template<typename M, typename BinaryOp>
        [[nodiscard]] Matrix EWiseAdd(const Matrix<M> &mask, bool complement, const Matrix<T> &other, BinaryOp op) const {
            return Mask(mask, complement).EWiseAdd(other.Mask(mask, complement), op);
        }

        template<typename W, typename B, typename MultOp, typename AddOp>
        [[nodiscard]] Matrix<W> MxM(const Matrix<B> &b, MultOp multOp, AddOp addOp) {
            assert(GetNcols() == b.GetNrows());

            std::vector<Index> rows;
            std::vector<Index> cols;
            std::vector<W> vals;

            auto aOffsets = ToOffsets(GetNrows(), mRows);
            auto bOffsets = ToOffsets(b.GetNrows(), b.mRows);

            for (std::size_t i = 0; i < GetNrows(); i++) {
                std::vector<Index> nnz;
                std::vector<Index> mask(b.GetNcols(), i + 1);
                std::vector<W> tmpResult(b.GetNcols());

                for (std::size_t ak = aOffsets[i]; ak < aOffsets[i + 1]; ak++) {
                    auto aCol = mCols[ak];
                    auto aVal = mVals[ak];

                    for (std::size_t bk = bOffsets[aCol]; bk < bOffsets[aCol + 1]; bk++) {
                        auto bCol = b.mCols[bk];
                        auto bVal = b.mVals[bk];

                        if (mask[bCol] != i) {
                            mask[bCol] = i;
                            nnz.push_back(bCol);
                            tmpResult[bCol] = multOp(aVal, bVal);
                        } else
                            tmpResult[bCol] = addOp(tmpResult[bCol], multOp(aVal, bVal));
                    }
                }

                if (!nnz.empty()) {
                    std::sort(nnz.begin(), nnz.end());
                    for (auto k : nnz) {
                        rows.push_back(i);
                        cols.push_back(k);
                        vals.push_back(tmpResult[k]);
                    }
                }
            }

            return Matrix<W>(GetNrows(), b.GetNcols(), std::move(rows), std::move(cols), std::move(vals));
        }

        template<typename W, typename M, typename B, typename MultOp, typename AddOp>
        [[nodiscard]] Matrix<W> MxM(const Matrix<M> &mask, bool complement, const Matrix<B> &b, MultOp multOp, AddOp addOp) {
            return MxM<W>(b, multOp, addOp).Mask(mask, complement);
        }

        Matrix<T> Transpose() {
            std::size_t M = GetNrows();
            std::size_t N = GetNcols();
            std::size_t nvals = GetNvals();
            std::vector<Index> rowSizeT(N, 0);

            for (std::size_t k = 0; k < nvals; k++) {
                rowSizeT[mCols[k]] += 1;
            }

            std::vector<Index> rowOffsetT(N, 0);
            std::exclusive_scan(rowSizeT.begin(), rowSizeT.end(), rowOffsetT.begin(), 0);

            std::vector<Index> rowsT(nvals);
            std::vector<Index> colsT(nvals);
            std::vector<T> valsT(nvals);

            for (std::size_t k = 0; k < nvals; k++) {
                auto i = mRows[k];
                auto j = mCols[k];
                auto v = mVals[k];
                auto &offset = rowOffsetT[j];
                rowsT[offset] = j;
                colsT[offset] = i;
                valsT[offset] = v;
                offset += 1;
            }

            return Matrix<T>(N, M, std::move(rowsT), std::move(colsT), std::move(valsT));
        }

        template<typename Selector>
        Matrix<T> Select(Selector selector) const {
            std::size_t M = GetNrows();
            std::size_t N = GetNcols();
            std::size_t nvals = GetNvals();

            std::vector<Index> rowsTria;
            std::vector<Index> colsTria;
            std::vector<T> valsTria;

            for (std::size_t k = 0; k < nvals; k++) {
                if (selector(GetRows()[k], GetCols()[k])) {
                    rowsTria.push_back(GetRows()[k]);
                    colsTria.push_back(GetCols()[k]);
                    valsTria.push_back(GetVals()[k]);
                }
            }

            return Matrix<T>(M, N, std::move(rowsTria), std::move(colsTria), std::move(valsTria));
        }

        Matrix<T> Tril() const {
            return Select([](Index i, Index j) { return i > j; });
        }

        Matrix<T> Triu() const {
            return Select([](Index i, Index j) { return i < j; });
        }

        template<typename ReduceT, typename R = std::invoke_result_t<ReduceT, T, T>>
        [[nodiscard]] R Reduce(ReduceT reduce) {
            if (mVals.empty()) {
                throw std::invalid_argument("Unable to reduce empty matrix");
            }
            R result = mVals[0];
            for (std::size_t i = 1; i < mVals.size(); ++i) {
                result = reduce(result, mVals[i]);
            }
            return result;
        }

        [[nodiscard]] spla::RefPtr<spla::HostMatrix> ToHostMatrix() const {
            std::vector<Index> rows = mRows;
            std::vector<Index> cols = mCols;
            std::vector<unsigned char> data(mVals.size() * sizeof(T));
            std::memcpy(data.data(), mVals.data(), mVals.size() * sizeof(T));
            return spla::RefPtr<spla::HostMatrix>(new spla::HostMatrix(mNrows, mNcols, std::move(rows), std::move(cols), std::move(data)));
        }

        static Matrix FromHostMatrix(const spla::RefPtr<spla::HostMatrix> &m) {
            std::vector<Index> rows = m->GetRowIndices();
            std::vector<Index> cols = m->GetColIndices();
            std::vector<T> values(m->GetNnvals());
            std::memcpy(values.data(), m->GetValues().data(), values.size() * sizeof(T));
            return Matrix(m->GetNrows(), m->GetNcols(), std::move(rows), std::move(cols), std::move(values));
        }

        static Matrix Empty(std::size_t nrows, std::size_t ncols) {
            return Matrix(nrows, ncols, std::vector<Index>{}, std::vector<Index>{}, std::vector<T>{});
        }

        static void Dump(const spla::RefPtr<spla::Matrix> &m) {
            std::stringstream ss;
            m->Dump(ss);
            std::cout << ss.str();
        }

    public:
        std::vector<Index> mRows;
        std::vector<Index> mCols;
        std::vector<T> mVals;
        size_t mNrows = 0;
        size_t mNcols = 0;
    };

    template<typename T>
    std::ostream &operator<<(std::ostream &os, const Matrix<T> &m) {
        os << "  rows: ";
        for (auto x : m.mRows) {
            os << x << ' ';
        }
        os << "\n  cols: ";
        for (auto x : m.mCols) {
            os << x << ' ';
        }
        os << "\n  vals: ";
        for (auto x : m.mVals) {
            os << x << ' ';
        }
        return os;
    }

}// namespace utils

#endif//SPLA_MATRIX_HPP
