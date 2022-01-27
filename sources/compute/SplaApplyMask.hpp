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

#ifndef SPLA_SPLAAPPLYMASK_HPP
#define SPLA_SPLAAPPLYMASK_HPP

#include <boost/compute/command_queue.hpp>
#include <boost/compute/container/vector.hpp>
#include <compute/SplaGather.hpp>
#include <compute/SplaMaskByKey.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @brief Apply mask to coo vector.
     *
     * @param mask Mask indices
     * @param inputRows Row indices
     * @param inputVals Values
     * @param outputRows Result row indices; resized automatically
     * @param outputVals Result values; resized automatically
     * @param byteSize Size of values; if 0, apply only indices mask
     * @param complement Pass true to apply inverse (complementary mask)
     * @param queue Command queue to perform operation
     */
    inline void ApplyMask(const boost::compute::vector<unsigned int> &mask,
                          const boost::compute::vector<unsigned int> &inputRows,
                          const boost::compute::vector<unsigned char> &inputVals,
                          boost::compute::vector<unsigned int> &outputRows,
                          boost::compute::vector<unsigned char> &outputVals,
                          std::size_t byteSize,
                          bool complement,
                          boost::compute::command_queue &queue) {
        using namespace boost;

        if (mask.empty() || inputRows.empty())
            return;

        auto ctx = queue.get_context();
        auto typeHasValues = byteSize != 0;
        auto nnz = inputRows.size();
        compute::vector<unsigned int> perm(ctx);

        if (typeHasValues) {
            // Fill permutation to link value to indices
            perm.resize(nnz, queue);
            compute::copy_n(compute::make_counting_iterator(0), nnz, perm.begin(), queue);
        }

        if (typeHasValues) {
            // Mask with new permutation buffer
            compute::vector<unsigned int> outputPerm(ctx);
            MaskByKeys(mask,
                       inputRows, perm,
                       outputRows, outputPerm,
                       complement,
                       queue);
            std::swap(perm, outputPerm);
        } else
            MaskKeys(mask, inputRows, outputRows, complement, queue);

        if (typeHasValues) {
            // Copy output values using indices buffer
            outputVals.resize(outputRows.size() * byteSize, queue);
            Gather(perm.begin(), perm.end(), inputVals.begin(), outputVals.begin(), byteSize, queue);
        }
    }

    /**
     * @brief Apply mask to coo matrix.
     *
     * @param maskRows Mask indices of rows
     * @param maskCols Mask indices of cols
     * @param inputRows Row indices
     * @param inputCols Col indices
     * @param inputVals Values
     * @param outputRows Result row indices; resized automatically
     * @param outputCols Result col indices; resized automatically
     * @param outputVals Result values; resized automatically
     * @param byteSize Size of values; if 0, apply only indices mask
     * @param complement If mask is a complement
     * @param queue Command queue to perform operation
     */
    inline void ApplyMask(const boost::compute::vector<unsigned int> &maskRows,
                          const boost::compute::vector<unsigned int> &maskCols,
                          const boost::compute::vector<unsigned int> &inputRows,
                          const boost::compute::vector<unsigned int> &inputCols,
                          const boost::compute::vector<unsigned char> &inputVals,
                          boost::compute::vector<unsigned int> &outputRows,
                          boost::compute::vector<unsigned int> &outputCols,
                          boost::compute::vector<unsigned char> &outputVals,
                          std::size_t byteSize,
                          bool complement,
                          boost::compute::command_queue &queue) {
        using namespace boost;

        assert(maskRows.size() == maskCols.size());
        assert(inputRows.size() == inputCols.size());

        if (maskRows.empty() || inputRows.empty())
            return;

        auto ctx = queue.get_context();
        auto typeHasValues = byteSize != 0;
        auto nnz = inputRows.size();
        compute::vector<unsigned int> perm(ctx);

        if (typeHasValues) {
            // Fill permutation to link value to indices
            perm.resize(nnz, queue);
            compute::copy_n(compute::make_counting_iterator(0), nnz, perm.begin(), queue);
        }

        if (typeHasValues) {
            // Mask with new permutation buffer
            compute::vector<unsigned int> outputPerm(ctx);
            MaskByPairKeys(maskRows, maskCols,
                           inputRows, inputCols, perm,
                           outputRows, outputCols, outputPerm,
                           complement,
                           queue);
            std::swap(perm, outputPerm);
        } else {
            MaskPairKeys(maskRows, maskCols, inputRows, inputCols, outputRows, outputCols, complement, queue);
        }

        if (typeHasValues) {
            // Copy output values using indices buffer
            outputVals.resize(outputRows.size() * byteSize, queue);
            Gather(perm.begin(), perm.end(), inputVals.begin(), outputVals.begin(), byteSize, queue);
        }
    }

    /**
     * @brief Apply to dense vector sparse mask
     *
     * @param v Dense vector to mask
     * @param m Sparse mask (list of indices)
     * @param complement True if complement mask
     * @param queue Queue to perform operation on
     */
    inline void ApplyMaskDenseSparse(boost::compute::vector<Index> &v,
                                     const boost::compute::vector<Index> &m,
                                     bool complement,
                                     boost::compute::command_queue &queue) {
        using namespace boost;

        assert(!v.empty());
        assert(v.size() >= m.size());

        auto N = v.size();

        if (complement) {
            BOOST_COMPUTE_CLOSURE(void, assignZero, (unsigned int rowId), (v), { v[rowId] = 0u; });
            compute::for_each(m.begin(), m.end(), assignZero, queue);
        } else {
            compute::vector<Index> masked(N, queue.get_context());
            BOOST_COMPUTE_CLOSURE(void, assignOne, (unsigned int rowId), (masked, v), { masked[rowId] = v[rowId]; });
            compute::fill(masked.begin(), masked.end(), 0u, queue);
            compute::for_each(m.begin(), m.end(), assignOne, queue);
            std::swap(masked, v);
        }
    }

    /**
     * @brief Apply to dense vector dense mask
     *
     * @param v Dense vector to mask
     * @param m Dense vector mask
     * @param complement True if complement mask
     * @param queue Queue to perform operation on
     */
    inline void ApplyMaskDenseDense(boost::compute::vector<Index> &v,
                                    const boost::compute::vector<Index> &m,
                                    bool complement,
                                    boost::compute::command_queue &queue) {
        using namespace boost;

        assert(!v.empty());
        assert(v.size() == m.size());

        auto N = v.size();

        if (complement) {
            // Inverse mask application (remember, stencil value either 1 or 0)
            // in  |      rs | x0 x1 x0 x1 x0 x0 x0 x1 |
            // in  |      st | x0 x0 x1 x1 x0 x1 x1 x0 |
            // tmp | ff + st | ff ff 00 00 00 00 00 ff |
            // out |      rs | x0 x1 x0 x0 x0 x0 x0 x1 |
            BOOST_COMPUTE_CLOSURE(void, maskInverse, (unsigned int i), (v, m), { v[i] = v[i] & (0xffffffffu + m[i]); });
            compute::for_each(compute::counting_iterator<unsigned int>(0), compute::counting_iterator<unsigned int>(N), maskInverse, queue);
        } else {
            BOOST_COMPUTE_CLOSURE(void, maskDirect, (unsigned int i), (v, m), { v[i] = v[i] & m[i]; });
            compute::for_each(compute::counting_iterator<unsigned int>(0), compute::counting_iterator<unsigned int>(N), maskDirect, queue);
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAAPPLYMASK_HPP
