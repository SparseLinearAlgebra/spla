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

#ifndef SPLA_STORAGE_MAANGER_VECTOR_HPP
#define SPLA_STORAGE_MAANGER_VECTOR_HPP

#include <storage/storage_manager.hpp>

#include <cpu/cpu_format_coo_vec.hpp>
#include <cpu/cpu_format_dense_vec.hpp>
#include <cpu/cpu_format_dok_vec.hpp>
#include <cpu/cpu_formats.hpp>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_format_coo_vec.hpp>
    #include <opencl/cl_format_dense_vec.hpp>
    #include <opencl/cl_formats.hpp>
#endif

namespace spla {

    template<typename T>
    using StorageManagerVector = StorageManager<T, FormatVector, static_cast<int>(FormatVector::Count)>;

    template<typename T>
    void register_formats_vector(StorageManagerVector<T>& manager) {
        using Storage = typename StorageManagerVector<T>::Storage;

        manager.register_constructor(FormatVector::CpuDok, [](Storage& s) {
            s.get_ref(FormatVector::CpuDok) = make_ref<CpuDokVec<T>>();
        });
        manager.register_constructor(FormatVector::CpuCoo, [](Storage& s) {
            s.get_ref(FormatVector::CpuCoo) = make_ref<CpuCooVec<T>>();
        });
        manager.register_constructor(FormatVector::CpuDense, [](Storage& s) {
            s.get_ref(FormatVector::CpuDense) = make_ref<CpuDenseVec<T>>();
        });

        manager.register_validator(FormatVector::CpuDok, [](Storage& s) {
            cpu_dok_vec_clear(*s.template get<CpuDokVec<T>>());
        });
        manager.register_validator(FormatVector::CpuCoo, [](Storage& s) {
            cpu_coo_vec_clear(*s.template get<CpuCooVec<T>>());
        });
        manager.register_validator(FormatVector::CpuDense, [](Storage& s) {
            cpu_dense_vec_resize(s.get_n_rows(), *s.template get<CpuDenseVec<T>>());
            cpu_dense_vec_fill(T(), *s.template get<CpuDenseVec<T>>());
        });

        manager.register_converter(FormatVector::CpuDok, FormatVector::CpuCoo, [](Storage& s) {
            auto* dok = s.template get<CpuDokVec<T>>();
            auto* coo = s.template get<CpuCooVec<T>>();
            cpu_coo_vec_resize(dok->values, *coo);
            cpu_dok_vec_to_coo(*dok, *coo);
        });
        manager.register_converter(FormatVector::CpuDok, FormatVector::CpuDense, [](Storage& s) {
            auto* dok   = s.template get<CpuDokVec<T>>();
            auto* dense = s.template get<CpuDenseVec<T>>();
            cpu_dense_vec_resize(s.get_n_rows(), *dense);
            cpu_dense_vec_fill(s.get_fill_value(), *dense);
            cpu_dok_vec_to_dense(s.get_n_rows(), *dok, *dense);
        });
        manager.register_converter(FormatVector::CpuCoo, FormatVector::CpuDok, [](Storage& s) {
            auto* coo = s.template get<CpuCooVec<T>>();
            auto* dok = s.template get<CpuDokVec<T>>();
            cpu_dok_vec_clear(*dok);
            cpu_coo_vec_to_dok(*coo, *dok);
        });
        manager.register_converter(FormatVector::CpuDense, FormatVector::CpuDok, [](Storage& s) {
            auto* dense = s.template get<CpuDenseVec<T>>();
            auto* dok   = s.template get<CpuDokVec<T>>();
            cpu_dok_vec_clear(*dok);
            cpu_dense_vec_to_dok(s.get_n_rows(), s.get_fill_value(), *dense, *dok);
        });


#if defined(SPLA_BUILD_OPENCL)
        manager.register_constructor(FormatVector::AccCoo, [](Storage& s) {
            s.get_ref(FormatVector::AccCoo) = make_ref<CLCooVec<T>>();
        });
        manager.register_constructor(FormatVector::AccDense, [](Storage& s) {
            s.get_ref(FormatVector::AccDense) = make_ref<CLDenseVec<T>>();
        });

        manager.register_validator(FormatVector::AccCoo, [](Storage& s) {
            auto* cl_coo = s.template get<CLCooVec<T>>();
            cl_coo_vec_clear(*cl_coo);
        });
        manager.register_validator(FormatVector::AccDense, [](Storage& s) {
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            cl_dense_vec_resize(s.get_n_rows(), *cl_dense);
            cl_dense_vec_fill_value(s.get_n_rows(), s.get_fill_value(), *cl_dense);
        });

        manager.register_converter(FormatVector::CpuDense, FormatVector::AccDense, [](Storage& s) {
            auto* cpu_dense = s.template get<CpuDenseVec<T>>();
            auto* cl_dense  = s.template get<CLDenseVec<T>>();
            cl_dense_vec_init(s.get_n_rows(), cpu_dense->Ax.data(), *cl_dense);
        });
        manager.register_converter(FormatVector::AccDense, FormatVector::CpuDense, [](Storage& s) {
            auto* cl_acc    = get_acc_cl();
            auto* cl_dense  = s.template get<CLDenseVec<T>>();
            auto* cpu_dense = s.template get<CpuDenseVec<T>>();
            cpu_dense_vec_resize(s.get_n_rows(), *cpu_dense);
            cl_dense_vec_read(s.get_n_rows(), cpu_dense->Ax.data(), *cl_dense, cl_acc->get_queue_default());
        });
        manager.register_converter(FormatVector::CpuCoo, FormatVector::AccCoo, [](Storage& s) {
            auto* cpu_coo = s.template get<CpuCooVec<T>>();
            auto* cl_coo  = s.template get<CLCooVec<T>>();
            cl_coo_vec_init(cpu_coo->values, cpu_coo->Ai.data(), cpu_coo->Ax.data(), *cl_coo);
        });
        manager.register_converter(FormatVector::AccCoo, FormatVector::CpuCoo, [](Storage& s) {
            auto* cl_acc  = get_acc_cl();
            auto* cl_coo  = s.template get<CLCooVec<T>>();
            auto* cpu_coo = s.template get<CpuCooVec<T>>();
            cpu_coo_vec_resize(cl_coo->values, *cpu_coo);
            cl_coo_vec_read(cl_coo->values, cpu_coo->Ai.data(), cpu_coo->Ax.data(), *cl_coo, cl_acc->get_queue_default());
        });
        manager.register_converter(FormatVector::AccCoo, FormatVector::AccDense, [](Storage& s) {
            auto* cl_acc   = get_acc_cl();
            auto* cl_coo   = s.template get<CLCooVec<T>>();
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            cl_dense_vec_resize(s.get_n_rows(), *cl_dense);
            cl_coo_vec_to_dense(s.get_n_rows(), s.get_fill_value(), *cl_coo, *cl_dense, cl_acc->get_queue_default());
        });
        manager.register_converter(FormatVector::AccDense, FormatVector::AccCoo, [](Storage& s) {
            auto* cl_acc   = get_acc_cl();
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            auto* cl_coo   = s.template get<CLCooVec<T>>();
            cl_dense_vec_to_coo(s.get_n_rows(), s.get_fill_value(), *cl_dense, *cl_coo, cl_acc->get_queue_default());
        });
#endif
    }

}// namespace spla

#endif//SPLA_STORAGE_MAANGER_VECTOR_HPP
