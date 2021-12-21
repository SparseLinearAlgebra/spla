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

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>
#include <spla-cpp/Spla.hpp>
#include <unordered_map>
#include <utility>
#include <utils/Typetraits.hpp>
#include <vector>

namespace utils {

    template<typename T>
    class Vector {
    public:
        using Index = unsigned int;
        static constexpr std::size_t IndexSize = sizeof(Index);
        static constexpr std::size_t ElementSize = sizeof(T);

        Vector(std::size_t nrows, std::vector<Index> rows, std::vector<T> vals)
            : mRows(std::move(rows)),
              mVals(std::move(vals)),
              mNrows(nrows) {
            assert(nrows > 0);
            assert(mRows.size() == mVals.size());
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

        [[nodiscard]] const T *GetVals() const {
            return mVals.data();
        }

        [[nodiscard]] const Index *GetRows() const {
            return mRows.data();
        }

        [[nodiscard]] const std::vector<T> &GetValsVec() const {
            return mVals;
        }

        [[nodiscard]] const std::vector<Index> &GetRowsVec() const {
            return mRows;
        }

        [[nodiscard]] std::size_t GetNrows() const {
            return mNrows;
        }

        [[nodiscard]] std::size_t GetNvals() const {
            return mVals.size();
        }

        [[nodiscard]] spla::RefPtr<spla::DataVector> GetData(spla::Library &library) {
            auto data = spla::DataVector::Make(library);
            data->SetRows(GetRows());
            data->SetVals(GetVals());
            data->SetNvals(GetNvals());
            return data;
        }

        [[nodiscard]] spla::RefPtr<spla::DataVector> GetDataIndices(spla::Library &library) {
            auto data = spla::DataVector::Make(library);
            data->SetRows(GetRows());
            data->SetNvals(GetNvals());
            return data;
        }

        [[nodiscard]] bool EqualsStructure(const spla::RefPtr<spla::Vector> &v) const {
            if (v->GetNrows() != GetNrows()) {
                std::cout << "Size mismatched" << std::endl;
                return false;
            }

            if (v->GetNvals() != GetNvals()) {
                std::cout << "Number of nnz mismatched" << std::endl;
                return false;
            }

            std::vector<Index> spRows(GetNvals());

            auto &library = v->GetLibrary();
            auto data = spla::DataVector::Make(library);
            data->SetRows(spRows.data());
            data->SetNvals(GetNvals());

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(v, data);
            readExpr->Submit();
            readExpr->Wait();

            if (readExpr->GetState() != spla::Expression::State::Evaluated) {
                std::cout << "Read expression is not evaluated" << std::endl;
                return false;
            }

            if (std::memcmp(GetRows(), spRows.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetRows()[i] << " actual=" << spRows[i] << " " << (GetRows()[i] == spRows[i] ? "eq" : "neq") << std::endl;

                Dump(v);

                std::cout << "Row indices not equal" << std::endl;
                return false;
            }

            return true;
        }

        [[nodiscard]] bool Equals(const spla::RefPtr<spla::Vector> &v, bool useError = UseError<T>(), T error = GetError<T>()) const {
            if (v->GetNrows() != GetNrows()) {
                std::cout << "Size mismatched" << std::endl;
                return false;
            }

            if (v->GetNvals() != GetNvals()) {
                std::cout << "Number of nnz mismatched "
                          << "expected=" << GetNvals() << " "
                          << "actual=" << v->GetNvals() << std::endl;
                return false;
            }

            if (ElementSize != v->GetType()->GetByteSize()) {
                std::cout << "Type has incompatible size" << std::endl;
                return false;
            }

            std::vector<Index> spRows(GetNvals());
            std::vector<T> spVals(GetNvals());

            auto &library = v->GetLibrary();
            auto data = spla::DataVector::Make(library);
            data->SetRows(spRows.data());
            data->SetVals(spVals.data());
            data->SetNvals(GetNvals());

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(v, data);
            readExpr->Submit();
            readExpr->Wait();

            if (readExpr->GetState() != spla::Expression::State::Evaluated) {
                std::cout << "Read expression is not evaluated" << std::endl;
                return false;
            }

            if (std::memcmp(GetRows(), spRows.data(), GetNvals() * IndexSize) != 0) {
                for (std::size_t i = 0; i < GetNvals(); i++)
                    std::cout << "expected=" << GetRows()[i] << " actual=" << spRows[i] << " " << (GetRows()[i] == spRows[i] ? "eq" : "neq") << std::endl;

                Dump(v);

                std::cout << "Row indices not equal" << std::endl;
                return false;
            }

            for (std::size_t i = 0; i < GetNvals(); i++) {
                auto a = GetVals()[i];
                auto b = spVals[i];

                if ((useError && std::abs(a - b) > error) || (!useError && a != b)) {
                    std::cout << "Values not equal at " << i << " a=" << a << " b=" << b << std::endl;
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] Vector SortReduceDuplicates() const {
            using Pair = std::pair<Index, std::size_t>;

            std::vector<Pair> indices;
            indices.reserve(GetNvals());

            for (std::size_t i = 0; i < GetNvals(); i++) {
                auto row = mRows[i];
                indices.emplace_back(row, i);
            }

            std::sort(indices.begin(), indices.end(), [](const Pair &a, const Pair &b) {
                return a.first < b.first || (a.first == b.first && a.second < b.second);
            });

            indices.erase(std::unique(indices.begin(), indices.end(), [](const Pair &a, const Pair &b) {
                              return a.first == b.first;
                          }),
                          indices.end());

            std::vector<Index> rows;
            std::vector<T> vals;

            rows.reserve(indices.size());
            vals.reserve(indices.size());

            for (const auto &index : indices) {
                auto i = index.second;
                rows.push_back(GetRows()[i]);
                vals.push_back(GetVals()[i]);
            }

            return Vector<T>(GetNrows(), std::move(rows), std::move(vals));
        }

        template<typename M>
        [[nodiscard]] Vector Mask(const Vector<M> &mask, bool complement) const {
            assert(GetNrows() == mask.GetNrows());

            std::vector<Index> rows;
            std::vector<T> vals;

            std::size_t a = 0;
            std::size_t b = 0;
            std::size_t endA = GetNvals();
            std::size_t endB = mask.GetNvals();

            if (complement) {
                while (a != endA && b != endB) {
                    if (GetRows()[a] == mask.GetRows()[b]) {
                        a += 1;
                        b += 1;
                    } else if (GetRows()[a] < mask.GetRows()[b]) {
                        rows.push_back(GetRows()[a]);
                        vals.push_back(GetVals()[a]);
                        a += 1;
                    } else {
                        b += 1;
                    }
                }

                while (a != endA) {
                    rows.push_back(GetRows()[a]);
                    vals.push_back(GetVals()[a]);
                    a += 1;
                }
            } else {
                while (a != endA && b != endB) {
                    if (GetRows()[a] == mask.GetRows()[b]) {
                        rows.push_back(GetRows()[a]);
                        vals.push_back(GetVals()[a]);
                        a += 1;
                        b += 1;
                    } else if (GetRows()[a] < mask.GetRows()[b]) {
                        a += 1;
                    } else {
                        b += 1;
                    }
                }
            }

            return Vector<T>(GetNrows(), std::move(rows), std::move(vals));
        }

        template<typename BinaryOp>
        [[nodiscard]] Vector EWiseAdd(const Vector<T> &other, BinaryOp op) const {
            assert(GetNrows() == other.GetNrows());

            std::vector<Index> rows;
            std::vector<T> vals;

            std::size_t a = 0;
            std::size_t b = 0;
            std::size_t endA = GetNvals();
            std::size_t endB = other.GetNvals();

            while (a != endA && b != endB) {
                if (GetRows()[a] == other.GetRows()[b]) {
                    rows.push_back(GetRows()[a]);
                    vals.push_back(op(GetVals()[a], other.GetVals()[b]));
                    a += 1;
                    b += 1;
                } else if (GetRows()[a] < other.GetRows()[b]) {
                    rows.push_back(GetRows()[a]);
                    vals.push_back(GetVals()[a]);
                    a += 1;
                } else {
                    rows.push_back(other.GetRows()[b]);
                    vals.push_back(other.GetVals()[b]);
                    b += 1;
                }
            }

            while (a != endA) {
                rows.push_back(GetRows()[a]);
                vals.push_back(GetVals()[a]);
                a += 1;
            }

            while (b != endB) {
                rows.push_back(other.GetRows()[b]);
                vals.push_back(other.GetVals()[b]);
                b += 1;
            }

            return Vector<T>(GetNrows(), std::move(rows), std::move(vals));
        }

        template<typename M, typename BinaryOp>
        [[nodiscard]] Vector EWiseAdd(const Vector<M> &mask, bool complement, const Vector<T> &other, BinaryOp op) const {
            return Mask(mask, complement).EWiseAdd(other.Mask(mask, complement), op);
        }

        template<typename M, typename BinaryOp>
        [[nodiscard]] Vector Assign(const Vector<M> &mask, bool complement, T scalar, BinaryOp accum) {
            std::vector<Index> rows(mNrows);
            std::vector<T> vals(mNrows, scalar);

            for (Index i = 0; i < mNrows; i++)
                rows[i] = i;

            auto tmp = Vector<T>(GetNrows(), std::move(rows), std::move(vals)).Mask(mask, complement);
            return EWiseAdd(tmp, accum);
        }

        template<typename ReduceT, typename R = std::invoke_result_t<ReduceT, T, T>>
        [[nodiscard]] R Reduce(ReduceT reduce) {
            if (mVals.empty()) {
                return 0;
            }

            R result = mVals[0];
            for (std::size_t i = 1; i < mVals.size(); ++i) {
                result = reduce(result, mVals[i]);
            }
            return result;
        }

        static Vector Generate(std::size_t nrows, std::size_t nvals, std::size_t seed = 0, const T &value = T()) {
            std::vector<Index> rows;
            std::vector<T> vals(nvals, value);

            rows.reserve(nvals);

            std::default_random_engine engine(seed);
            auto distRows = std::uniform_int_distribution<Index>(0, nrows > 0 ? nrows - 1 : nrows);

            for (std::size_t i = 0; i < nvals; i++) {
                auto row = distRows(engine);
                assert(row < nrows);
                rows.push_back(row);
            }

            return Vector<T>(nrows, std::move(rows), std::move(vals));
        }

        [[nodiscard]] spla::RefPtr<spla::HostVector> ToHostVector() const {
            std::vector<Index> rows = mRows;
            std::vector<unsigned char> data(mVals.size() * sizeof(T));
            std::memcpy(data.data(), mVals.data(), mVals.size() * sizeof(T));
            return spla::RefPtr<spla::HostVector>(new spla::HostVector(mNrows, std::move(rows), std::move(data)));
        }

        static Vector FromHostVector(const spla::RefPtr<spla::HostVector> &m) {
            std::vector<Index> rows = m->GetRowIndices();
            std::vector<T> values(m->GetNnvals());
            std::memcpy(values.data(), m->GetValues().data(), values.size() * sizeof(T));
            return Vector(m->GetNrows(), std::move(rows), std::move(values));
        }

        static Vector Empty(std::size_t nrows) {
            return Vector(nrows, std::vector<Index>{}, std::vector<T>{});
        }

        static void Dump(const spla::RefPtr<spla::Vector> &v) {
            std::stringstream ss;
            v->Dump(ss);
            std::cout << ss.str();
        }

    private:
        std::vector<Index> mRows;
        std::vector<T> mVals;
        std::size_t mNrows;
    };

}// namespace utils

#endif//SPLA_VECTOR_HPP
