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
    SPLA_API Status bfs(const ref_ptr<Vector>&     v,
                        const ref_ptr<Matrix>&     A,
                        uint                       s,
                        const ref_ptr<Descriptor>& descriptor);

    /**
     * @brief Naive breadth-first search algorithm
     *
     * @param v int vector to store reached distances
     * @param A int graph adjacency lists filled with 1 where exist edge from i to j
     * @param s start vertex id to search
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status bfs_naive(std::vector<int>&                     v,
                              std::vector<std::vector<spla::uint>>& A,
                              uint                                  s,
                              const ref_ptr<Descriptor>&            descriptor);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_ALGORITHM_HPP
