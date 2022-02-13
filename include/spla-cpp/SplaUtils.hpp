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

#ifndef SPLA_SPLAUTILS_HPP
#define SPLA_SPLAUTILS_HPP

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <vector>

#include <spla-cpp/SplaConfig.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class CpuTimer
     * @brief Simple timer for measuring time on cpu side
     */
    class CpuTimer {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = std::chrono::nanoseconds;
        using Value = double;

    public:
        void Start() {
            mStart = mEnd = Clock::now();
        }

        void Stop() {
            mEnd = Clock::now();
            mElapsedMs += GetDurationMs();
        }

        Value Mark() {
            Stop();
            auto duration = GetDurationMs();
            Start();
            return duration;
        }

        void Reset() {
            mElapsedMs = 0.0;
            mStart = mEnd = TimePoint{};
        }

        [[nodiscard]] Duration GetDuration() const { return GetEnd() - GetStart(); }
        [[nodiscard]] Value GetDurationMs() const { return static_cast<double>(GetDuration().count()) * 1e-6; }
        [[nodiscard]] Value GetElapsedMs() const { return mElapsedMs; }

        [[nodiscard]] TimePoint GetStart() const noexcept { return mStart; }
        [[nodiscard]] TimePoint GetEnd() const noexcept { return mEnd; }

    private:
        TimePoint mStart{};
        TimePoint mEnd{};

        double mElapsedMs = 0.0;
    };

    /**
     * @class MatrixLoader
     * @brief A matrix loader of .mtx format
     *
     * @tparam Value A type of target matrix value
     */
    template<typename Value>
    class MatrixLoader {
    public:
        MatrixLoader() = default;

        void Save(std::ostream &out) const {
            out << GetNrows() << " " << GetNcols() << " " << GetNvals() << "\n";
            for (std::size_t i = 0; i < GetNvals(); i++) {
                out << mRows[i] + 1 << " " << mCols[i] + 1 << "\n";
            }
        }

        /**
         * @brief Loads matrix from @p is
         *
         * @tparam FileValue Type of value which is written in the file
         * @param is An input stream with .mtx matrix inside
         * @param makeUndirected Add backward edges for directed edges
         * @param removeSelfLoops Remove loops (edges of type i -> i)
         * @param ignoreValues Ignore values inside file
         * @param verbose Verbose std output info
         * @param source Source name to display
         *
         * @return Reference at created matrix
         */
        MatrixLoader &Load(std::istream &is, bool makeUndirected, bool removeSelfLoops, bool ignoreValues, bool verbose = true, const std::string &source = "") {
            if (verbose) {
                std::cout << "Loading Matrix-market coordinate format graph...\n";
                std::cout << " Reading from " << source << "\n";

                if (removeSelfLoops)
                    std::cout << " Removing self-loops\n";
            }

            CpuTimer timer;
            CpuTimer total;
            total.Start();
            timer.Start();

            std::string line;
            std::size_t lineN = 0;
            while (std::getline(is, line)) {
                ++lineN;
                if (line[0] != '%') break;
            }
            Size nnz;
            std::stringstream headerLineStream(line);
            headerLineStream >> mNrows >> mNcols >> nnz;

            mRows.reserve(nnz);
            mCols.reserve(nnz);
            mVals.reserve(nnz);

            timer.Mark();

            while (std::getline(is, line)) {
                ++lineN;
                std::stringstream lineStream(line);
                Index i, j;
                lineStream >> i >> j;

                if (!(1 <= i && i <= mNrows)) {
                    throw std::logic_error("Row index is out of bounds on the line " + std::to_string(lineN));
                }

                if (!(1 <= j && j <= mNcols)) {
                    throw std::logic_error("Column index is out of bounds on the line " + std::to_string(lineN));
                }

                if (removeSelfLoops) {
                    if (i == j) {
                        nnz -= 1;
                        continue;
                    }
                }

                mRows.push_back(i);
                mCols.push_back(j);

                if (!lineStream.eof() && !ignoreValues) {
                    Value v;
                    lineStream >> v;
                    mVals.push_back(v);
                }
            }

            if (ignoreValues)
                mVals.resize(mRows.size());

            timer.Stop();

            if (mRows.size() != nnz) {
                throw std::logic_error("Number of non zero values is not valid");
            }

            if (verbose)
                std::cout << " Parsing MTX file ("
                          << mNrows << " rows, "
                          << mNcols << " cols, "
                          << nnz << " directed edges)"
                          << " in " << timer.GetElapsedMs() << " ms\n";

            timer.Start();

            // Offset indices
            for (std::size_t k = 0; k < mRows.size(); k++) {
                mRows[k] -= 1;
                mCols[k] -= 1;
            }

            timer.Stop();

            if (verbose)
                std::cout << " Offset indices by -1 in "
                          << timer.GetDurationMs() << " ms\n";

            if (makeUndirected) {
                timer.Start();
                DoubleEdges();
                timer.Stop();

                if (verbose)
                    std::cout << " Doubling edges: " << nnz
                              << " to " << GetNvals()
                              << " in " << timer.GetDurationMs() << " ms\n";
            }

            {
                timer.Start();
                auto oldNnz = GetNvals();
                SortReduceDuplicates();
                auto newNnz = GetNvals();
                timer.Stop();

                if (verbose)
                    std::cout << " Sort values and reduce duplicates:"
                              << " old " << oldNnz
                              << " new " << newNnz
                              << " diff " << oldNnz - newNnz
                              << " in " << timer.GetDurationMs() << " ms\n";
            }


            double averageDegree;
            std::size_t maxDegree = 0;
            std::size_t minDegree = 0;

            if (GetNrows() == GetNcols())
                ComputeStats(minDegree, maxDegree, averageDegree);

            total.Stop();

            if (verbose) {
                std::cout << " Stats: min.deg " << minDegree
                          << ", max.deg " << maxDegree
                          << ", avg.deg " << averageDegree << "\n";
                std::cout << " Loaded graph"
                          << " vertices " << GetNrows()
                          << " edges " << GetNvals()
                          << " in " << total.GetElapsedMs() << " ms\n";
            }

            return *this;
        }

        MatrixLoader &Load(const std::string &filename, bool makeUndirected, bool removeSelfLoops, bool ignoreValues, bool verbose = true) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::invalid_argument("Could not open '" + filename + "' to read matrix");
            }
            return Load(file, makeUndirected, removeSelfLoops, ignoreValues, verbose, filename);
        }

        void Fill(Value value) {
            for (auto &v : mVals) {
                v = value;
            }
        }

        template<typename Generator>
        void Generate(Generator &&generator) {
            for (auto &v : mVals) {
                v = generator();
            }
        }

        [[nodiscard]] Size GetNrows() const { return mNrows; }
        [[nodiscard]] Size GetNcols() const { return mNcols; }
        [[nodiscard]] Size GetNvals() const { return mRows.size(); }
        [[nodiscard]] static constexpr Size GetElementSize() { return sizeof(Value); }

        [[nodiscard]] const std::vector<Index> &GetRowIndices() const { return mRows; }
        [[nodiscard]] const std::vector<Index> &GetColIndices() const { return mCols; }
        [[nodiscard]] const std::vector<Value> &GetValues() const { return mVals; }

        [[nodiscard]] std::vector<Index> &GetRowIndices() { return mRows; }
        [[nodiscard]] std::vector<Index> &GetColIndices() { return mCols; }
        [[nodiscard]] std::vector<Value> &GetValues() { return mVals; }

    private:
        void DoubleEdges() {
            auto nnz = GetNvals();
            for (std::size_t i = 0; i < nnz; i++) {
                if (mRows[i] != mCols[i]) {
                    mRows.push_back(mCols[i]);
                    mCols.push_back(mRows[i]);
                    mVals.push_back(mVals[i]);
                }
            }
        }

        void SortReduceDuplicates() {
            struct Entry {
                Index i;
                Index j;
                Value v;

                bool operator!=(const Entry &e) const {
                    return i != e.i || j != e.j;
                }
            };
            std::vector<Entry> entries(GetNvals());

            for (std::size_t i = 0; i < GetNvals(); i++)
                entries[i] = Entry{mRows[i], mCols[i], mVals[i]};

            std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) { return a.i < b.i || (a.i == b.i && a.j < b.j); });

            std::vector<Value> newVals;
            std::vector<Index> newRows;
            std::vector<Index> newCols;

            Entry prev{0xffffffff, 0xffffffff, Value()};
            for (const auto &e : entries) {
                if (prev != e) {
                    prev = e;
                    newVals.push_back(e.v);
                    newRows.push_back(e.i);
                    newCols.push_back(e.j);
                }
            }

            std::swap(newVals, mVals);
            std::swap(newRows, mRows);
            std::swap(newCols, mCols);
        }

        void ComputeStats(std::size_t &minDegree, std::size_t &maxDegree, double &averageDegree) {
            std::vector<std::size_t> degreePerVertex(GetNrows(), 0);

            for (std::size_t k = 0; k < GetNvals(); k++) {
                degreePerVertex[mRows[k]] += 1;
            }

            maxDegree = 0;
            minDegree = GetNvals() + 1;
            averageDegree = 0.0f;

            for (auto d : degreePerVertex) {
                maxDegree = std::max(maxDegree, d);
                minDegree = std::min(minDegree, d);
                averageDegree += static_cast<double>(d);
            }

            averageDegree = GetNrows() > 0 ? averageDegree / static_cast<double>(GetNrows()) : 0.0;
        }

        std::vector<Value> mVals{};
        std::vector<Index> mRows{};
        std::vector<Index> mCols{};

        Size mNrows{};
        Size mNcols{};
    };

    namespace {
        inline void OutputMeasurements(std::ostream &stream, const CpuTimer &warmUp, const std::vector<CpuTimer> &iters) {
            stream << "warm-up(ms): " << std::setprecision(15) << warmUp.GetElapsedMs() << "\n";
            stream << "iters(ms): ";
            for (auto &iter : iters)
                stream << iter.GetElapsedMs() << " ";
            stream << std::endl;
        }

        inline void OutputMeasurements(const CpuTimer &warmUp, const std::vector<CpuTimer> &iters) {
            OutputMeasurements(std::cout, warmUp, iters);
        }
    }// namespace

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAUTILS_HPP