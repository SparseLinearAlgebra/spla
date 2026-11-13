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
#include <map>
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

        ref_ptr<Descriptor> desc = Descriptor::make();
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
        assert(p);
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

#pragma region Tc

    Status tc(
            int&                       ntrins,
            const ref_ptr<Matrix>&     A,
            const ref_ptr<Matrix>&     B,
            const ref_ptr<Descriptor>& descriptor) {
        assert(A);
        assert(B);

        ref_ptr<Scalar> zero   = Scalar::make_int(0);
        ref_ptr<Scalar> result = Scalar::make(INT);

#ifndef SPLA_RELEASE
        std::cout << "start tc" << std::endl;

        Timer tight;
        tight.start();
#endif

        spla::exec_mxmT_masked(B, A, A, A, MULT_INT, PLUS_INT, GTZERO_INT, zero);
        spla::exec_m_reduce(result, zero, B, PLUS_INT);

        ntrins = result->as_int();

#ifndef SPLA_RELEASE
        tight.stop();

        std::cout << " - ntrins " << ntrins
                  << " " << tight.get_elapsed_ms() << " ms" << std::endl;

        Library::get()->time_profile_dump();
        Library::get()->time_profile_reset();
#endif

        return Status::Ok;
    }

    Status tc_naive(
            int&                                  ntrins,
            std::vector<std::vector<spla::uint>>& Ai,
            const ref_ptr<Descriptor>&            descriptor) {

        ntrins = 0;

        for (const auto& row_Ai : Ai) {
            for (const auto neighbor : row_Ai) {
                const auto& row_neighbor = Ai[neighbor];

                auto it1 = row_Ai.begin();
                auto it2 = row_neighbor.begin();

                auto end1 = row_Ai.end();
                auto end2 = row_neighbor.end();

                while (it1 != end1 && it2 != end2) {
                    if (*it1 == *it2) {
                        ++ntrins;
                        ++it1;
                        ++it2;
                    } else if (*it1 < *it2) {
                        ++it1;
                    } else {
                        ++it2;
                    }
                }
            }
        }

        return Status::Ok;
    }

#pragma endregion Pr

