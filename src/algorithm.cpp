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

#include <cassert>

namespace spla {

    Status bfs(const ref_ptr<Vector>&     v,
               const ref_ptr<Matrix>&     A,
               uint                       s,
               const ref_ptr<Descriptor>& descriptor) {
        assert(v);
        assert(A);

        auto N = v->get_n_rows();

        ref_ptr<Vector> frontier       = make_vector(N, INT);
        ref_ptr<Scalar> frontier_size  = make_int(1);
        ref_ptr<Scalar> depth          = make_int(1);
        int             current_level  = 1;
        bool            frontier_empty = false;
        bool            complement     = true;

        ref_ptr<Schedule>     bfs_body   = make_schedule();
        ref_ptr<ScheduleTask> bfs_assign = make_sched_v_assign_masked(v, frontier, depth, SECOND_INT);
        ref_ptr<ScheduleTask> bfs_step   = make_sched_mxv_masked(frontier, v, A, frontier, MULT_INT, PLUS_INT, complement);
        ref_ptr<ScheduleTask> bfs_check  = make_sched_v_reduce(frontier_size, frontier, PLUS_INT);
        bfs_body->step_task(bfs_assign);
        bfs_body->step_task(bfs_step);
        bfs_body->step_task(bfs_check);

        frontier->set_int(s, 1);

        while (!frontier_empty) {
            depth->set_int(current_level);

            bfs_body->submit();

            int observed_vertices;
            frontier_size->get_int(observed_vertices);

            frontier_empty = observed_vertices == 0;
            current_level += 1;
        }

        return Status::Ok;
    }

}// namespace spla
