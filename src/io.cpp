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

#include <spla/io.hpp>
#include <spla/library.hpp>
#include <spla/timer.hpp>

#include <core/logger.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace spla {

    static const uint   GROUPS_COUNT    = 10;
    static const double DISPLAY_DENSITY = 100;

    MtxLoader::MtxLoader(std::string name) : m_name(std::move(name)) {
    }

    bool MtxLoader::load(std::filesystem::path file_path, bool offset_indices, bool make_undirected, bool remove_loops) {
        m_file_path = std::move(file_path);

        std::ifstream file(m_file_path, std::ios::in);

        if (!file.is_open()) {
            LOG_MSG(Status::Error, "failed to open file " << m_file_path);
            return false;
        }

        Timer t;

        t.start();

        std::size_t n_lines = 0;
        std::string line;
        while (std::getline(file, line)) {
            if (line[0] != '%') break;
            n_lines++;
        }

        std::size_t       nnz;
        std::stringstream header(line);
        header >> m_n_rows >> m_n_cols >> nnz;

        std::size_t       to_read = nnz;
        std::vector<uint> Ai;
        std::vector<uint> Aj;

        while (to_read > 0) {
            to_read--;
            n_lines++;

            std::getline(file, line);
            std::stringstream entry(line);

            uint i, j;
            entry >> i >> j;

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

        m_Ai       = std::move(Ai);
        m_Aj       = std::move(Aj);
        m_n_values = m_Ai.size();

        t.lap_end();// parsing

        calc_stats();
        t.lap_end();// stats

        t.stop();

        std::cout << "Loading matrix-market coordinate format data... " << std::endl;
        std::cout << " Reading from " << m_file_path << std::endl;
        std::cout << " Matrix size " << m_n_rows << " rows, " << m_n_cols << " cols" << std::endl;
        if (remove_loops) std::cout << " Removing self-loops" << std::endl;
        if (offset_indices) std::cout << " Offsetting indices by -1" << std::endl;
        if (make_undirected) std::cout << " Doubling edges" << std::endl;
        std::cout << " Read data: " << n_lines << " lines, " << nnz << " directed edges" << std::endl;
        std::cout << " Parsed in " << t.get_laps_ms()[0] * 1e-3 << " sec" << std::endl;
        std::cout << " Calc stats in " << t.get_laps_ms()[1] * 1e-3 << " sec" << std::endl;
        std::cout << " Loaded in " << t.get_elapsed_ms() * 1e-3 << " sec, " << m_n_values << " edges total" << std::endl;

        output_stats();

        return true;
    }

    void MtxLoader::calc_stats() {
        std::vector<double> deg_pre_vertex(m_n_rows, 0.0f);

        for (auto i : m_Ai) {
            deg_pre_vertex[i] += 1.0f;
        }

        m_deg_sd  = 0.0;
        m_deg_avg = 0.0;
        m_deg_max = -1.0;
        m_deg_min = 1.0 + static_cast<double>(m_n_values);

        for (auto deg : deg_pre_vertex) {
            m_deg_min = std::min(m_deg_min, deg);
            m_deg_max = std::max(m_deg_max, deg);
            m_deg_avg += deg;
            m_deg_sd += deg * deg;
        }

        auto n = static_cast<double>(m_n_rows);

        m_deg_avg = m_deg_avg / n;
        m_deg_sd  = std::sqrt(n * (m_deg_sd / n - m_deg_avg * m_deg_avg) / (n > 1.0 ? n - 1.0 : 1.0));

        std::vector<double> distributions(GROUPS_COUNT, 0.0);

        auto range = m_deg_max - m_deg_min;
        auto g     = static_cast<double>(GROUPS_COUNT);

        for (auto deg : deg_pre_vertex) {
            auto slot = static_cast<uint>(std::max(std::floor(((deg - m_deg_min) / range) * g) - 1.0, 0.0));
            assert(0 <= slot && slot < GROUPS_COUNT);
            distributions[slot] += 1.0;
        }

        for (auto& d : distributions) {
            d = d / n;
        }

        m_deg_distribution = std::move(distributions);
    }
    void MtxLoader::output_stats() {
        std::cout << " "
                  << "deg: "
                  << "min " << m_deg_min << ", "
                  << "max " << m_deg_max << ", "
                  << "avg " << m_deg_avg << ", "
                  << "sd " << m_deg_sd << std::endl;

        std::cout << " distribution:" << std::endl;

        const auto n     = static_cast<double>(m_n_rows);
        const auto g     = static_cast<double>(GROUPS_COUNT);
        const auto range = m_deg_max - m_deg_min;
        const auto default_precision{std::cout.precision()};
        const auto n_digits = static_cast<uint>(std::log10(n) + 1.0);

        for (std::size_t i = 0; i < m_deg_distribution.size(); i++) {
            auto k       = static_cast<double>(i);
            auto k_start = std::floor(m_deg_min + (k / g * range));
            auto k_end   = std::floor(m_deg_min + ((k + 1.0) / g * range));
            auto k_count = std::round(static_cast<uint>(m_deg_distribution[i] * DISPLAY_DENSITY));

            std::cout << "  [" << std::setw(n_digits) << k_start << " - " << std::setw(n_digits) << k_end << "]: ";
            std::cout << std::setw(6) << std::setprecision(2) << m_deg_distribution[i] * 100.0 << std::setprecision(default_precision) << "% ";
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