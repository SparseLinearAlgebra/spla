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

#ifndef SPLA_CL_INTERSECT_HPP
#define SPLA_CL_INTERSECT_HPP

#include <schedule/schedule_tasks.hpp>

#include <core/dispatcher.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>
#include <core/tscalar.hpp>
#include <core/ttype.hpp>
#include <core/tvector.hpp>

#include <opencl/cl_counter.hpp>
#include <opencl/cl_format_coo_vec.hpp>
#include <opencl/cl_formats.hpp>
#include <opencl/cl_merge.hpp>
#include <opencl/cl_program_builder.hpp>
#include <opencl/generated/auto_intersect.hpp>

#include <cmath>
#include <sstream>

namespace spla {

    template<typename T>
    class Algo_intersect_cl final : public RegistryAlgo {
    public:
        ~Algo_intersect_cl() override = default;

        std::string get_name() override {
            return "intersect";
        }

        std::string get_description() override {
            return "parallel set intersection of key-value arrays on opencl device";
        }

        Status execute(const DispatchContext& ctx) override {
            auto t = ctx.task.template cast_safe<ScheduleTask_intersect>();
            if (!t) {
                LOG_MSG(Status::InvalidArgument, "invalid task type for cl_intersect");
                return Status::InvalidArgument;
            }

            return execute_impl(ctx);
        }

    private:
        static bool use_naive_impl(uint a_size, uint b_size) {
            const uint   min_size   = std::min(a_size, b_size);
            const uint   max_size   = std::max(a_size, b_size);
            const double naive_cost = double(min_size) * std::log2(double(max_size));
            const double merge_cost = double(a_size) + double(b_size);
            return naive_cost < merge_cost;
        }

        Status execute_impl(const DispatchContext& ctx) {
            auto t = ctx.task.template cast_safe<ScheduleTask_intersect>();

            const uint a_size = t->a_keys ? t->a_keys->get_n_rows() : 0;
            const uint b_size = t->b_keys ? t->b_keys->get_n_rows() : 0;

            if (use_naive_impl(a_size, b_size)) {
                return execute_naive(ctx);
            }

            return execute_merge_path(ctx);
        }

        Status execute_merge_path(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/intersect_merge_path");

            auto t = ctx.task.template cast_safe<ScheduleTask_intersect>();

            if (!t->a_keys || !t->a_vals || !t->b_keys || !t->b_vals || !t->r_keys || !t->r_vals) {
                LOG_MSG(Status::InvalidArgument, "null vector argument for cl_intersect");
                return Status::InvalidArgument;
            }

            ref_ptr<TVector<uint32_t>> a_keys_vec = t->a_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        a_vals_vec = t->a_vals.template cast_safe<TVector<T>>();
            ref_ptr<TVector<uint32_t>> b_keys_vec = t->b_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        b_vals_vec = t->b_vals.template cast_safe<TVector<T>>();

            ref_ptr<TVector<uint32_t>> r_keys_vec = t->r_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        r_vals_vec = t->r_vals.template cast_safe<TVector<T>>();

            ref_ptr<TOpBinary<T, T, T>> op = t->op.template cast_safe<TOpBinary<T, T, T>>();
            if (!op) {
                LOG_MSG(Status::Error, "Failed to cast binary operation");
                return Status::Error;
            }

            r_keys_vec->validate_wd(FormatVector::AccCoo);
            r_vals_vec->validate_wd(FormatVector::AccCoo);
            a_keys_vec->validate_rw(FormatVector::AccDense);
            a_vals_vec->validate_rw(FormatVector::AccDense);
            b_keys_vec->validate_rw(FormatVector::AccDense);
            b_vals_vec->validate_rw(FormatVector::AccDense);

            auto*       p_cl_r_keys = r_keys_vec->template get<CLCooVec<uint32_t>>();
            auto*       p_cl_r_vals = r_vals_vec->template get<CLCooVec<T>>();
            const auto* p_cl_a_keys = a_keys_vec->template get<CLDenseVec<uint32_t>>();
            const auto* p_cl_a_vals = a_vals_vec->template get<CLDenseVec<T>>();
            const auto* p_cl_b_keys = b_keys_vec->template get<CLDenseVec<uint32_t>>();
            const auto* p_cl_b_vals = b_vals_vec->template get<CLDenseVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            const uint a_size = a_keys_vec->get_n_rows();
            const uint b_size = b_keys_vec->get_n_rows();

            if (a_size == 0 || b_size == 0) {
                r_keys_vec->clear();
                r_vals_vec->clear();
                return Status::Ok;
            }

            const uint max_result = std::min(a_size, b_size);
            cl_coo_vec_resize(max_result, *p_cl_r_keys);
            cl_coo_vec_resize(max_result, *p_cl_r_vals);

            auto* p_tmp_alloc = p_cl_acc->get_alloc_tmp();

            const uint result_count = cl_merge_intersect<T>(queue,
                                                            p_cl_a_keys->Ax, p_cl_a_vals->Ax, a_size,
                                                            p_cl_b_keys->Ax, p_cl_b_vals->Ax, b_size,
                                                            p_cl_r_keys->Ai, p_cl_r_keys->Ax,
                                                            p_cl_r_vals->Ai, p_cl_r_vals->Ax,
                                                            op, p_tmp_alloc);

            p_tmp_alloc->free_all();

            p_cl_r_keys->values = result_count;
            p_cl_r_vals->values = result_count;

            LOG_MSG(Status::Ok, "Found " << result_count << " matches");
            return Status::Ok;
        }

