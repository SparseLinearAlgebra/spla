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
    // graph: a: 0->1, 1->2, 2->0; b: 0->3, 3->0.
    auto adj_a = spla::Matrix::make(4, 4, spla::INT);
    adj_a->set_int(0, 1, 1);
    adj_a->set_int(1, 2, 1);
    adj_a->set_int(2, 0, 1);

    auto adj_b = spla::Matrix::make(4, 4, spla::INT);
    adj_b->set_int(0, 3, 1);
    adj_b->set_int(3, 0, 1);

    // grammar (nonterminals S=0, S1=1, A=2, B=3): S -> AB | A S1, S1 -> S B, A -> a, B -> b.
    std::vector<std::array<spla::uint, 3>>         binary   = {{0, 2, 3}, {0, 2, 1}, {1, 0, 3}};
    std::vector<std::pair<spla::uint, spla::uint>> terminal = {{2, 0}, {3, 1}};

    std::vector<spla::ref_ptr<spla::Matrix>> T;
    spla::single_path_cfpq(T, {adj_a, adj_b}, binary, terminal, {}, 4);

    spla::PathIndex v;
    T[0]->get_path_index(0, 0, v);
    std::cout << "S[0][0] = {middle=" << v.middle << ", height=" << v.height << "}" << std::endl;

    return 0;
}
