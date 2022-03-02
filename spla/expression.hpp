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

#ifndef SPLA_EXPRESSION_HPP
#define SPLA_EXPRESSION_HPP

#include <spla/descriptor.hpp>
#include <spla/expression/expression_node.hpp>
#include <spla/vector.hpp>

namespace spla {

    /**
     * @class Expression
     * @brief Computational expression for compilation
     *
     * Represents single self-sufficient set of computational nodes with dependencies,
     * which can be compiled and submitted for the execution at once and then asynchronously
     * checked state of computation or waited (blocked) until completion.
     */
    class Expression {
    public:
    private:
        template<typename T, typename ReduceOp>
        Ref<ExpressionNode> build(const Vector<T> &vector,
                                  std::vector<Index> rows,
                                  std::vector<T> values,
                                  const Descriptor &descriptor);
    };

}// namespace spla

#endif//SPLA_EXPRESSION_HPP