        Status execute_naive(const DispatchContext& ctx) {
            TIME_PROFILE_SCOPE("opencl/intersect_naive");

            auto t = ctx.task.template cast_safe<ScheduleTask_intersect>();

            if (!t->a_keys || !t->a_vals || !t->b_keys || !t->b_vals || !t->r_keys || !t->r_vals) {
                LOG_MSG(Status::InvalidArgument, "null vector argument for cl_intersect");
                return Status::InvalidArgument;
            }

            ref_ptr<TVector<uint32_t>> a_keys_vec = t->a_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        a_vals_vec = t->a_vals.template cast_safe<TVector<T>>();
            ref_ptr<TVector<uint32_t>> b_keys_vec = t->b_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        b_vals_vec = t->b_vals.template cast_safe<TVector<T>>();

            ref_ptr<TVector<uint32_t>> r_keys_vec = t->r_keys.template cast_safe<TVector<uint32_t>>();
            ref_ptr<TVector<T>>        r_vals_vec = t->r_vals.template cast_safe<TVector<T>>();

            ref_ptr<TOpBinary<T, T, T>> op = t->op.template cast_safe<TOpBinary<T, T, T>>();
            if (!op) {
                LOG_MSG(Status::Error, "Failed to cast binary operation");
                return Status::Error;
            }

            r_keys_vec->validate_wd(FormatVector::AccCoo);
            r_vals_vec->validate_wd(FormatVector::AccCoo);
            a_keys_vec->validate_rw(FormatVector::AccDense);
            a_vals_vec->validate_rw(FormatVector::AccDense);
            b_keys_vec->validate_rw(FormatVector::AccDense);
            b_vals_vec->validate_rw(FormatVector::AccDense);

            auto*       p_cl_r_keys = r_keys_vec->template get<CLCooVec<uint32_t>>();
            auto*       p_cl_r_vals = r_vals_vec->template get<CLCooVec<T>>();
            const auto* p_cl_a_keys = a_keys_vec->template get<CLDenseVec<uint32_t>>();
            const auto* p_cl_a_vals = a_vals_vec->template get<CLDenseVec<T>>();
            const auto* p_cl_b_keys = b_keys_vec->template get<CLDenseVec<uint32_t>>();
            const auto* p_cl_b_vals = b_vals_vec->template get<CLDenseVec<T>>();

            auto* p_cl_acc = get_acc_cl();
            auto& queue    = p_cl_acc->get_queue_default();

            const uint a_size = a_keys_vec->get_n_rows();
            const uint b_size = b_keys_vec->get_n_rows();

            if (a_size == 0 || b_size == 0) {
                r_keys_vec->clear();
                r_vals_vec->clear();
                return Status::Ok;
            }

            const uint max_result = std::min(a_size, b_size);

            cl_coo_vec_resize(max_result, *p_cl_r_keys);
            cl_coo_vec_resize(max_result, *p_cl_r_vals);

            CLCounterWrapper cl_result_count;
            cl_result_count.set(queue, 0);

            std::shared_ptr<CLProgram> program;
            if (!ensure_kernel(op, program)) {
                return Status::CompilationError;
            }

            const bool a_is_smaller = a_size <= b_size;

            const auto* p_cl_x_keys = a_is_smaller ? p_cl_a_keys : p_cl_b_keys;
            const auto* p_cl_x_vals = a_is_smaller ? p_cl_a_vals : p_cl_b_vals;
            const auto* p_cl_y_keys = a_is_smaller ? p_cl_b_keys : p_cl_a_keys;
            const auto* p_cl_y_vals = a_is_smaller ? p_cl_b_vals : p_cl_a_vals;
            const uint  x_size      = a_is_smaller ? a_size : b_size;
            const uint  y_size      = a_is_smaller ? b_size : a_size;

            auto kernel = program->make_kernel("intersect_naive");
            kernel.setArg(0, p_cl_x_keys->Ax);
            kernel.setArg(1, p_cl_x_vals->Ax);
            kernel.setArg(2, x_size);
            kernel.setArg(3, p_cl_y_keys->Ax);
            kernel.setArg(4, p_cl_y_vals->Ax);
            kernel.setArg(5, y_size);
            kernel.setArg(6, p_cl_r_keys->Ai);
            kernel.setArg(7, p_cl_r_keys->Ax);
            kernel.setArg(8, p_cl_r_vals->Ai);
            kernel.setArg(9, p_cl_r_vals->Ax);
            kernel.setArg(10, cl_result_count.buffer());
            kernel.setArg(11, a_is_smaller ? 0u : 1u);

            const uint wgs         = p_cl_acc->get_default_wgs();
            const uint n_groups    = div_up_clamp(x_size, wgs, 1, 1024);
            const uint global_size = n_groups * wgs;

            cl::NDRange global(global_size);
            cl::NDRange local(wgs);

            CL_DISPATCH_PROFILED("exec", queue, kernel, cl::NDRange(), global, local);

            const uint result_count = cl_result_count.get(queue);
            LOG_MSG(Status::Ok, "Found " << result_count << " matches");

            p_cl_r_keys->values = result_count;
            p_cl_r_vals->values = result_count;

            return Status::Ok;
        }

        bool ensure_kernel(const ref_ptr<TOpBinary<T, T, T>>& op, std::shared_ptr<CLProgram>& program) {
            CLProgramBuilder builder;
            builder.set_name("intersect")
                    .add_type("TYPE", get_ttype<T>().template as<Type>())
                    .add_define("WARP_SIZE", get_acc_cl()->get_wave_size())
                    .add_op("OP_BINARY", op.template as<OpBinary>())
                    .set_source(source_intersect)
                    .acquire();

            program = builder.get_program();
            return true;
        }
    };

}// namespace spla

#endif//SPLA_CL_INTERSECT_HPP