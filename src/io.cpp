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

#include <spla/io.hpp>
#include <spla/library.hpp>
#include <spla/timer.hpp>

#include <core/logger.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

namespace spla {

    MtxLoader::MtxLoader(std::string name) : m_name(std::move(name)) {
    }

    bool MtxLoader::load(std::filesystem::path file_path, bool offset_indices, bool make_undirected, bool remove_loops) {
        m_file_path    = std::move(file_path);
        m_base_is_zero = offset_indices;

        std::fstream file(m_file_path, std::ios::in);
        if (!file.is_open()) {
            LOG_MSG(Status::Error, "failed to open file " << m_file_path);
            return false;
        }

        Timer t;
        t.start();

        std::size_t n_lines = 0;
        std::size_t n_sort  = 0;

        std::string line;
        while (std::getline(file, line)) {
            if (line[0] != '%') break;
            n_lines++;
        }

        std::size_t       nnz;
        std::stringstream header(line);
        header >> m_n_rows >> m_n_cols >> nnz;

        std::cout << "Loading matrix-market coordinate format data... " << std::endl;
        std::cout << " Reading from " << m_file_path << std::endl;
        std::cout << " Matrix size " << m_n_rows << " rows, " << m_n_cols << " cols" << std::endl;
        std::cout << " Data: " << nnz << " directed edges" << std::endl;
        if (remove_loops) std::cout << " Opt: remove self-loops" << std::endl;
        if (offset_indices) std::cout << " Opt: offset indices by -1" << std::endl;
        if (make_undirected) std::cout << " Opt: double edges" << std::endl;
        std::cout << " Reading data: ";

        // optimized reading by sliding window
        const std::size_t BUFFER_CAPACITY = 1024 * 8;
        std::size_t       buffer_size     = 0;
        std::size_t       buffer_offset   = 0;
        char              buffer[BUFFER_CAPACITY + 1];

        // read data
        std::size_t       to_count       = 0;
        std::size_t       to_read        = nnz;
        std::size_t       to_preallocate = to_read * (make_undirected ? 2 : 1);
        std::vector<uint> Ai;
        std::vector<uint> Aj;

        // preallocate to avoid copy
        Ai.reserve(to_preallocate);
        Aj.reserve(to_preallocate);

        float job_done  = 0.0f;
        float job_total = 35.0f;

        while (to_read > 0) {
            to_count++;
            to_read--;
            n_lines++;

            // display current progress of reading the file
            while (float(to_count) / float(nnz) > job_done / job_total) {
                job_done += 1.0f;
                std::cout << "|";
            }

            // try to find where next line is ends up
            bool        line_found = false;
            std::size_t line_end;
            while (!line_found) {
                line_end = buffer_offset;

                // travers buffer to find ending
                while (line_end < buffer_size && buffer[line_end] != '\n') {
                    line_end += 1;
                }

                // buffer not ended of file is ended
                line_found = line_end < buffer_size || file.eof();

                // not found in buffer, need to fetch more data
                if (!line_found) {
                    assert(!file.eof());
                    assert(buffer_offset <= BUFFER_CAPACITY);

                    if (buffer_offset > 0) {
                        if (buffer_offset < BUFFER_CAPACITY) {
                            std::memcpy(buffer, buffer + buffer_offset, BUFFER_CAPACITY - buffer_offset);
                        }
                        buffer_offset = BUFFER_CAPACITY - buffer_offset;
                    }

                    auto bytes_to_read = BUFFER_CAPACITY - buffer_offset;
                    file.read(buffer + buffer_offset, std::streamsize(bytes_to_read));
                    auto bytes_actually_read = file.gcount();
                    buffer_size              = buffer_offset + bytes_actually_read;
                    buffer_offset            = 0;
                    buffer[buffer_size]      = '\0';
                    assert(buffer_size <= BUFFER_CAPACITY + 1);
                }
            }

            char* end     = nullptr;
            auto  i       = uint(std::strtoll(buffer + buffer_offset, &end, 10));
            auto  j       = uint(std::strtoll(end, &end, 10));
            buffer_offset = line_end + 1;

            assert(i > 0 && j > 0);

            if (remove_loops) {
                if (i == j) continue;
            }
            if (offset_indices) {
                i -= 1;
                j -= 1;
            }
            if (make_undirected) {
                Ai.push_back(j);
                Aj.push_back(i);
            }

            Ai.push_back(i);
            Aj.push_back(j);
        }
        t.lap_end();// parsing

        std::vector<std::uint64_t> sorted;
        {
            sorted.reserve(Ai.size());
            n_sort = Ai.size();

            for (std::size_t k = 0; k < Ai.size(); k++) {
                std::uint64_t entry = 0;
                entry |= std::uint64_t(Ai[k]) << 32u;
                entry |= std::uint64_t(Aj[k]) << 0u;
                sorted.push_back(entry);
            }
            Ai.clear();
            Aj.clear();

            std::sort(sorted.begin(), sorted.end());
        }
        t.lap_end();// sorting

        std::vector<uint> reduced_Ai;
        std::vector<uint> reduced_Aj;
        {
            reduced_Ai.reserve(sorted.size());
            reduced_Aj.reserve(sorted.size());

            std::uint64_t entry_prev = 0xffffffffffffffff;
            for (std::uint64_t entry : sorted) {
                if (entry_prev != entry) {
                    uint i = uint((entry >> 32u) & 0xffffffff);
                    uint j = uint((entry >> 0u) & 0xffffffff);
                    reduced_Ai.push_back(i);
                    reduced_Aj.push_back(j);
                }
                entry_prev = entry;
            }

            m_n_values = reduced_Ai.size();
            m_Ai       = std::move(reduced_Ai);
            m_Aj       = std::move(reduced_Aj);
        }
        t.lap_end();// reducing

        calc_stats();
        t.lap_end();// stats

        t.stop();

        std::cout << " 100%" << std::endl;
        std::cout << " Parsed in " << t.get_laps_ms()[0] * 1e-3 << " sec " << n_lines << " lines"
                  << " speed " << float(n_lines) / (t.get_laps_ms()[0] * 1e-3) << " lines/sec" << std::endl;
        std::cout << " Sorted in " << t.get_laps_ms()[1] * 1e-3 << " sec " << n_sort << " lines" << std::endl;
        std::cout << " Reduced in " << t.get_laps_ms()[2] * 1e-3 << " sec " << m_n_values << " lines" << std::endl;
        std::cout << " Calc stats in " << t.get_laps_ms()[3] * 1e-3 << " sec" << std::endl;
        std::cout << " Loaded in " << t.get_elapsed_ms() * 1e-3 << " sec, " << m_n_values << " edges total" << std::endl;

        output_stats();

        return true;
    }

