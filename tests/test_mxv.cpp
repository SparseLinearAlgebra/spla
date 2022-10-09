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

#include "test_common.hpp"

#include <spla/spla.hpp>

TEST(mxv_masked_comp, naive) {
    spla::uint M = 4, N = 5;

    //            v 3 0 3 0 -1
    //
    // r    r   mask       M
    // ________________________
    // 2 |  2 |  1 | 0 2 0 0 -9
    // 9 | -5 |  0 | 2 0 0 0 -8
    // 4 |  4 |  1 | 0 0 3 0  0
    // 6 |  5 |  0 | 0 0 0 0 -1

    auto ir    = spla::make_vector(M, spla::INT);
    auto imask = spla::make_vector(M, spla::INT);
    auto iv    = spla::make_vector(N, spla::INT);
    auto iM    = spla::make_matrix(M, N, spla::INT);

    ir->set_int(0, 2);
    ir->set_int(1, -5);
    ir->set_int(2, 4);
    ir->set_int(3, 5);

    imask->set_int(0, 1);
    imask->set_int(1, 0);
    imask->set_int(2, 1);
    imask->set_int(3, 0);

    iv->set_int(0, 3);
    iv->set_int(1, 0);
    iv->set_int(2, 3);
    iv->set_int(3, 0);
    iv->set_int(4, -1);

    iM->set_int(0, 1, 2);
    iM->set_int(0, 4, -9);
    iM->set_int(1, 0, 2);
    iM->set_int(1, 4, -8);
    iM->set_int(2, 2, 3);
    iM->set_int(3, 4, -1);

    auto schedule = spla::make_schedule();
    schedule->step_task(spla::make_sched_mxv_masked(ir, imask, iM, iv, spla::MULT_INT, spla::PLUS_INT, true));
    schedule->submit();

    int r;

    ir->get_int(0, r);
    EXPECT_EQ(r, 2);

    ir->get_int(1, r);
    EXPECT_EQ(r, 9);

    ir->get_int(2, r);
    EXPECT_EQ(r, 4);

    ir->get_int(3, r);
    EXPECT_EQ(r, 6);
}

SPLA_GTEST_MAIN_WITH_FINALIZE