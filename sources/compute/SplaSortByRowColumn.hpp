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

#ifndef SPLA_SPLASORTBYROWCOLUMN_HPP
#define SPLA_SPLASORTBYROWCOLUMN_HPP

#include <boost/compute.hpp>
#include <cassert>
#include <compute/SplaGather.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @brief Sort matrix data in coo format in row-column order.
     * @note If elementsInSequence is 0 and vals is empty, sorts only matrix indices.
     *
     * @param rows Vector with row indices to sort
     * @param cols Vector with column indices to sort
     * @param vals Vector with values byte data
     * @param elementsInSequence Size in bytes of values in vals vector
     * @param queue Queue to perform sort operation
     */
    inline void SortByRowColumn(boost::compute::vector<unsigned int> &rows,
                                boost::compute::vector<unsigned int> &cols,
                                boost::compute::vector<unsigned char> &vals,
                                std::size_t elementsInSequence,
                                boost::compute::command_queue &queue) {
        using namespace boost;

        compute::context ctx = queue.get_context();
        std::size_t nvals = rows.size();
        bool typeHasValues = elementsInSequence != 0;

        assert(cols.size() == nvals);
        assert(vals.size() == nvals * elementsInSequence);

        // Use permutation buffer to synchronize position of indices of the same
        // entries in all 3 buffers: rows, cols and vals.
        compute::vector<unsigned int> permutation(nvals, ctx);
        compute::copy(compute::counting_iterator<unsigned int>(0),
                      compute::counting_iterator<unsigned int>(nvals),
                      permutation.begin(),
                      queue);

        compute::vector<unsigned int> tmp(nvals, ctx);
        compute::copy(cols.begin(), cols.end(), tmp.begin(), queue);

        // Sort in column order and then, using permutation, shuffle row indices
        compute::sort_by_key(tmp.begin(), tmp.end(), permutation.begin(), queue);
        compute::gather(permutation.begin(), permutation.end(), rows.begin(), tmp.begin(), queue);

        // Sort in row order and then, using permutation, shuffle column indices
        compute::sort_by_key(tmp.begin(), tmp.end(), permutation.begin(), queue);
        compute::copy(tmp.begin(), tmp.end(), rows.begin(), queue);
        compute::copy(cols.begin(), cols.end(), tmp.begin(), queue);
        compute::gather(permutation.begin(), permutation.end(), tmp.begin(), cols.begin(), queue);

        // Copy values, using permutation
        if (typeHasValues) {
            compute::vector<unsigned char> valsTmp(nvals * elementsInSequence, ctx);
            Gather(permutation.begin(), permutation.end(), vals.begin(), valsTmp.begin(), elementsInSequence, queue);
            std::swap(vals, valsTmp);
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLASORTBYROWCOLUMN_HPP
