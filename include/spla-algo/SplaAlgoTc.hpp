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
#ifndef SPLA_SPLAALGOTC_HPP
#define SPLA_SPLAALGOTC_HPP

#include <spla-algo/SplaAlgoCommon.hpp>
#include <spla-cpp/SplaMatrix.hpp>
#include <spla-cpp/SplaVector.hpp>

namespace spla {

    /**
     * @addtogroup Algorithm
     * @{
     */

    /**
     * @brief Triangle counting
     *
     * @note If graph is
     *  - Directed, then a triangle is a set of edges
     *      - a -> b
     *      - a -> c
     *      - b -> c
     *   and in the resulting matrix @p spB in the position [a, b]
     *   will be the number of such triangles
     *   And in the @p ntrins will be total number of triangles.
     *
     *  - Undirected, than a triangle is a set of edges
     *      - a <-> b
     *      - b <-> c
     *      - c <-> a
     *   and in the resulting matrix in the position [a, b]
     *   will be the number of such triangles. But
     *   total number of triangles @p ntrins will not be the sum of
     *   elements of @p spB. Instead, it will be the sum divided by 6,
     *   because each triangle was counted 6 times.
     *
     *
     * @param [out] ntris - number of triangles in the graph
     * @param [out] spB - matrix b with number of triangles per vertex
     * @param [in]  spA - input adjacency matrix of directed graph with values of type int32_t,
     *                   with 1 stored where has edge between i and j
     * @param [in]  dir - is input graph directed or not
     */
    SPLA_API void Tc(std::int32_t &ntrins, RefPtr<Matrix> &spB, const RefPtr<Matrix> &spA, bool dir);

    /**
     * @brief Triangle counting
     *
     * @note If graph is
     *  - Directed, then a triangle is a set of edges
     *      - a -> b
     *      - a -> c
     *      - b -> c
     *   and in the resulting matrix @p spB in the position [a, b]
     *   will be the number of such triangles
     *   And in the @p ntrins will be total number of triangles.
     *
     *  - Undirected, than a triangle is a set of edges
     *      - a <-> b
     *      - b <-> c
     *      - c <-> a
     *   and in the resulting matrix in the position [a, b]
     *   will be the number of such triangles. But
     *   total number of triangles @p ntrins will not be the sum of
     *   elements of @p spB. Instead, it will be the sum divided by 6,
     *   because each triangle was counted 6 times.
     *
     *
     * @note Naive cpu reference algo implementation for correctness only
     *
     * @param [out] ntris - number of triangles in the graph
     * @param [out] B - matrix b with number of triangles per vertex
     * @param [in]  A - input adjacency matrix of directed graph with values of type int32_t,
     *                 with 1 stored where has edge between i and j
     * @param [in]  dir - is input graph directed or not
     */
    SPLA_API void Tc(std::int32_t &ntrins, RefPtr<HostMatrix> &B, const RefPtr<HostMatrix> &A, bool dir);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAALGOTC_HPP
