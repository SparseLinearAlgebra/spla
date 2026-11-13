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

#include <array>
#include <vector>

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
            const ref_ptr<Descriptor>& descriptor = spla::Descriptor::make());

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
            const ref_ptr<Descriptor>&            descriptor = spla::Descriptor::make());

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
                               const ref_ptr<Descriptor>&       descriptor = spla::Descriptor::make());

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
            const ref_ptr<Descriptor>& descriptor = spla::Descriptor::make());

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
            const ref_ptr<Descriptor>&       descriptor = spla::Descriptor::make());

    /**
     * @brief Triangles counting algorithm
     *
     * @param ntrins Number of triangles counted
     * @param A Lower trilingual int matrix with 1 where has edge in a graph
     * @param B Buffer int matrix to store result
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status tc(
            int&                       ntrins,
            const ref_ptr<Matrix>&     A,
            const ref_ptr<Matrix>&     B,
            const ref_ptr<Descriptor>& descriptor = spla::Descriptor::make());

    /**
     * @brief Naive triangles counting algorithm (reference cpu implementation)
     *
     * @param ntrins Number of triangles counted
     * @param A Lower trilingual int matrix structure
     * @param descriptor optional descriptor for algorithm
     *
     * @return ok on success
     */
    SPLA_API Status tc_naive(
            int&                                  ntrins,
            std::vector<std::vector<spla::uint>>& Ai,
            const ref_ptr<Descriptor>&            descriptor = spla::Descriptor::make());

    /**
     * @brief Multi-source breadth-first search: parents
     *
     * For each source, computes the parent of every reachable vertex in the BFS
     * tree (the smallest parent id on ties). The parent of a source is 0 and an
     * unreachable vertex has no parent (0).
     *
     * @param P int matrix (k x n) to store parents, row t = parents for source t
     * @param A int adjacency matrix (n x n) with 1 where an edge exists
     * @param sources source vertex ids
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status msp_bfs(
            const ref_ptr<Matrix>&     P,
            const ref_ptr<Matrix>&     A,
            const std::vector<uint>&   sources,
            const ref_ptr<Descriptor>& descriptor = spla::Descriptor::make());

    /**
     * @brief Multi-source breadth-first search: levels
     *
     * For each source, computes the BFS level (1-based) of every reachable
     * vertex; an unreachable vertex has level 0.
     *
     * @param S int matrix (k x n) to store levels, row t = levels for source t
     * @param A int adjacency matrix (n x n) with 1 where an edge exists
     * @param sources source vertex ids
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status msl_bfs(
            const ref_ptr<Matrix>&     S,
            const ref_ptr<Matrix>&     A,
            const std::vector<uint>&   sources,
            const ref_ptr<Descriptor>& descriptor = spla::Descriptor::make());

    /**
     * @brief Naive multi-source breadth-first search: parents (reference cpu implementation)
     *
     * @param P int matrix rows of parents, row t = parents for source t (-1 = unreachable)
     * @param A graph adjacency lists
     * @param sources source vertex ids
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status msp_bfs_naive(
            std::vector<std::vector<int>>&        P,
            const std::vector<std::vector<uint>>& A,
            const std::vector<uint>&              sources,
            const ref_ptr<Descriptor>&            descriptor = spla::Descriptor::make());

    /**
     * @brief Naive multi-source breadth-first search: levels (reference cpu implementation)
     *
     * @param S int matrix rows of levels, row t = levels for source t (0 = unreachable)
     * @param A graph adjacency lists
     * @param sources source vertex ids
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status msl_bfs_naive(
            std::vector<std::vector<int>>&        S,
            const std::vector<std::vector<uint>>& A,
            const std::vector<uint>&              sources,
            const ref_ptr<Descriptor>&            descriptor = spla::Descriptor::make());

    /**
     * @brief Single-source regular path query (2-RPQ) reachability
     *
     * Computes the vertices reachable from `source` by paths accepted by the
     * automaton, via BFS over the direct product (graph x NFA).
     *
     * @param v int vector (n) to store 1 for reachable vertices
     * @param A label-decomposed adjacency matrices A[0..labels)
     * @param R label-decomposed NFA transition matrices R[0..labels) (R[l][p][q] = 1)
     * @param source source vertex id
     * @param start_states NFA start states
     * @param final_states NFA final states
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status reach_rpq(
            const ref_ptr<Vector>&              v,
            const std::vector<ref_ptr<Matrix>>& A,
            const std::vector<ref_ptr<Matrix>>& R,
            uint                                source,
            const std::vector<uint>&            start_states,
            const std::vector<uint>&            final_states,
            const ref_ptr<Descriptor>&          descriptor = spla::Descriptor::make());

    /**
     * @brief Single-path context-free path querying
     *
     * Computes, for a graph and a grammar in weak Chomsky normal form, the
     * minimal-derivation-height path index between every pair of vertices for
     * every nonterminal.
     *
     * @param T output PathIndex matrices per nonterminal (n x n)
     * @param adj_matrices label-decomposed adjacency matrices (n x n) per terminal
     * @param binary_rules productions A -> BC (nonterminal ids)
     * @param terminal_rules productions A -> terminal (nonterminal id, terminal id)
     * @param epsilon_nonterms nonterminals with an A -> eps production
     * @param nonterms number of nonterminals
     * @param descriptor optional descriptor
     *
     * @return ok on success
     */
    SPLA_API Status single_path_cfpq(
            std::vector<ref_ptr<Matrix>>&             T,
            const std::vector<ref_ptr<Matrix>>&       adj_matrices,
            const std::vector<std::array<uint, 3>>&   binary_rules,
            const std::vector<std::pair<uint, uint>>& terminal_rules,
            const std::vector<uint>&                  epsilon_nonterms,
            uint                                      nonterms,
            const ref_ptr<Descriptor>&                descriptor = spla::Descriptor::make());

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_ALGORITHM_HPP
