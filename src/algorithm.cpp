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
#include <spla/exec.hpp>
#include <spla/op.hpp>
#include <spla/schedule.hpp>
#include <spla/timer.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>

namespace spla {

#pragma region Bfs

    Status bfs(const ref_ptr<Vector>&     v,
               const ref_ptr<Matrix>&     A,
               uint                       s,
               const ref_ptr<Descriptor>& descriptor) {
        assert(v);
        assert(A);

        const auto N = v->get_n_rows();

        ref_ptr<Vector> frontier_prev  = Vector::make(N, INT);
        ref_ptr<Vector> frontier_new   = Vector::make(N, INT);
        ref_ptr<Scalar> frontier_size  = Scalar::make_int(1);
        ref_ptr<Scalar> depth          = Scalar::make_int(1);
        ref_ptr<Scalar> zero           = Scalar::make_int(0);
        int             current_level  = 1;
        int             discovered     = 1;
        bool            frontier_empty = false;

        ref_ptr<Descriptor> desc = make_desc();
        desc->set_early_exit(true);
        desc->set_struct_only(true);

        frontier_prev->set_int(s, 1);

        bool  push         = descriptor->get_push_only();
        bool  pull         = descriptor->get_pull_only();
        bool  push_pull    = descriptor->get_push_pull();
        float front_factor = descriptor->get_front_factor();

        if (!(push || pull || push_pull)) push = true;

#ifndef SPLA_RELEASE
        std::string mode;
        if (push_pull) mode = "(push_pull " + std::to_string(front_factor * 100.0f) + "%)";
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

            float front_density  = float(frontier_size->as_int()) / float(N);
            bool  is_push_better = (front_density <= front_factor);

            if (push || (push_pull && is_push_better)) {
                exec_vxm_masked(frontier_new, v, frontier_prev, A, BAND_INT, BOR_INT, EQZERO_INT, zero, desc);
            } else {
                exec_mxv_masked(frontier_new, v, A, frontier_prev, BAND_INT, BOR_INT, EQZERO_INT, zero, desc);
            }

            exec_v_count_mf(frontier_size, frontier_new);

#ifndef SPLA_RELEASE
            tight.stop();
            std::cout << " - iter " << current_level
                      << " front " << frontier_size->as_int() << " discovered " << discovered << " "
                      << tight.get_elapsed_ms() << " ms" << std::endl;
            Library::get()->time_profile_dump();
            Library::get()->time_profile_reset();
#endif
            frontier_empty = frontier_size->as_int() == 0;
            discovered += frontier_size->as_int();
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

#pragma endregion Bfs

#pragma region Sssp

    Status sssp(const ref_ptr<Vector>&     v,
                const ref_ptr<Matrix>&     A,
                uint                       s,
                const ref_ptr<Descriptor>& descriptor) {
        assert(v);
        assert(A);

        const auto N   = v->get_n_rows();
        const auto inf = std::numeric_limits<float>::max();

        ref_ptr<Vector> dummy_mask     = Vector::make(N, FLOAT);
        ref_ptr<Vector> frontier       = Vector::make(N, FLOAT);
        ref_ptr<Vector> feedback       = Vector::make(N, FLOAT);
        ref_ptr<Scalar> feedback_size  = Scalar::make_int(0);
        ref_ptr<Scalar> inf_init       = Scalar::make_float(inf);
        int             current_level  = 1;
        bool            feedback_empty = false;

        v->set_fill_value(inf_init);
        feedback->set_fill_value(inf_init);
        frontier->set_fill_value(inf_init);

        v->set_float(s, 0.0f);
        feedback->set_float(s, 0.0f);

        bool  push         = descriptor->get_push_only();
        bool  pull         = descriptor->get_pull_only();
        bool  push_pull    = descriptor->get_push_pull();
        float front_factor = descriptor->get_front_factor();

        if (!(push || pull || push_pull)) push = true;

#ifndef SPLA_RELEASE
        std::string mode;
        if (push_pull) mode = "(push_pull " + std::to_string(front_factor * 100.0f) + "%)";
        if (pull) mode = "(pull)";
        if (push) mode = "(push)";

        std::cout << "start sssp from " << s << " " << mode << std::endl;

        Timer tight;
#endif
        while (!feedback_empty) {
#ifndef SPLA_RELEASE
            tight.start();
#endif
            float front_density  = float(feedback_size->as_int()) / float(N);
            bool  is_push_better = (front_density <= front_factor);

            if (push || (push_pull && is_push_better)) {
                exec_vxm_masked(frontier, dummy_mask, feedback, A, PLUS_FLOAT, MIN_FLOAT, ALWAYS_FLOAT, inf_init);
            } else {
                exec_mxv_masked(frontier, dummy_mask, A, feedback, PLUS_FLOAT, MIN_FLOAT, ALWAYS_FLOAT, inf_init);
            }

            exec_v_eadd_fdb(v, frontier, feedback, MIN_FLOAT);
            exec_v_count_mf(feedback_size, feedback);

#ifndef SPLA_RELEASE
            tight.stop();
            std::cout << " - iter " << current_level
                      << " feed " << feedback_size->as_int()
                      << " " << tight.get_elapsed_ms() << " ms" << std::endl;
            Library::get()->time_profile_dump();
            Library::get()->time_profile_reset();
#endif
            feedback_empty = feedback_size->as_int() == 0;
            current_level += 1;
        }

        return Status::Ok;
    }

    Status sssp_naive(std::vector<float>&              v,
                      std::vector<std::vector<uint>>&  Ai,
                      std::vector<std::vector<float>>& Ax,
                      uint                             s,
                      const ref_ptr<Descriptor>&       descriptor) {

        const auto N   = v.size();
        const auto inf = std::numeric_limits<float>::max();

        std::queue<uint>  front;
        std::vector<bool> in_queue(N, false);
        std::fill(v.begin(), v.end(), inf);

        front.push(s);
        in_queue[s] = true;
        v[s]        = 0.0f;

        while (!front.empty()) {
            auto i = front.front();
            front.pop();
            in_queue[i] = false;

            const auto& col_ids  = Ai[i];
            const auto& col_vals = Ax[i];
            const auto  n_vals   = col_ids.size();

            for (std::size_t k = 0; k < n_vals; k += 1) {
                const uint  j = col_ids[k];
                const float w = col_vals[k];

                if (v[j] == inf || v[i] + w < v[j]) {
                    v[j] = v[i] + w;
                    if (!in_queue[j]) {
                        in_queue[j] = true;
                        front.push(j);
                    }
                }
            }
        }

        return Status::Ok;
    }

#pragma endregion Sssp

#pragma region Pr

    Status pr(ref_ptr<Vector>&           p,
              const ref_ptr<Matrix>&     A,
              float                      alpha,
              float                      eps,
              const ref_ptr<Descriptor>& descriptor) {
        assert(v);
        assert(A);

        const auto N = p->get_n_rows();

        ref_ptr<Vector> dummy_mask = Vector::make(N, FLOAT);
        ref_ptr<Vector> p_prev     = Vector::make(N, FLOAT);
        ref_ptr<Vector> p_tmp      = Vector::make(N, FLOAT);
        ref_ptr<Vector> addition   = Vector::make(N, FLOAT);
        ref_ptr<Vector> errors     = Vector::make(N, FLOAT);
        ref_ptr<Scalar> error2     = Scalar::make(FLOAT);
        ref_ptr<Scalar> zero       = Scalar::make_float(0.0f);

        addition->fill_with(Scalar::make_float((1.0f - alpha) / float(N)));
        p_prev->fill_with(Scalar::make_float(1.0f / float(N)));

        float error = eps + 0.1f;
#ifndef SPLA_RELEASE
        int iter = 0;

        std::cout << "start pr alpha=" << alpha << " eps " << eps << std::endl;

        Timer tight;
#endif
        while (error > eps) {
#ifndef SPLA_RELEASE
            tight.start();
#endif
            // p = A*p + (1-alpha)/N
            exec_mxv_masked(p_tmp, dummy_mask, A, p_prev, MULT_FLOAT, PLUS_FLOAT, ALWAYS_FLOAT, zero);
            exec_v_eadd(p, p_tmp, addition, PLUS_FLOAT);

            // error = sqrt((p[01]-prev[0])^2 + ... + p[N-1]-prev[N-1])^2)
            exec_v_eadd(errors, p, p_prev, MINUS_POW2_FLOAT);
            exec_v_reduce(error2, zero, errors, PLUS_FLOAT);

            error = std::sqrt(error2->as_float());

            std::swap(p, p_prev);

#ifndef SPLA_RELEASE
            tight.stop();
            std::cout << " - iter " << iter++
                      << " error " << error
                      << " " << tight.get_elapsed_ms() << " ms" << std::endl;
            Library::get()->time_profile_dump();
            Library::get()->time_profile_reset();
#endif
        }

        std::swap(p, p_prev);
        return Status::Ok;
    }

    Status pr_naive(std::vector<float>&              p,
                    std::vector<std::vector<uint>>&  Ai,
                    std::vector<std::vector<float>>& Ax,
                    float                            alpha,
                    float                            eps,
                    const ref_ptr<Descriptor>&       descriptor) {

        const auto N = p.size();

        std::vector<float> p_prev(N, 1.0f / float(N));

        float error = eps + 0.1f;

        while (error > eps) {
            for (std::size_t i = 0; i < N; i++) {
                p[i] = 0;

                for (std::size_t k = 0; k < Ai[i].size(); k++) {
                    p[i] += Ax[i][k] * p_prev[Ai[i][k]];
                }

                p[i] += (1.0f - alpha) / float(N);
            }

            error = 0.0f;

            for (std::size_t i = 0; i < N; i++) {
                error += (p[i] - p_prev[i]) * (p[i] - p_prev[i]);
            }

            error = std::sqrt(error);

            std::swap(p, p_prev);
        }

        std::swap(p, p_prev);
        return Status::Ok;
    }

#pragma endregion Pr

}// namespace spla