    bool MtxLoader::save(const std::filesystem::path& file_path, bool stats_only) {
        std::fstream file(file_path, std::ios::out);

        if (!file.is_open()) {
            LOG_MSG(Status::Error, "failed to open file " << file_path);
            return false;
        }

        file << "%%MatrixMarket matrix coordinate pattern general\n";
        file << "%-------------------------------------------------------------------------------\n";
        file << "%-------------------------------------------------------------------------------\n";

        file << "% meta-info:\n";
        file << "% name: " << m_name << "\n";
        file << "% source-file: " << m_file_path << "\n";
        file << "% deg-avg: " << m_deg_avg << "\n";
        file << "% deg-sd: " << m_deg_sd << "\n";
        file << "% deg-min: " << m_deg_min << "\n";
        file << "% deg-max: " << m_deg_max << "\n";
        file << "% deg-distribution: \n";

        for (std::size_t i = 0; i < m_deg_distribution.size(); i++) {
            file << "%  " << m_deg_ranges[i] << " " << m_deg_ranges[i + 1] << " " << m_deg_distribution[i] << "\n";
        }

        file << "%-------------------------------------------------------------------------------\n";
        file << m_n_rows << " " << m_n_cols << " " << m_n_values << "\n";

        if (!stats_only) {
            const uint offset = m_base_is_zero ? 1 : 0;
            for (std::size_t k = 0; k < m_n_values; k++) {
                file << m_Ai[k] + offset << " " << m_Aj[k] + offset << "\n";
            }
        }

        return true;
    }