#pragma region Multi-source BFS

    Status msl_bfs(const ref_ptr<Matrix>& S, const ref_ptr<Matrix>& A, const std::vector<uint>& sources, const ref_ptr<Descriptor>& descriptor) {
        assert(S);
        assert(A);

        const uint n = A->get_n_rows();
        const uint k = static_cast<uint>(sources.size());

        ref_ptr<Scalar> zero = Scalar::make_int(0);

        std::vector<std::vector<uint>> F(k, std::vector<uint>(n, 0));
        for (uint t = 0; t < k; ++t) {
            F[t][sources[t]] = 1;
        }

        uint level = 1;
        while (true) {
            bool any = false;
            for (uint t = 0; t < k; ++t) {
                for (uint j = 0; j < n; ++j) {
                    if (F[t][j]) {
                        S->set_int(t, j, static_cast<int>(level));
                        any = true;
                    }
                }
            }
            if (!any) break;

            ref_ptr<Matrix> F_mat = Matrix::make(k, n, INT);
            for (uint t = 0; t < k; ++t) {
                for (uint j = 0; j < n; ++j) {
                    if (F[t][j]) F_mat->set_int(t, j, 1);
                }
            }

            ref_ptr<Matrix> F_new = Matrix::make(k, n, INT);
            exec_mxm(F_new, F_mat, A, LAND_INT, LOR_INT, zero);

            for (uint t = 0; t < k; ++t) {
                for (uint j = 0; j < n; ++j) {
                    int fv;
                    int sv;
                    F_new->get_int(t, j, fv);
                    S->get_int(t, j, sv);
                    F[t][j] = (fv != 0 && sv == 0) ? 1 : 0;
                }
            }

            level += 1;
        }

        return Status::Ok;
    }

    Status msp_bfs(const ref_ptr<Matrix>& P, const ref_ptr<Matrix>& A, const std::vector<uint>& sources, const ref_ptr<Descriptor>& descriptor) {
        assert(P);
        assert(A);

        const uint n = A->get_n_rows();
        const uint k = static_cast<uint>(sources.size());

        ref_ptr<Scalar> inf = Scalar::make_int(std::numeric_limits<int>::max());

        std::vector<std::vector<int>>  P_host(k, std::vector<int>(n, -1));
        std::vector<std::vector<uint>> F(k, std::vector<uint>(n, 0));
        for (uint t = 0; t < k; ++t) {
            F[t][sources[t]]      = 1;
            P_host[t][sources[t]] = 0;
        }

        bool any = true;
        while (any) {
            ref_ptr<Matrix> F_mat = Matrix::make(k, n, INT);
            for (uint t = 0; t < k; ++t) {
                for (uint i = 0; i < n; ++i) {
                    if (F[t][i]) F_mat->set_int(t, i, static_cast<int>(i) + 1);
                }
            }

            ref_ptr<Matrix> F_new = Matrix::make(k, n, INT);
            exec_mxm(F_new, F_mat, A, FIRST_INT, MIN_INT, inf);

            any = false;
            for (uint t = 0; t < k; ++t) {
                for (uint j = 0; j < n; ++j) {
                    F[t][j] = 0;
                }
            }

            for (uint t = 0; t < k; ++t) {
                for (uint j = 0; j < n; ++j) {
                    int fv;
                    F_new->get_int(t, j, fv);
                    if (fv != 0 && P_host[t][j] == -1) {
                        P_host[t][j] = fv - 1;
                        F[t][j]      = 1;
                        any          = true;
                    }
                }
            }
        }

        for (uint t = 0; t < k; ++t) {
            for (uint j = 0; j < n; ++j) {
                P->set_int(t, j, P_host[t][j]);
            }
        }

        return Status::Ok;
    }

    Status msp_bfs_naive(std::vector<std::vector<int>>&        P,
                         const std::vector<std::vector<uint>>& A,
                         const std::vector<uint>&              sources,
                         const ref_ptr<Descriptor>&            descriptor) {
        const uint n = static_cast<uint>(A.size());
        const uint k = static_cast<uint>(sources.size());

        P.assign(k, std::vector<int>(n, -1));

        for (uint t = 0; t < k; ++t) {
            std::vector<int>  parent(n, -1);
            std::vector<uint> frontier = {sources[t]};
            parent[sources[t]]         = 0;

            while (!frontier.empty()) {
                std::map<uint, uint> next;
                for (uint u : frontier) {
                    for (uint v : A[u]) {
                        if (parent[v] == -1) {
                            auto it = next.find(v);
                            if (it == next.end() || u < it->second) next[v] = u;
                        }
                    }
                }

                std::vector<uint> next_frontier;
                next_frontier.reserve(next.size());
                for (const auto& [v, p] : next) {
                    parent[v] = static_cast<int>(p);
                    next_frontier.push_back(v);
                }
                frontier = std::move(next_frontier);
            }

            P[t] = std::move(parent);
        }

        return Status::Ok;
    }

    Status msl_bfs_naive(std::vector<std::vector<int>>&        S,
                         const std::vector<std::vector<uint>>& A,
                         const std::vector<uint>&              sources,
                         const ref_ptr<Descriptor>&            descriptor) {
        const uint n = static_cast<uint>(A.size());
        const uint k = static_cast<uint>(sources.size());

        S.assign(k, std::vector<int>(n, 0));

        for (uint t = 0; t < k; ++t) {
            std::vector<int> level(n, 0);
            std::queue<uint> q;
            q.push(sources[t]);
            level[sources[t]] = 1;

            while (!q.empty()) {
                const uint u = q.front();
                q.pop();
                for (uint v : A[u]) {
                    if (level[v] == 0) {
                        level[v] = level[u] + 1;
                        q.push(v);
                    }
                }
            }

            S[t] = std::move(level);
        }

        return Status::Ok;
    }

#pragma endregion Multi - source BFS

