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

#include <spla/algorithm.hpp>
#include <spla/op.hpp>
#include <spla/schedule.hpp>
#include <spla/timer.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
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
        int             front_size     = -1;
        int             discovered     = 1;
        bool            frontier_empty = false;

        ref_ptr<Descriptor> desc = make_desc();
        desc->set_early_exit(true);
        desc->set_struct_only(true);

        frontier_prev->set_int(s, 1);

        bool  push              = descriptor->get_push_only();
        bool  pull              = descriptor->get_pull_only();
        bool  push_pull         = descriptor->get_push_pull();
        float front_factor      = descriptor->get_front_factor();
        float discovered_factor = descriptor->get_discovered_factor();

        if (!(push || pull || push_pull)) push = true;

#ifndef SPLA_RELEASE
        std::string mode;
        if (push_pull) mode = "(push_pull " + std::to_string(front_factor * 100.0f) + "% " + std::to_string(discovered_factor * 100.0f) + "%)";
        if (pull) mode = "(pull)";
        if (push) mode = "(push)";

        std::cout << "start bfs from " << s << " " << mode << std::endl;

        Timer tight;
#endif
        while (!frontier_empty) {
#ifndef SPLA_RELEASE
            tight.start();
#endif
            depth->set_int(current_level);
            exec_v_assign_masked(v, frontier_prev, depth, SECOND_INT, NQZERO_INT);

            float front_density      = float(front_size) / float(N);
            float discovered_density = float(discovered) / float(N);
            bool  is_push_better     = (front_density <= front_factor);
            if (push || (push_pull && is_push_better)) {
                exec_vxm_masked(frontier_new, v, frontier_prev, A, BAND_INT, BOR_INT, EQZERO_INT, zero, desc);
            } else {
                exec_mxv_masked(frontier_new, v, A, frontier_prev, BAND_INT, BOR_INT, EQZERO_INT, zero, desc);
            }

            exec_v_reduce(frontier_size, zero, frontier_new, PLUS_INT, desc);
            frontier_size->get_int(front_size);

#ifndef SPLA_RELEASE
            tight.stop();
            std::cout << " - iter " << current_level
                      << " front " << front_size << " discovered " << discovered << " "
                      << tight.get_elapsed_ms() << " ms" << std::endl;
            get_library()->time_profile_dump();
            get_library()->time_profile_reset();
#endif
            frontier_empty = front_size == 0;
            discovered += front_size;
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