    void MtxLoader::calc_stats() {
        std::vector<uint> deg_pre_vertex(m_n_rows, 0.0f);

        for (auto i : m_Ai) {
            deg_pre_vertex[m_base_is_zero ? i : i - 1] += 1;
        }

        m_deg_sd  = 0.0;
        m_deg_avg = 0.0;
        m_deg_max = -1.0;
        m_deg_min = 1.0 + static_cast<double>(m_n_values);

        for (auto deg : deg_pre_vertex) {
            m_deg_min = std::min(m_deg_min, static_cast<double>(deg));
            m_deg_max = std::max(m_deg_max, static_cast<double>(deg));
            m_deg_avg += deg;
            m_deg_sd += deg * deg;
        }

        auto n = static_cast<double>(m_n_rows);

        m_deg_avg = m_deg_avg / n;
        m_deg_sd  = std::sqrt(n * (m_deg_sd / n - m_deg_avg * m_deg_avg) / (n > 1.0 ? n - 1.0 : 1.0));

        const uint GROUPS_COUNT_MAX = std::max(uint(10), uint(std::log2(double(m_n_rows) * 0.77)));

        std::vector<uint> count_per_deg(static_cast<uint>(m_deg_max) + 2, 0);
        std::vector<uint> count_per_deg_offsets(static_cast<uint>(m_deg_max) + 2, 0);

        for (uint i = 0; i < m_n_rows; i++) {
            count_per_deg[std::min(deg_pre_vertex[i], uint(m_deg_max))] += 1;
        }

        std::exclusive_scan(count_per_deg.begin(), count_per_deg.end(), count_per_deg_offsets.begin(), 0);
        count_per_deg_offsets.back() += 1;

        std::vector<double> distributions;
        std::vector<uint>   ranges;

        auto range        = m_deg_max - m_deg_min;
        auto groups_count = std::max(std::min(GROUPS_COUNT_MAX, static_cast<uint>(range)), 1u);
        auto g            = static_cast<double>(groups_count);

        auto total = static_cast<double>(count_per_deg_offsets.back());
        auto from  = count_per_deg_offsets.begin();

        ranges.push_back(static_cast<uint>(m_deg_min));
        for (uint i = 0; i < groups_count; ++i) {
            auto next      = (from + 1 == count_per_deg_offsets.end()) ? from : from + 1;
            auto to        = std::lower_bound(next, count_per_deg_offsets.end(), static_cast<uint>(total / g * static_cast<double>(i + 1)));
            auto to_offset = std::distance(count_per_deg_offsets.begin(), to);

            assert(to != count_per_deg_offsets.end());

            distributions.push_back(static_cast<double>(*to - *from) / total);
            ranges.push_back(static_cast<uint>(to_offset));
            from = to;
        }

        m_deg_distribution = std::move(distributions);
        m_deg_ranges       = std::move(ranges);
    }
    void MtxLoader::output_stats() {
        std::cout << " "
                  << "deg: "
                  << "min " << m_deg_min << ", "
                  << "max " << m_deg_max << ", "
                  << "avg " << m_deg_avg << ", "
                  << "sd " << m_deg_sd << std::endl;

        std::cout << " distribution:" << std::endl;

        const auto n = static_cast<double>(m_n_rows);
        const auto default_precision{std::cout.precision()};
        const auto n_digits = static_cast<uint>(std::log10(n) + 1.0);

        const double DISPLAY_DENSITY = std::max(double(100), double(m_deg_distribution.size()));

        for (std::size_t i = 0; i < m_deg_distribution.size(); i++) {
            auto deg     = m_deg_distribution[i] >= 0.01 ? m_deg_distribution[i] : 0.0;
            auto k_start = m_deg_ranges[i];
            auto k_end   = m_deg_ranges[i + 1];
            auto k_count = std::round(static_cast<uint>(deg * DISPLAY_DENSITY));

            std::cout << "  [" << std::setw(n_digits) << k_start << " - " << std::setw(n_digits) << k_end << "): ";
            std::cout << std::setw(6) << std::setprecision(2) << deg * 100.0 << std::setprecision(default_precision) << "% ";
            for (uint s = 0; s < k_count; ++s) { std::cout << "*"; }
            std::cout << std::endl;
        }
    }

    const std::vector<uint>& MtxLoader::get_Ai() const {
        return m_Ai;
    }
    const std::vector<uint>& MtxLoader::get_Aj() const {
        return m_Aj;
    }

    uint MtxLoader::get_n_rows() const {
        return m_n_rows;
    }
    uint MtxLoader::get_n_cols() const {
        return m_n_cols;
    }
    std::size_t MtxLoader::get_n_values() const {
        return m_n_values;
    }

}// namespace spla