#pragma region Reach RPQ

    Status reach_rpq(const ref_ptr<Vector>&              v,
                     const std::vector<ref_ptr<Matrix>>& A,
                     const std::vector<ref_ptr<Matrix>>& R,
                     uint                                source,
                     const std::vector<uint>&            start_states,
                     const std::vector<uint>&            final_states,
                     const ref_ptr<Descriptor>&          descriptor) {
        assert(v);
        assert(A.size() == R.size());

        const uint nl      = static_cast<uint>(A.size());
        const uint n       = v->get_n_rows();
        const uint nstates = R[0]->get_n_rows();

        ref_ptr<Scalar> zero = Scalar::make_int(0);

        std::vector<std::vector<uint>> visited(nstates, std::vector<uint>(n, 0));
        std::vector<std::vector<uint>> frontier(nstates, std::vector<uint>(n, 0));
        for (uint p : start_states) {
            frontier[p][source] = 1;
            visited[p][source]  = 1;
        }

        for (;;) {
            bool empty = true;
            for (uint p = 0; p < nstates && empty; ++p) {
                for (uint j = 0; j < n && empty; ++j) {
                    if (frontier[p][j]) empty = false;
                }
            }
            if (empty) break;

            ref_ptr<Matrix> F = Matrix::make(nstates, n, INT);
            for (uint p = 0; p < nstates; ++p) {
                for (uint j = 0; j < n; ++j) {
                    if (frontier[p][j]) F->set_int(p, j, 1);
                }
            }

            ref_ptr<Matrix> next = Matrix::make(nstates, n, INT);

            for (uint i = 0; i < nl; ++i) {
                ref_ptr<Matrix> Rt = Matrix::make(nstates, nstates, INT);
                exec_m_transpose(Rt, R[i], IDENTITY_INT);

                ref_ptr<Matrix> symbol = Matrix::make(nstates, n, INT);
                exec_mxm(symbol, Rt, F, LAND_INT, LOR_INT, zero);

                ref_ptr<Matrix> tmp = Matrix::make(nstates, n, INT);
                exec_mxm(tmp, symbol, A[i], LAND_INT, LOR_INT, zero);

                ref_ptr<Matrix> merged = Matrix::make(nstates, n, INT);
                exec_m_eadd(merged, next, tmp, LOR_INT);
                next = merged;
            }

            // mask by not-visited and rebuild the frontier
            for (uint p = 0; p < nstates; ++p) {
                for (uint j = 0; j < n; ++j) {
                    int nv;
                    next->get_int(p, j, nv);
                    frontier[p][j] = (nv != 0 && !visited[p][j]) ? 1 : 0;
                    if (frontier[p][j]) visited[p][j] = 1;
                }
            }
        }

        for (uint p : final_states) {
            for (uint j = 0; j < n; ++j) {
                if (visited[p][j]) {
                    v->set_int(j, 1);
                }
            }
        }

        return Status::Ok;
    }
#pragma endregion Reach RPQ

#pragma region Single-path CFPQ

    Status single_path_cfpq(std::vector<ref_ptr<Matrix>>&             T,
                            const std::vector<ref_ptr<Matrix>>&       adj_matrices,
                            const std::vector<std::array<uint, 3>>&   binary_rules,
                            const std::vector<std::pair<uint, uint>>& terminal_rules,
                            const std::vector<uint>&                  epsilon_nonterms,
                            uint                                      nonterms,
                            const ref_ptr<Descriptor>&                descriptor) {
        const uint n = adj_matrices[0]->get_n_rows();

        T.clear();
        T.resize(nonterms);
        for (uint k = 0; k < nonterms; ++k) {
            T[k] = Matrix::make(n, n, PATH_INDEX);
        }

        ref_ptr<Scalar> bottom = Scalar::make(PATH_INDEX);
        bottom->set_path_index(PathIndex(0, 0));

        // terminal rules: A -> term
        for (const auto& [nt, term] : terminal_rules) {
            const auto& M = adj_matrices[term];
            for (uint i = 0; i < n; ++i) {
                for (uint j = 0; j < n; ++j) {
                    int mv;
                    M->get_int(i, j, mv);
                    if (mv != 0) {
                        T[nt]->set_path_index(i, j, PathIndex(i, 1));
                    }
                }
            }
        }

        // epsilon rules: A -> eps
        for (uint nt : epsilon_nonterms) {
            for (uint i = 0; i < n; ++i) {
                T[nt]->set_path_index(i, i, PathIndex(i, 1));
            }
        }

        // fixed point: iterate until a full pass over the binary rules adds no
        // new (i, j) entry to any nonterminal matrix (nnz-based change detection,
        // as in the LAGraph reference). The matrices only grow, and each entry
        // first appears with its minimal derivation height, so the fixed point
        // is reached when the nnz stops growing.
        auto nnz = [](const ref_ptr<Matrix>& M) {
            ref_ptr<MemView> k1, k2, v;
            M->read(k1, k2, v);
            return static_cast<uint>(k1->get_size() / sizeof(uint));
        };

        for (;;) {
            bool changed = false;

            for (const auto& [lhs, rhs1, rhs2] : binary_rules) {
                const uint before = nnz(T[lhs]);

                ref_ptr<Matrix> prod = Matrix::make(n, n, PATH_INDEX);
                exec_mxm(prod, T[rhs1], T[rhs2], MUL_PATH_INDEX, MIN_PATH_INDEX, bottom);

                ref_ptr<Matrix> merged = Matrix::make(n, n, PATH_INDEX);
                exec_m_eadd(merged, T[lhs], prod, MIN_PATH_INDEX);
                T[lhs] = merged;

                const uint after = nnz(T[lhs]);
                changed          = changed || (before != after);
            }

            if (!changed) break;
        }

        return Status::Ok;
    }

#pragma endregion Single - path CFPQ

}// namespace spla
