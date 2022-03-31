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

#ifndef SPLA_BFS_HPP
#define SPLA_BFS_HPP

#include <cassert>
#include <cmath>
#include <numeric>
#include <optional>
#include <queue>

#include <spla/descriptor.hpp>
#include <spla/expression.hpp>
#include <spla/matrix.hpp>
#include <spla/vector.hpp>

namespace spla {

    inline int bfs(Vector<int> &v,
                   const Matrix<int> &A,
                   Index source,
                   const Descriptor &descriptor = Descriptor()) {
        assert(A.nrows() == A.ncols());
        assert(A.nrows() == v.nrows());
        assert(source < v.nrows());

        auto N = A.nrows();

        v.clear();
        Vector<int> q(N);

        Expression init_q;
        init_q.build(q, NullOp(), {source}, {1});
        init_q.submit().wait();

        int depth = 0;

        using multiply_op = binary_op::lor<int>;
        using reduce_op = binary_op::land<int>;

        Descriptor vxm_desc;
        vxm_desc.scmp() = true;
        vxm_desc.replace() = true;

        while (!q.empty()) {
            depth += 1;
            spla::Expression traverse_step;
            auto t1_mark_reached = traverse_step.assign(v, std::make_optional(q), NullOp(), depth);
            auto t2_update_front = traverse_step.vxm(q, std::make_optional(v), NullOp(), multiply_op(), reduce_op(), q, A, vxm_desc);
            t1_mark_reached->precede(t2_update_front);
            traverse_step.submit().wait();
        }

        return depth;
    }

    inline int bfs_naive(Vector<int> &v,
                         const Matrix<int> &A,
                         Index source,
                         const Descriptor &descriptor = Descriptor()) {
        assert(A.nrows() == A.ncols());
        assert(A.nrows() == v.nrows());
        assert(source < v.nrows());

        auto N = A.nrows();
        v.clear();

        std::vector<Index> graph_src;
        std::vector<Index> graph_dst;

        Expression read_graph;
        read_graph.read(A, [&](auto &a, auto &b, auto &) { graph_src = std::move(a); graph_dst = std::move(b); });
        read_graph.submit().wait();

        std::vector<Index> lengths(N, 0);
        std::vector<Index> offsets(N);
        for (auto i : graph_src) lengths[i] += 1;
        std::exclusive_scan(lengths.begin(), lengths.end(), offsets.begin(), Index{0});

        std::queue<Index> front;
        std::vector<int> depths(N, -1);
        depths[source] = 1;
        front.push(source);

        int max_depth = 1;

        while (!front.empty()) {
            auto i = front.front();
            front.pop();

            for (Index k = offsets[i]; k < offsets[i] + lengths[i]; k++) {
                auto j = graph_dst[k];

                if (depths[j] == -1) {
                    depths[j] = depths[i] + 1;
                    front.push(j);
                    max_depth = std::max(depths[j], max_depth);
                }
            }
        }

        std::vector<Index> v_rows;
        std::vector<int> v_values;
        for (Index i = 0; i < N; i++) {
            if (depths[i] != -1) {
                v_rows.push_back(i);
                v_values.push_back(depths[i]);
            }
        }

        Expression build_v;
        build_v.build(v, NullOp(), std::move(v_rows), std::move(v_values));
        build_v.submit().wait();

        return max_depth;
    }

}// namespace spla

#endif//SPLA_BFS_HPP
