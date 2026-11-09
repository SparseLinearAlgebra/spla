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

#include <spla.hpp>

#include <iostream>

int main() {
    // labels a, b; edges a: 0->1, 2->3; b: 1->2, 1->3.
    auto Aa = spla::Matrix::make(4, 4, spla::INT);
    Aa->set_int(0, 1, 1);
    Aa->set_int(2, 3, 1);

    auto Ab = spla::Matrix::make(4, 4, spla::INT);
    Ab->set_int(1, 2, 1);
    Ab->set_int(1, 3, 1);

    // NFA "ab": q0-a->q1-b->q2(final).
    auto Ra = spla::Matrix::make(3, 3, spla::INT);
    Ra->set_int(0, 1, 1);

    auto Rb = spla::Matrix::make(3, 3, spla::INT);
    Rb->set_int(1, 2, 1);

    auto v = spla::Vector::make(4, spla::INT);
    spla::reach_rpq(v, {Aa, Ab}, {Ra, Rb}, 0, {0}, {2});

    int x;
    for (spla::uint j = 0; j < 4; ++j) {
        v->get_int(j, x);
        std::cout << "reachable(" << j << ") = " << x << std::endl;
    }

    return 0;
}
