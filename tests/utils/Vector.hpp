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

#include <spla-cpp/Spla.hpp>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <random>

namespace utils {

    template <typename T>
    class Vector {
    public:
        using RowIndex = unsigned int;
        static constexpr std::size_t RowIndexSize = sizeof(RowIndex);
        static constexpr std::size_t ElementSize = sizeof(T);

        explicit Vector(std::size_t size, std::size_t values, bool sorted, std::size_t seed = 0)
            : mSize(size),
              mValues(values),
              mRows(values) {
            std::default_random_engine engine(seed);
            auto distRows = std::uniform_int_distribution<size_t>(0, size - 1);

            for (size_t i = 0; i < values; i++) {
                mRows[i] = distRows(engine);
            }
            
            if (sorted) {
                *this = SortReduceDuplicates();
            }
        }

        template <typename Gen>
        void Fill(Gen generator) {
            for (T &value : mValues) {
                value = generator();
            }
        }

        const std::vector<T> &GetValuesVector() const noexcept {
            return mValues;
        }

        [[nodiscard]] T *GetData() noexcept {
            return mValues.data();
        }

        [[nodiscard]] RowIndex *GetRows() noexcept {
            return mRows.data();
        }

        [[nodiscard]] std::size_t GetSize() const noexcept {
            return mSize;
        }

        [[nodiscard]] std::size_t GetValues() const noexcept {
            return mValues.size();
        }

        [[nodiscard]] bool Equals(const spla::RefPtr<spla::Vector> &v) const {
            if (v->GetNrows() != GetSize() || v->GetNvals() != GetValues())
                return false;

            if (ElementSize != v->GetType()->GetByteSize())
                return false;

            std::vector<RowIndex> spRows(mSize);
            std::vector<char> spVals(GetSize() * ElementSize);

            auto &library = v->GetLibrary();
            auto data = spla::DataVector::Make(library);
            data->SetRows(spRows.data());
            data->SetVals(spVals.data());
            data->SetNvals(GetValues());

            auto readExpr = spla::Expression::Make(library);
            readExpr->MakeDataRead(v, data);
            library.Submit(readExpr);

            if (readExpr->GetState() != spla::Expression::State::Evaluated)
                return false;

            if (!std::memcmp(GetRows(), spRows.data(), GetSize() * RowIndexSize))
                return false;
            if (!std::memcmp(GetData(), spVals.data(), GetValues() * ElementSize))
                return false;

            return true;
        }

        [[nodiscard]] Vector SortReduceDuplicates() const {
            std::vector<RowIndex> sortedIndices = mRows;
            std::sort(sortedIndices.begin(), sortedIndices.end());
            sortedIndices.erase(std::unique(sortedIndices.begin(), sortedIndices.end()), sortedIndices.end());
            std::unordered_map<RowIndex, T> positionValue;
            for (size_t i = 0; i < mRows.size(); ++i) {
                positionValue[mRows[i]] = mValues[i];
            }

            Vector vSorted(*this);
            vSorted.mSize = mSize;
            vSorted.mRows = sortedIndices;
            vSorted.mValues.resize(0);
            vSorted.mValues.reserve(mSize);

            for (RowIndex initialIndex : sortedIndices) {
                vSorted.mValues.push_back(positionValue[initialIndex]);
            }

            return vSorted;
        }

        private:
            std::size_t mSize;
            std::vector<T> mValues;
            std::vector<unsigned int> mRows;
    };

}// namespace utils

#endif//SPLA_VECTOR_HPP
