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

#ifndef SPLA_ALGORITHM_HPP
#define SPLA_ALGORITHM_HPP

#include "config.hpp"
#include "descriptor.hpp"
#include "matrix.hpp"
#include "scalar.hpp"
#include "vector.hpp"

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @brief Breadth-first search algorithm
     *
     * @param v int vector to store reached distances
     * @param A int matrix filled with 1 where exist edge from i to j
     * @param s start vertex id to search
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status bfs(
            const ref_ptr<Vector>&     v,
            const ref_ptr<Matrix>&     A,
            uint                       s,
            const ref_ptr<Descriptor>& descriptor = ref_ptr<Descriptor>());

    /**
     * @brief Naive breadth-first search algorithm (reference cpu implementation)
     *
     * @param v int vector to store reached distances
     * @param A int graph adjacency lists filled with 1 where exist edge from i to j
     * @param s start vertex id to search
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status bfs_naive(
            std::vector<int>&                     v,
            std::vector<std::vector<spla::uint>>& A,
            uint                                  s,
            const ref_ptr<Descriptor>&            descriptor = ref_ptr<Descriptor>());

    /**
     * @brief Single-source shortest path algorithm
     *
     * @param v float vector to store reached distances
     * @param A float matrix filled with >0.0f distances where exist edge from i to j otherwise 0.0f
     * @param s start vertex id to search
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status sssp(
            const ref_ptr<Vector>&     v,
            const ref_ptr<Matrix>&     A,
            uint                       s,
            const ref_ptr<Descriptor>& descriptor = ref_ptr<Descriptor>());

    /**
     * @brief Naive single-source shortest path algorithm (reference cpu implementation)
     *
     * @param v float vector to store reached distances
     * @param Ai uint matrix column indices
     * @param Ax float matrix values with >0.0f distances where exist edge from i to j
     * @param s start vertex id to search
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status sssp_naive(std::vector<float>&              v,
                               std::vector<std::vector<uint>>&  Ai,
                               std::vector<std::vector<float>>& Ax,
                               uint                             s,
                               const ref_ptr<Descriptor>&       descriptor = ref_ptr<Descriptor>());

    /**
     * @brief PageRank algorithm
     *
     * @param p float vector to store result vertices weights
     * @param A float graph matrix with weights A[i][j] = alpha / outdegree(i)
     * @param alpha float alpha to control PageRank (default is 0.85)
     * @param eps float tolerance to control precision of PageRank (default is 1e-6)
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status pr(
            ref_ptr<Vector>&           p,
            const ref_ptr<Matrix>&     A,
            float                      alpha      = 0.85,
            float                      eps        = 1e-6,
            const ref_ptr<Descriptor>& descriptor = ref_ptr<Descriptor>());

    /**
     * @brief Naive PageRank algorithm (reference cpu implementation)
     *
     * @param p float vector to store result vertices weights
     * @param Ai float graph matrix column indices
     * @param Ax float graph matrix weights A[i][j] = alpha / outdegree(i)
     * @param alpha float alpha to control PageRank (default is 0.85)
     * @param eps float tolerance to control precision of PageRank (default is 1e-6)
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status pr_naive(
            std::vector<float>&              p,
            std::vector<std::vector<uint>>&  Ai,
            std::vector<std::vector<float>>& Ax,
            float                            alpha      = 0.85,
            float                            eps        = 1e-6,
            const ref_ptr<Descriptor>&       descriptor = ref_ptr<Descriptor>());

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_ALGORITHM_HPP
