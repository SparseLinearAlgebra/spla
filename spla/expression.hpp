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

#include <cassert>
#include <functional>
#include <optional>
#include <stdexcept>

#include <spla/descriptor.hpp>
#include <spla/library.hpp>
#include <spla/matrix.hpp>
#include <spla/vector.hpp>

#include <spla/expression_node.hpp>
#include <spla/expression_submission.hpp>

#include <spla/expression/assign.hpp>
#include <spla/expression/build.hpp>
#include <spla/expression/read.hpp>

#define RETURN_NEW_NODE \
    return add_node(new expression::
#define WITH_ARGS(...) \
    (__VA_ARGS__, descriptor, next_id(), *this));

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

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
        /**
         * @class Node
         * @brief Expression node
         */
        using Node = detail::Ref<ExpressionNode>;

        /**
         * @class VectorReadCallback
         * @brief Callback function which can be used to read vector content
         *
         * @tparam T Type of stored values
         */
        template<typename T>
        using VectorReadCallback = std::function<void(const std::vector<Index> &, const std::vector<T> &)>;


        /**
         * @brief Build vector from host data
         *
         * @tparam T Type of values
         * @tparam ReduceOp Binary op of type TxT->T
         *
         * @param vector Vector to build
         * @param reduceOp Reduce op used to reduce duplicates in input data
         * @param rows Vector rows indices
         * @param values Vector values
         * @param descriptor Operation descriptor
         *
         * @return Expression node
         */
        template<typename T,
                 typename ReduceOp>
        Node build(const Vector<T> &vector,
                   ReduceOp reduceOp,
                   std::vector<Index> rows,
                   std::vector<T> values,
                   const Descriptor &descriptor = Descriptor()) {
            RETURN_NEW_NODE BuildVector<T, ReduceOp> WITH_ARGS(vector, std::move(reduceOp), std::move(rows), std::move(values));
        }

        /**
         * @brief Read vector to host using callback
         *
         * @details
         *  Uses user provided callback function to consume vector data.
         *  Callback must accept two vector of type Index and T respectively.
         *  Vectors can be consumed by copy, by const or non-const reference.
         *
         * @tparam T Type of values
         * @tparam Callback callback function to consume vector index and values data
         *
         * @param vector Vector to read
         * @param callback Callback function to consume data
         * @param descriptor Operation descriptor
         *
         * @return Expression node
         */
        template<typename T,
                 typename Callback>
        Node read(const Vector<T> &vector,
                  Callback callback,
                  const Descriptor &descriptor = Descriptor()) {
            RETURN_NEW_NODE ReadVector<T, Callback> WITH_ARGS(vector, std::move(callback));
        }

        /**
         * @brief Assign vector value using mask
         *
         * @tparam T Type of vector values
         * @tparam M Type of mask values (not used)
         * @tparam AccumOp Binary op of type t x t -> t
         *
         * @param w Vector to assign values
         * @param mask Optional mask structure used to select values
         * @param accumOp Binary op used to accum new and existing values
         * @param value Value to assign
         * @param descriptor Operation descriptor
         *
         * @return Expression node
         */
        template<typename T,
                 typename M,
                 typename AccumOp>
        Node assign(const Vector<T> &w,
                    const std::optional<Vector<M>> &mask,
                    AccumOp accumOp,
                    T value,
                    const Descriptor &descriptor = Descriptor()) {
            RETURN_NEW_NODE AssignVector<T, M, AccumOp> WITH_ARGS(w, mask, std::move(accumOp), std::move(value));
        }

        template<typename T,
                 typename M,
                 typename AccumOp,
                 typename MultiplyOp,
                 typename ReduceOp>
        Node vxm(const Vector<T> &w,
                 const std::optional<Vector<M>> &mask,
                 AccumOp accumOp,
                 MultiplyOp multiplyOp,
                 ReduceOp reduceOp,
                 const Vector<T> &a,
                 const Matrix<T> &m,
                 const Descriptor &descriptor = Descriptor()) {
            // ...
        }

        /**
         * @brief Submit expression for the execution.
         *
         * @note Expression asynchronously submitted for execution
         * @note Expression cannot be modified after submission
         *
         * @see ExpressionSubmission::wait()
         * @see ExpressionSubmission::state()
         */
        ExpressionSubmission submit() {
            ExpressionSubmission submission;

            auto submission_private = submission.m_private.get();
            auto &state = submission.m_private->m_state;
            auto &flow = submission.m_private->m_taskflow;
            auto &future = submission.m_private->m_future;

            auto &executor = library().executor();

            // Edge case: empty expression
            if (m_nodes.empty()) {
                state.store(ExpressionState::Evaluated);
                future = std::make_unique<tf::Future<void>>(std::move(executor.run(flow)));
                return submission;
            }

            std::vector<tf::Task> tasks_execute(m_nodes.size());
            std::vector<tf::Task> tasks_finalize(m_nodes.size());

            for (std::size_t i = 0; i < m_nodes.size(); i++) {
                auto node = m_nodes[i];
                tasks_execute[i] = flow.emplace([node, submission_private](tf::Subflow &subflow) {
                    if (submission_private->m_state.load() == ExpressionState::Aborted)
                        return;

                    detail::SubtaskBuilder subtaskBuilder(subflow, submission_private);

                    try {
                        // Prepare used to acquire resources
                        node->prepare();
                        // Execute (or create sub-tasks)
                        node->execute(subtaskBuilder);
                    } catch (const std::exception &exception) {
                        submission_private->m_state.store(ExpressionState::Aborted);
                        std::lock_guard<std::mutex> lockGuard(submission_private->m_mutex);
                        submission_private->m_error = exception.what();
                        SPLA_LOG_ERROR("exception inside node-" << node->type()
                                                                << " '" << node->name() << "'"
                                                                << " what: " << exception.what());
                    }
                });

                tasks_finalize[i] = flow.emplace([node, submission_private]() {
                    try {
                        // Finalize is always called to release aquired resources
                        node->finalize();
                    } catch (const std::exception &exception) {
                        submission_private->m_state.store(ExpressionState::Aborted);
                        std::lock_guard<std::mutex> lockGuard(submission_private->m_mutex);
                        submission_private->m_error = exception.what();
                        SPLA_LOG_ERROR("exception inside node-" << node->type()
                                                                << " '" << node->name() << "'"
                                                                << " what: " << exception.what());
                    }
                });

                tasks_execute[i].precede(tasks_finalize[i]);
            }

            tf::Task notification = flow.emplace([submission_private]() {
                if (submission_private->m_state.load() != ExpressionState::Aborted)
                    submission_private->m_state.store(ExpressionState::Evaluated);
            });

            for (std::size_t i = 0; i < m_nodes.size(); i++) {
                auto &node = m_nodes[i];
                auto &node_task = tasks_finalize[i];
                for (const auto &succ : node->successors()) {
                    node_task.precede(tasks_execute[succ->id()]);
                }
            }

            for (auto &task : tasks_finalize)
                task.precede(notification);

            future = std::make_unique<tf::Future<void>>(std::move(executor.run(flow)));

            return submission;
        }

    private:
        std::size_t next_id() {
            return m_nodes.size();
        }

        Node add_node(ExpressionNode *node_ptr) {
            Node node(node_ptr);
            m_nodes.push_back(node);
            return node;
        }

    private:
        std::vector<detail::Ref<ExpressionNode>> m_nodes;
    };

    /**
     * @}
     */

}// namespace spla

#undef RETURN_NEW_NODE
#undef WITH_ARGS

#endif//SPLA_EXPRESSION_HPP
