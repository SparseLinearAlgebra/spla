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

#ifndef SPLA_EXPRESSION_SUBMISSION_HPP
#define SPLA_EXPRESSION_SUBMISSION_HPP

#include <atomic>
#include <mutex>
#include <stdexcept>

#include <taskflow/taskflow.hpp>

#include <spla/detail/ref.hpp>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class ExpressionState
     * @brief State of the expression
     */
    enum class ExpressionState {
        /** @brief Submitted for evaluation */
        Submitted,
        /** @brief Successfully evaluated */
        Evaluated,
        /** @brief Aborted (not evaluated) */
        Aborted
    };

    /**
     * @brief Expression state to string
     *
     * @param state State to convert to string
     * @return State string representation
     */
    inline std::string to_string(ExpressionState state) {
        switch (state) {
            case ExpressionState::Submitted:
                return "submitted";
            case ExpressionState::Evaluated:
                return "evaluated";
            case ExpressionState::Aborted:
                return "aborted";
            default:
                return "unknown";
        }
    }

    namespace expression {
        class SubtaskBuilder;
    }

    /**
     * @class ExpressionSubmission
     * @brief Submitted expression for evaluation
     */
    class ExpressionSubmission {
    public:
        ExpressionSubmission(const ExpressionSubmission &) = default;
        ExpressionSubmission(ExpressionSubmission &&) noexcept = default;

        ~ExpressionSubmission() = default;

        ExpressionSubmission &operator=(const ExpressionSubmission &) = default;
        ExpressionSubmission &operator=(ExpressionSubmission &&) noexcept = default;

        /**
         * @brief Wait until expression submission is evaluated or aborted
         * @note Must be called only for submitted expression
         */
        void wait() const {
            if (!m_private->m_future)
                throw std::runtime_error("cannot wait submission in pending state");

            m_private->m_future->wait();
        }

        /**
         * @brief Get state of expression submission
         * @return State
         */
        [[nodiscard]] ExpressionState state() const {
            return m_private->m_state.load();
        }

        /**
         * @brief Get error string for aborted expression submission
         * @return Error string with message
         */
        [[nodiscard]] std::string error() const {
            if (state() != ExpressionState::Aborted)
                throw std::runtime_error("error string can be accessed only for aborted expression");

            std::lock_guard<std::mutex> lockGuard(m_private->m_mutex);
            return m_private->m_error;
        }

    private:
        friend class Expression;
        friend class expression::SubtaskBuilder;

        ExpressionSubmission() : m_private(new Private) {}

    private:
        /**
         * @brief Information required for expression submission
         */
        class Private final : public detail::RefCnt {
        public:
            ~Private() override {
                // Note: we must wait for the execution of any pending work
                // in order to safely release taskflow graph with tasks
                if (m_future)
                    m_future->wait();
            }

        public:
            tf::Taskflow m_taskflow;
            std::unique_ptr<tf::Future<void>> m_future;
            std::atomic<ExpressionState> m_state{ExpressionState::Submitted};
            std::string m_error;

            mutable std::mutex m_mutex;
        };

        detail::Ref<Private> m_private;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_EXPRESSION_SUBMISSION_HPP
