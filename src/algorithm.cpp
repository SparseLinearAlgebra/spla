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

#include <spla/algorithm.hpp>
#include <spla/op.hpp>
#include <spla/schedule.hpp>

#include <algorithm>
#include <cassert>
#include <queue>

namespace spla {

    Status bfs(const ref_ptr<Vector>&     v,
               const ref_ptr<Matrix>&     A,
               uint                       s,
               const ref_ptr<Descriptor>& descriptor) {
        assert(v);
        assert(A);

        const auto N = v->get_n_rows();

        ref_ptr<Vector> frontier_prev  = make_vector(N, INT);
        ref_ptr<Vector> frontier_new   = make_vector(N, INT);
        ref_ptr<Scalar> frontier_size  = make_int(1);
        ref_ptr<Scalar> depth          = make_int(1);
        ref_ptr<Scalar> zero           = make_int(0);
        int             current_level  = 1;
        bool            frontier_empty = false;
        bool            complement     = true;

        frontier_prev->set_int(s, 1);

        while (!frontier_empty) {
            depth->set_int(current_level);

            spla::execute_immediate(make_sched_v_assign_masked(v, frontier_prev, depth, SECOND_INT));
            spla::execute_immediate(make_sched_mxv_masked(frontier_new, v, A, frontier_prev, MULT_INT, OR_INT, zero, complement));
            spla::execute_immediate(make_sched_v_reduce(frontier_size, zero, frontier_new, PLUS_INT));

            int observed_vertices;
            frontier_size->get_int(observed_vertices);

            frontier_empty = observed_vertices == 0;
            current_level += 1;

            std::swap(frontier_prev, frontier_new);
        }

        return Status::Ok;
    }

    Status bfs_naive(std::vector<int>&                     v,
                     std::vector<std::vector<spla::uint>>& A,
                     uint                                  s,
                     const ref_ptr<Descriptor>&            descriptor) {

        const auto N = v.size();

        std::queue<uint>  front;
        std::vector<bool> visited(N, false);

        std::fill(v.begin(), v.end(), 0);

        front.push(s);
        visited[s] = true;
        v[s]       = 1;

        while (!front.empty()) {
            auto i = front.front();
            front.pop();

            for (auto j : A[i]) {
                if (!visited[j]) {
                    visited[j] = true;
                    v[j]       = v[i] + 1;
                    front.push(j);
                }
            }
        }

        return Status::Ok;
    }

}// namespace spla
