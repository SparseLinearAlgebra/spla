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

#ifndef SPLA_CPU_INTERSECT_HPP
#define SPLA_CPU_INTERSECT_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/logger.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <cstdint>
#include <vector>

namespace spla {

    template<typename T>
    class Algo_intersect_cpu final : public RegistryAlgo {
    public:
        ~Algo_intersect_cpu() override = default;

        std::string get_name() override {
            return "intersect";
        }

        std::string get_description() override {
            return "sequential set intersection of two key-value arrays";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_intersect>();
            if (!t) {
                LOG_MSG(Status::InvalidArgument, "invalid task type for cpu_intersect");
                return Status::InvalidArgument;
            }

            if (!t->a_keys || !t->a_vals || !t->b_keys || !t->b_vals || !t->r_keys || !t->r_vals) {
                LOG_MSG(Status::InvalidArgument, "null vector argument for intersect");
                return Status::InvalidArgument;
            }

            const uint a_size = t->a_keys->get_n_rows();
            const uint b_size = t->b_keys->get_n_rows();

            if (a_size == 0 || b_size == 0) {
                if (t->r_keys) t->r_keys->clear();
                if (t->r_vals) t->r_vals->clear();
                return Status::Ok;
            }

            auto a_keys_vec = t->a_keys.template cast_safe<TVector<uint32_t>>();
            if (!a_keys_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast a_keys to TVector<uint32_t>");
                return Status::InvalidArgument;
            }

            auto a_vals_vec = t->a_vals.template cast_safe<TVector<T>>();
            if (!a_vals_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast a_vals to TVector<T>");
                return Status::InvalidArgument;
            }

            auto b_keys_vec = t->b_keys.template cast_safe<TVector<uint32_t>>();
            if (!b_keys_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast b_keys to TVector<uint32_t>");
                return Status::InvalidArgument;
            }

            auto b_vals_vec = t->b_vals.template cast_safe<TVector<T>>();
            if (!b_vals_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast b_vals to TVector<T>");
                return Status::InvalidArgument;
            }

            auto r_keys_vec = t->r_keys.template cast_safe<TVector<uint32_t>>();
            if (!r_keys_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast r_keys to TVector<uint32_t>");
                return Status::InvalidArgument;
            }

            auto r_vals_vec = t->r_vals.template cast_safe<TVector<T>>();
            if (!r_vals_vec) {
                LOG_MSG(Status::InvalidArgument, "Failed to cast r_vals to TVector<T>");
                return Status::InvalidArgument;
            }

            auto op = t->op.template cast_safe<TOpBinary<T, T, T>>();
            if (!op) {
                LOG_MSG(Status::Error, "Failed to cast binary operation");
                return Status::Error;
            }

            r_keys_vec->validate_wd(FormatVector::CpuCoo);
            r_vals_vec->validate_wd(FormatVector::CpuCoo);
            a_keys_vec->validate_rw(FormatVector::CpuDense);
            a_vals_vec->validate_rw(FormatVector::CpuDense);
            b_keys_vec->validate_rw(FormatVector::CpuDense);
            b_vals_vec->validate_rw(FormatVector::CpuDense);

            auto*       p_r_keys = r_keys_vec->template get<CpuCooVec<uint32_t>>();
            auto*       p_r_vals = r_vals_vec->template get<CpuCooVec<T>>();
            const auto* p_a_keys = a_keys_vec->template get<CpuDenseVec<uint32_t>>();
            const auto* p_a_vals = a_vals_vec->template get<CpuDenseVec<T>>();
            const auto* p_b_keys = b_keys_vec->template get<CpuDenseVec<uint32_t>>();
            const auto* p_b_vals = b_vals_vec->template get<CpuDenseVec<T>>();

            p_r_keys->Ai.clear();
            p_r_keys->Ax.clear();
            p_r_vals->Ai.clear();
            p_r_vals->Ax.clear();
            p_r_keys->values = 0;
            p_r_vals->values = 0;

            const auto& function = op->function;

            // Two-pointer scan over sorted arrays
            uint i = 0, j = 0;
            while (i < a_size && j < b_size) {
                const uint32_t key_a = p_a_keys->Ax[i];
                const uint32_t key_b = p_b_keys->Ax[j];

                if (key_a < key_b) {
                    ++i;
                } else if (key_b < key_a) {
                    ++j;
                } else {
                    // Match found
                    p_r_keys->Ai.push_back(p_r_keys->values);
                    p_r_keys->Ax.push_back(key_a);
                    p_r_vals->Ai.push_back(p_r_vals->values);
                    p_r_vals->Ax.push_back(function(p_a_vals->Ax[i], p_b_vals->Ax[j]));
                    p_r_keys->values++;
                    p_r_vals->values++;
                    ++i;
                    ++j;
                }
            }

            LOG_MSG(Status::Ok, "Found " << p_r_keys->values << " matches");
            return Status::Ok;
        }
    };

}// namespace spla

#endif//SPLA_CPU_INTERSECT_HPP