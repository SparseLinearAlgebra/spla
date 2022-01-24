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

#ifndef SPLA_SPLASTABLESORTBYCOLUMN_HPP
#define SPLA_SPLASTABLESORTBYCOLUMN_HPP

#include <cassert>

#include <boost/compute.hpp>
#include <boost/compute/lambda.hpp>

#include <compute/SplaGather.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @brief Sort matrix data in coo format in column order.
     *
     * @note If elementsInSequence is 0 and vals is empty, sorts only matrix indices.
     * @note The internal sort is stable. Hence, if data is ordered by row before,
     *      then it will be sorted first by row, then by column after.
     *
     * @param rows Vector with row indices to sort
     * @param cols Vector with column indices to sort
     * @param vals Vector with values byte data
     * @param elementsInSequence Size in bytes of values in vals vector
     * @param queue Queue to perform sort operation
     */
    inline void StableSortByColumn(boost::compute::vector<unsigned int> &rows,
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

        BOOST_COMPUTE_CLOSURE(bool, compareRowCol, (unsigned int a, unsigned int b),
                              (cols, rows),
                              {
                                      if (rows[a] != rows[b]) {
                                          return rows[a] < rows[b];
                                      }
                                      return cols[a] < cols[b];
                              });
        compute::stable_sort(permutation.begin(), permutation.end(), compareRowCol, queue);

        compute::vector<unsigned int> colsTmp(nvals, ctx);
        compute::gather(permutation.begin(), permutation.end(), cols.begin(), colsTmp.begin(), queue);
        std::swap(cols, colsTmp);

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

#endif//SPLA_SPLASTABLESORTBYCOLUMN_HPP
