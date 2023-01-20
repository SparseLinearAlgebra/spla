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

#ifndef SPLA_CL_UTILS_HPP
#define SPLA_CL_UTILS_HPP

#include <core/ttype.hpp>
#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program.hpp>

#include <string>
#include <unordered_map>

namespace spla {

    /**
     * @class CLUtils
     * @brief Utility-functions shared across opencl algorithms
     */
    class CLUtils {
    public:
        CLUtils();

        template<typename T>
        bool sort_by_key(const cl::Buffer& uint_keys, const cl::Buffer& t_values, uint size, cl::CommandQueue& queue) {
            auto& state = acquire_state<T>();
            auto& sort  = state.sort_bitonic;

            if (!sort.ensure(state.type)) return false;

            auto* acc = get_acc_cl();

            cl::NDRange global(acc->get_max_wgs());
            cl::NDRange local(acc->get_max_wgs());

            if (size <= sort.local_size) {
                sort.local.setArg(0, uint_keys);
                sort.local.setArg(1, t_values);
                sort.local.setArg(2, size);
                queue.enqueueNDRangeKernel(sort.local, cl::NDRange(), global, local);
            } else {
                sort.global.setArg(0, uint_keys);
                sort.global.setArg(1, t_values);
                sort.global.setArg(2, size);
                queue.enqueueNDRangeKernel(sort.global, cl::NDRange(), global, local);
            }

            return true;
        }

        template<typename T>
        bool vec_coo_to_dense(const cl::Buffer& Ai, const cl::Buffer& Ax, cl::Buffer& Rx, uint n, cl::CommandQueue& queue) {
            auto& state  = acquire_state<T>();
            auto& format = state.vector_formats;

            if (!format.ensure(state.type)) return false;

            auto* acc = get_acc_cl();

            uint block_size           = acc->get_wave_size();
            uint n_groups_to_dispatch = std::max(std::min(n / block_size, uint(512)), uint(1));

            cl::NDRange global(block_size * n_groups_to_dispatch);
            cl::NDRange local(block_size);

            format.sparse_to_dense.setArg(0, Ai);
            format.sparse_to_dense.setArg(1, Ax);
            format.sparse_to_dense.setArg(2, Rx);
            format.sparse_to_dense.setArg(3, n);
            queue.enqueueNDRangeKernel(format.sparse_to_dense, cl::NDRange(), global, local);

            return true;
        }

        template<typename T>
        bool vec_dense_to_coo(const cl::Buffer& Ax, cl::Buffer& Ri, cl::Buffer& Rx, uint n, uint& count, cl::CommandQueue& queue) {
            auto& state  = acquire_state<T>();
            auto& format = state.vector_formats;

            if (!format.ensure(state.type)) return false;

            auto* acc = get_acc_cl();

            count = 0;
            cl::Buffer cl_count(acc->get_context(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uint), &count);

            uint block_size           = acc->get_wave_size();
            uint n_groups_to_dispatch = std::max(std::min(n / block_size, uint(512)), uint(1));

            cl::NDRange global(block_size * n_groups_to_dispatch);
            cl::NDRange local(block_size);

            format.dense_to_sparse.setArg(0, Ax);
            format.dense_to_sparse.setArg(1, Ri);
            format.dense_to_sparse.setArg(2, Rx);
            format.dense_to_sparse.setArg(3, cl_count);
            format.dense_to_sparse.setArg(4, n);
            queue.enqueueNDRangeKernel(format.dense_to_sparse, cl::NDRange(), global, local);

            queue.enqueueReadBuffer(cl_count, true, 0, sizeof(count), &count);

            return true;
        }

        template<typename T>
        bool fill_zero(const cl::Buffer& values, uint n, cl::CommandQueue& queue) {
            auto& state = acquire_state<T>();
            auto& fill  = state.fill;

            if (!fill.ensure(state.type)) return true;

            auto* acc = get_acc_cl();

            uint block_size           = acc->get_wave_size();
            uint n_groups_to_dispatch = std::max(std::min(n / block_size, uint(512)), uint(1));

            cl::NDRange global(block_size * n_groups_to_dispatch);
            cl::NDRange local(block_size);

            fill.fill_zero.setArg(0, values);
            fill.fill_zero.setArg(1, n);
            queue.enqueueNDRangeKernel(fill.fill_zero, cl::NDRange(), global, local);

            return true;
        }

    private:
        struct CachedState {
            ref_ptr<Type> type;

            struct SortBitonic {
                std::shared_ptr<CLProgram> program;
                cl::Kernel                 local;
                cl::Kernel                 global;
                int                        local_size = 1024 * 8;
                bool                       ensure(const ref_ptr<Type>& type);
            } sort_bitonic;

            struct VectorFormats {
                std::shared_ptr<CLProgram> program;
                cl::Kernel                 sparse_to_dense;
                cl::Kernel                 dense_to_sparse;
                bool                       ensure(const ref_ptr<Type>& type);
            } vector_formats;

            struct Fill {
                std::shared_ptr<CLProgram> program;
                cl::Kernel                 fill_zero;
                cl::Kernel                 fill_value;
                bool                       ensure(const ref_ptr<Type>& type);
            } fill;
        };

        template<typename T>
        CachedState& acquire_state() { return m_typed_cache[get_ttype<T>()->get_name()]; }

    private:
        std::unordered_map<std::string, CachedState> m_typed_cache;
    };

}// namespace spla

#endif//SPLA_CL_UTILS_HPP
