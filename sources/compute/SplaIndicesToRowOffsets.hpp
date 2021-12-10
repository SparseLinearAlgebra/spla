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

#ifndef SPLA_SPLAINDICESTOROWOFFSETS_HPP
#define SPLA_SPLAINDICESTOROWOFFSETS_HPP

#include <boost/compute/algorithm.hpp>
#include <boost/compute/command_queue.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @brief Compute row offsets from coo row indices buffer.
     *
     * Result offsets array has size n + 1.
     * Offsets[i] stores first index of row i in indices buffer.
     * Offsets[i + 1] - Offsets[i] equals number of nnz values in row i in indices buffer.
     * Offsets[n] equals number off values in indices buffer.
     * Lengths[i] equals number of nnz values in row i in indices buffer.
     *
     * @param indices Array of rows indices
     * @param[out] offsets Output array with offsets
     * @param[out] lengths Output array of row lengths
     * @param n Number of rows in matrix
     * @param queue Command queue to execute
     */
    inline void IndicesToRowOffsets(const boost::compute::vector<unsigned int> &indices,
                                    boost::compute::vector<unsigned int> &offsets,
                                    boost::compute::vector<unsigned int> &lengths,
                                    std::size_t n,
                                    boost::compute::command_queue &queue) {
        using namespace boost;

        lengths.resize(n + 1, queue);
        offsets.resize(n + 1, queue);
        compute::fill(lengths.begin(), lengths.end(), 0u, queue);

        if (indices.empty()) {
            compute::fill(offsets.begin(), offsets.end(), 0u, queue);
            return;
        }

        BOOST_COMPUTE_CLOSURE(void, countRowLengths, (unsigned int i), (indices, lengths), {
            uint rowId = indices[i];
            atomic_inc(&lengths[rowId]);
        });

        compute::for_each_n(compute::counting_iterator<unsigned int>(0), indices.size(), countRowLengths, queue);

        compute::exclusive_scan(lengths.begin(), lengths.end(), offsets.begin(), queue);
    }

    /**
     * Overload
     */
    inline void IndicesToRowOffsets(const boost::compute::vector<unsigned int> &indices,
                                    boost::compute::vector<unsigned int> &offsets,
                                    std::size_t n,
                                    boost::compute::command_queue &queue) {
        using namespace boost;
        boost::compute::vector<unsigned int> lengths(queue.get_context());
        IndicesToRowOffsets(indices, offsets, lengths, n, queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAINDICESTOROWOFFSETS_HPP
