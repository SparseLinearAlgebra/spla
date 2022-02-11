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
#include <spla-cpp/SplaDescriptor.hpp>
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
     * As an input algorithm expects undirected graph in form of adjacency matrix
     * with values of type int32_t with 1 in the cells (i,j) when graph has edge (i,j).
     * Adjacency matrix must passed as lower and upper triangular parts @p spL and @p spU.
     *
     * As an output algorithm produces number of triangles @p ntrins in the graph
     * as well as buffer matrix @p spB with triangles per vertex.
     *
     * @param[out] ntris Number of triangles in the graph
     * @param[out] spB  Matrix b with number of triangles per vertex
     * @param[in] spL Lower triangular part of adjacency matrix
     * @param[in] spU Upper triangular part of adjacency matrix
     * @param[in] descriptor Algorithm descriptor
     */
    SPLA_API void Tc(std::int32_t &ntrins, RefPtr<Matrix> &spB, const RefPtr<Matrix> &spL, const RefPtr<Matrix> &spU, const RefPtr<Descriptor> &descriptor);

    /**
     * @brief Triangle counting
     *
     * As an input expects undirected graph.
     * Triangle is a set of edges:
     *      - a <-> b
     *      - b <-> c
     *      - c <-> a
     * and in the resulting matrix in the position [a, b]
     * will be the number of such triangles. But
     * total number of triangles @p ntrins will not be the sum of
     * elements of @p spB. Instead, it will be the sum divided by 6,
     * because each triangle was counted 6 times.
     *
     * @param[out] ntris Number of triangles in the graph
     * @param[out] spB  Matrix b with number of triangles per vertex
     * @param[in] spA Input adjacency matrix of directed graph with values of type int32_t,
     *                with 1 stored where has edge between i and j
     */
    SPLA_API void Tc(std::int32_t &ntrins, RefPtr<Matrix> &spB, const RefPtr<Matrix> &spA);

    /**
     * @brief Triangle counting
     *
     * As an input expects undirected graph.
     * Triangle is a set of edges:
     *      - a <-> b
     *      - b <-> c
     *      - c <-> a
     * and in the resulting matrix in the position [a, b]
     * will be the number of such triangles. But
     * total number of triangles @p ntrins will not be the sum of
     * elements of @p spB. Instead, it will be the sum divided by 6,
     * because each triangle was counted 6 times.
     *
     * @note Naive cpu reference algo implementation for correctness only
     *
     * @param[out] ntris Number of triangles in the graph
     * @param[out] B Matrix b with number of triangles per vertex
     * @param[in] A Input adjacency matrix of directed graph with values of type int32_t,
     *              with 1 stored where has edge between i and j
     */
    SPLA_API void Tc(std::int32_t &ntrins, RefPtr<HostMatrix> &B, const RefPtr<HostMatrix> &A);

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAALGOTC_HPP
