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

#ifndef SPLA_SPLAEXPRESSIONMANAGER_HPP
#define SPLA_SPLAEXPRESSIONMANAGER_HPP

#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaExpression.hpp>
#include <spla-cpp/SplaExpressionNode.hpp>
#include <expression/SplaNodeProcessor.hpp>
#include <unordered_map>

namespace spla {

    class ExpressionManager final: public RefCnt {
    public:
        explicit ExpressionManager(Library& library);
        ~ExpressionManager() override = default;

        void Submit(const RefPtr<Expression> &expression);
        void Register(const RefPtr<NodeProcessor> &processor);

    private:
        RefPtr<NodeProcessor> SelectProcessor(const RefPtr<ExpressionNode> &node);

    private:
        using ProcessorList = std::vector<RefPtr<NodeProcessor>>;
        using ProcessorMap = std::unordered_map<ExpressionNode::Operation, ProcessorList>;
        ProcessorMap mProcessors;

        Library& mLibrary;
    };

}

#endif //SPLA_SPLAEXPRESSIONMANAGER_HPP
