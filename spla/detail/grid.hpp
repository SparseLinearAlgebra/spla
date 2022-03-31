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

#ifndef SPLA_GRID_HPP
#define SPLA_GRID_HPP

#include <cstddef>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <spla/detail/pair_utils.hpp>

namespace spla::detail {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class Grid
     * @brief Two-dimensional grid of elements
     *
     * @tparam T Type of stored elements inside grid
     */
    template<typename T>
    class Grid {
    public:
        /**
         * @class Index
         * @brief Auxiliary to index elements of the grid
         */
        typedef std::pair<std::size_t, std::size_t> Coord;
        typedef std::unordered_map<Coord, T, PairHash> Elements;

        /**
         * @class Slice
         * @brief Represents slice of the grid in one row
         */
        class Slice {
        public:
            Slice(std::size_t row, std::pair<std::size_t, std::size_t> dim, Elements &elements)
                : m_row(row), m_dim(std::move(dim)), m_elements(elements) {}

            [[nodiscard]] bool has(std::size_t col) const {
                assert(col < m_dim.second);
                return m_elements.find(Coord{m_row, col}) != m_elements.end();
            }

            T &operator[](std::size_t col) {
                assert(col < m_dim.second);
                return m_elements[Coord{m_row, col}];
            }

        private:
            std::size_t m_row;
            std::pair<std::size_t, std::size_t> m_dim;
            Elements &m_elements;
        };

        Grid() = default;

        explicit Grid(std::pair<std::size_t, std::size_t> dim) : m_dim(std::move(dim)) {}

        Slice operator[](std::size_t row) {
            assert(row < m_dim.first);
            return Slice(row, m_dim, m_elements);
        }

        T &operator[](const Coord &idx) {
            assert(idx.first < m_dim.first);
            assert(idx.second < m_dim.second);
            return m_elements[idx];
        }

        T operator[](const Coord &idx) const {
            assert(idx.first < m_dim.first);
            assert(idx.second < m_dim.second);

            auto query = m_elements.find(idx);
            return query != m_elements.end() ? query->second : T();
        }

        [[nodiscard]] std::pair<std::size_t, std::size_t> dim() const { return m_dim; }
        [[nodiscard]] const Elements &elements() const { return m_elements; }

        [[nodiscard]] std::vector<T> column(std::size_t j) const {
            assert(j < m_dim.second);
            std::vector<T> values;

            for (std::size_t i = 0; i < m_dim.first; i++) {
                auto query = m_elements.find(Coord{i, j});
                if (query != m_elements.end())
                    values.push_back(query->second);
            }

            return values;
        }

    private:
        std::pair<std::size_t, std::size_t> m_dim{0, 0};
        Elements m_elements;
    };

    template<typename T>
    class GridTS {
    public:
        typedef typename Grid<T>::Coord Coord;
        typedef typename Grid<T>::Elements Elements;

        explicit GridTS(const std::pair<std::size_t, std::size_t> &dim) : m_grid(dim) {}

        void set_block(const Coord &idx, const T &element) {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            m_grid[idx] = element;
        }

        const Grid<T> &grid() const { return m_grid; }
        const Elements &elements() const { return m_grid.elements(); }

    private:
        Grid<T> m_grid;
        mutable std::mutex m_mutex;
    };

    /**
     * @}
     */

}// namespace spla::detail

#endif//SPLA_GRID_HPP
