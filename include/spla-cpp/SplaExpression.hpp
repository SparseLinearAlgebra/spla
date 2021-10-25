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

#ifndef SPLA_SPLAEXPRESSION_HPP
#define SPLA_SPLAEXPRESSION_HPP

#include <atomic>
#include <memory>
#include <spla-cpp/SplaData.hpp>
#include <spla-cpp/SplaDescriptor.hpp>
#include <spla-cpp/SplaExpressionNode.hpp>
#include <spla-cpp/SplaFunctionBinary.hpp>
#include <spla-cpp/SplaMatrix.hpp>
#include <spla-cpp/SplaObject.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <spla-cpp/SplaVector.hpp>
#include <vector>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class Expression
     * @brief Computational expression for submission.
     *
     * Represents single self-sufficient set of computational nodes with dependencies,
     * which can be submitted for the execution at once and then asynchronously
     * checked state of computation of waited (blocked) until completion.
     *
     * @warning Expression cannot be modified after submission.
     * @warning Expression::Wait() is valid call only for submitted expressions.
     *
     * @note Call Expression::Wait() to ensure completion.
     * @note Call Expression::GetState() to get current state to ensure, that expression is not running.
     * @note If expression is released, it implicitly calls Expression::Wait() internally before destruction.
     */
    class SPLA_API Expression final : public Object {
    public:
        ~Expression() override;

        /** Current expression state */
        enum class State {
            /** Default expression state after creation */
            Default,
            /** Expression submitted for evaluation */
            Submitted,
            /** Expression successfully evaluated after submission */
            Evaluated,
            /** Expression evaluation aborted due some error/exception */
            Aborted
        };

        /**
         * Wait for expression until it is computation is completed.
         *
         * @note Blocks current thread until evaluated or aborted.
         * @note Must be called only after expression is submitted.
         * @note Check expression state after completion to ensure correctness.
         *
         * @see Expression::GetState()
         */
        void Wait();

        /**
         * Makes dependency between provided expression nodes.
         * Next `succ` node will be evaluated only after `pred` node evaluation is finished.
         *
         * @param pred Expression node
         * @param succ Expression node, which evaluation depends on `pred` node.
         */
        void Dependency(const RefPtr<ExpressionNode> &pred, const RefPtr<ExpressionNode> &succ);

        /**
         * Make user matrix data write to the provided matrix object.
         * Matrix data is supposed to be stored as array of (i, j, values) pairs.
         *
         * @note By default values are automatically sorted and duplicates reduces (keep first entry)
         * @note Use descriptor `ValuesSorted` hint if values already sorted in row-column order.
         * @note Use descriptor `NoDuplicates` hint if values already has no duplicates
         *
         * @param matrix Matrix to write data
         * @param data Raw host data to write
         * @param desc Operation descriptor
         *
         * @return Created expression node
         */
        RefPtr<ExpressionNode> MakeDataWrite(const RefPtr<Matrix> &matrix,
                                             const RefPtr<DataMatrix> &data,
                                             const RefPtr<Descriptor> &desc = nullptr);

        /**
         * Make user vector data write to the provided vector object.
         * Vector data is supposed to be stored as array of (i, values) pairs.
         *
         * @note By default values are automatically sorted and duplicates reduces (keep first entry)
         * @note Use descriptor `ValuesSorted` hint if values already sorted in row order.
         * @note Use descriptor `NoDuplicates` hint if values already has no duplicates
         *
         * @param vector Vector to write data
         * @param data Raw host data to write
         * @param desc Operation descriptor
         *
         * @return Created expression node
         */
        RefPtr<ExpressionNode> MakeDataWrite(const RefPtr<Vector> &vector,
                                             const RefPtr<DataVector> &data,
                                             const RefPtr<Descriptor> &desc = nullptr);

        /**
         * Make matrix data read from the provided matrix object.
         * Matrix data is will be stored as array of (i, j, values) pairs.
         *
         * The data arrays must be provided by the user and the size of this arrays must
         * be greater or equal the values count of the matrix.
         *
         * @param matrix Matrix to read data
         * @param data Host buffers to store data
         * @param desc Operation descriptor
         *
         * @return Created expression node
         */
        RefPtr<ExpressionNode> MakeDataRead(const RefPtr<Matrix> &matrix,
                                            const RefPtr<DataMatrix> &data,
                                            const RefPtr<Descriptor> &desc = nullptr);

        /**
         * Make vertex data read from the provided vector object.
         * Vector data is will be stored as array of (i, values) pairs.
         *
         * The data arrays must be provided by the user and the size of this arrays must
         * be greater or equal the values count of the vector.
         *
         * @param vector Vector to read data
         * @param data Host buffers to store data
         * @param desc Operation descriptor
         *
         * @return Created expression node
         */
        RefPtr<ExpressionNode> MakeDataRead(const RefPtr<Vector> &vector,
                                            const RefPtr<DataVector> &data,
                                            const RefPtr<Descriptor> &desc = nullptr);

        /**
         * Make vector masked element-wise add expression node.
         * Operation is evaluated as `w<mask> = a +(op) b`.
         *
         * @param w Vector to store result
         * @param mask Mask to filter input vectors; may be null
         * @param op Binary op to sum elements of input vectors
         * @param a Input a vector
         * @param b Input b vector
         * @param desc Operation descriptor
         *
         * @return Created expression node
         */
        RefPtr<ExpressionNode> MakeEWiseAdd(const RefPtr<Vector> &w,
                                            const RefPtr<Vector> &mask,
                                            const RefPtr<FunctionBinary> &op,
                                            const RefPtr<Vector> &a,
                                            const RefPtr<Vector> &b,
                                            const RefPtr<Descriptor> &desc = nullptr);

        /** @return Current expression state */
        State GetState() const;

        /** @return True if this is empty expression (has no nodes for computation) */
        bool Empty() const;

        /** @return Nodes of this expression */
        const std::vector<RefPtr<class ExpressionNode>> &GetNodes() const;

        /**
         * Creates new expression.
         *
         * @param library Global library state
         *
         * @return Created expression
         */
        static RefPtr<Expression> Make(class Library &library);

    private:
        friend class ExpressionManager;
        friend class TaskBuilder;
        explicit Expression(class Library &library);

        RefPtr<ExpressionNode> MakeNode(ExpressionNode::Operation op,
                                        std::vector<RefPtr<Object>> &&args,
                                        const RefPtr<Descriptor> &desc);

        void SetState(State state);
        void SetFuture(std::unique_ptr<class ExpressionFuture> &&future);
        void SetTasks(std::unique_ptr<class ExpressionTasks> &&tasks);

        std::vector<RefPtr<class ExpressionNode>> mNodes;
        std::unique_ptr<class ExpressionFuture> mFuture;
        std::unique_ptr<class ExpressionTasks> mTasks;
        std::atomic<State> mState;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAEXPRESSION_HPP