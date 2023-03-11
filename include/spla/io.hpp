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

#ifndef SPLA_IO_HPP
#define SPLA_IO_HPP

#include "config.hpp"

#include <filesystem>
#include <vector>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class MtxLoader
     * @brief Loader for matrix data stored in matrix-market (.mtx) format
     */
    class MtxLoader {
    public:
        SPLA_API explicit MtxLoader(std::string name = "");
        SPLA_API ~MtxLoader() = default;

        /**
         * @brief Load .mtx data from given file path
         *
         * @param file_path Relative or absolute path to file
         * @param offset_indices True if requires indices offset by -1
         * @param make_undirected True if for each directed edge reverse edge must be added
         * @param remove_loops True if self-loops must be removed
         *
         * @return True if successfully loaded
         */
        SPLA_API bool load(std::filesystem::path file_path,
                           bool                  offset_indices  = true,
                           bool                  make_undirected = true,
                           bool                  remove_loops    = true);

        /**
         * @brief Saves loaded data at file
         *
         * @param file_path File to create where to save data
         * @param stats_only Save only stats without actual graph data
         *
         * @return True if successfully saved
         */
        SPLA_API bool save(const std::filesystem::path& file_path,
                           bool                         stats_only = false);

        SPLA_API void calc_stats();
        SPLA_API void output_stats();

        [[nodiscard]] SPLA_API const std::vector<uint>& get_Ai() const;
        [[nodiscard]] SPLA_API const std::vector<uint>& get_Aj() const;
        [[nodiscard]] SPLA_API uint                     get_n_rows() const;
        [[nodiscard]] SPLA_API uint                     get_n_cols() const;
        [[nodiscard]] SPLA_API std::size_t get_n_values() const;

    private:
        std::string           m_name;
        std::filesystem::path m_file_path;
        std::vector<uint>     m_Ai;
        std::vector<uint>     m_Aj;
        bool                  m_base_is_zero = false;
        uint                  m_n_rows       = 0;
        uint                  m_n_cols       = 0;
        std::size_t           m_n_values     = 0;
        double                m_deg_avg      = -1.0;
        double                m_deg_sd       = -1.0;
        double                m_deg_min      = -1.0;
        double                m_deg_max      = -1.0;
        std::vector<double>   m_deg_distribution;
        std::vector<uint>     m_deg_ranges;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_IO_HPP
