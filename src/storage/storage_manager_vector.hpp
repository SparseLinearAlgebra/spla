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

#include <sequential/cpu_coo_vec.hpp>
#include <sequential/cpu_dense_vec.hpp>
#include <sequential/cpu_dok_vec.hpp>
#include <sequential/cpu_formats.hpp>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_coo_vec.hpp>
    #include <opencl/cl_dense_vec.hpp>
    #include <opencl/cl_formats.hpp>
#endif

namespace spla {

    template<typename T>
    using StorageManagerVector = StorageManager<T, static_cast<int>(Format::CountVector)>;

    template<typename T>
    void register_formats_vector(StorageManagerVector<T>& manager) {
        using Storage = typename StorageManagerVector<T>::Storage;

        manager.register_constructor(Format::CpuDokVec, [](Storage& s) {
            s.get_ref(Format::CpuDokVec) = make_ref<CpuDokVec<T>>();
        });
        manager.register_constructor(Format::CpuCooVec, [](Storage& s) {
            s.get_ref(Format::CpuCooVec) = make_ref<CpuCooVec<T>>();
        });
        manager.register_constructor(Format::CpuDenseVec, [](Storage& s) {
            s.get_ref(Format::CpuDenseVec) = make_ref<CpuDenseVec<T>>();
        });

        manager.register_validator(Format::CpuDokVec, [](Storage& s) {
            cpu_dok_vec_clear(*s.template get<CpuDokVec<T>>());
        });
        manager.register_validator(Format::CpuCooVec, [](Storage& s) {
            cpu_coo_vec_clear(*s.template get<CpuCooVec<T>>());
        });
        manager.register_validator(Format::CpuDenseVec, [](Storage& s) {
            cpu_dense_vec_resize(s.get_n_rows(), *s.template get<CpuDenseVec<T>>());
            cpu_dense_vec_fill(T(), *s.template get<CpuDenseVec<T>>());
        });

        manager.register_converter(Format::CpuDokVec, Format::CpuCooVec, [](Storage& s) {
            auto* dok = s.template get<CpuDokVec<T>>();
            auto* coo = s.template get<CpuCooVec<T>>();
            cpu_coo_vec_resize(dok->values, *coo);
            cpu_dok_vec_to_coo(*dok, *coo);
        });
        manager.register_converter(Format::CpuDokVec, Format::CpuDenseVec, [](Storage& s) {
            auto* dok   = s.template get<CpuDokVec<T>>();
            auto* dense = s.template get<CpuDenseVec<T>>();
            cpu_dense_vec_resize(s.get_n_rows(), *dense);
            cpu_dok_vec_to_dense(s.get_n_rows(), *dok, *dense);
        });
        manager.register_converter(Format::CpuCooVec, Format::CpuDokVec, [](Storage& s) {
            auto* coo = s.template get<CpuCooVec<T>>();
            auto* dok = s.template get<CpuDokVec<T>>();
            cpu_dok_vec_clear(*dok);
            cpu_coo_vec_to_dok(*coo, *dok);
        });
        manager.register_converter(Format::CpuDenseVec, Format::CpuDokVec, [](Storage& s) {
            auto* dense = s.template get<CpuDenseVec<T>>();
            auto* dok   = s.template get<CpuDokVec<T>>();
            cpu_dok_vec_clear(*dok);
            cpu_dense_vec_to_dok(s.get_n_rows(), *dense, *dok);
        });


#if defined(SPLA_BUILD_OPENCL)
        manager.register_constructor(Format::CLCooVec, [](Storage& s) {
            s.get_ref(Format::CLCooVec) = make_ref<CLCooVec<T>>();
        });
        manager.register_constructor(Format::CLDenseVec, [](Storage& s) {
            s.get_ref(Format::CLDenseVec) = make_ref<CLDenseVec<T>>();
        });

        manager.register_validator(Format::CLCooVec, [](Storage& s) {
            auto* cl_coo = s.template get<CLCooVec<T>>();
            cl_coo_vec_clear(*cl_coo);
        });
        manager.register_validator(Format::CLDenseVec, [](Storage& s) {
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            cl_dense_vec_resize(s.get_n_rows(), *cl_dense);
        });

        manager.register_converter(Format::CpuDenseVec, Format::CLDenseVec, [](Storage& s) {
            auto* cpu_dense = s.template get<CpuDenseVec<T>>();
            auto* cl_dense  = s.template get<CLDenseVec<T>>();
            cl_dense_vec_init(s.get_n_rows(), cpu_dense->Ax.data(), *cl_dense);
        });
        manager.register_converter(Format::CLDenseVec, Format::CpuDenseVec, [](Storage& s) {
            auto* cl_acc    = get_acc_cl();
            auto* cl_dense  = s.template get<CLDenseVec<T>>();
            auto* cpu_dense = s.template get<CpuDenseVec<T>>();
            cpu_dense_vec_resize(s.get_n_rows(), *cpu_dense);
            cl_dense_vec_read(s.get_n_rows(), cpu_dense->Ax.data(), *cl_dense, cl_acc->get_queue_default());
        });
        manager.register_converter(Format::CpuCooVec, Format::CLCooVec, [](Storage& s) {
            auto* cpu_coo = s.template get<CpuCooVec<T>>();
            auto* cl_coo  = s.template get<CLCooVec<T>>();
            cl_coo_vec_init(cpu_coo->values, cpu_coo->Ai.data(), cpu_coo->Ax.data(), *cl_coo);
        });
        manager.register_converter(Format::CLCooVec, Format::CpuCooVec, [](Storage& s) {
            auto* cl_acc  = get_acc_cl();
            auto* cl_coo  = s.template get<CLCooVec<T>>();
            auto* cpu_coo = s.template get<CpuCooVec<T>>();
            cpu_coo_vec_resize(cl_coo->values, *cpu_coo);
            cl_coo_vec_read(cl_coo->values, cpu_coo->Ai.data(), cpu_coo->Ax.data(), *cl_coo, cl_acc->get_queue_default());
        });
        manager.register_converter(Format::CLCooVec, Format::CLDenseVec, [](Storage& s) {
            auto* cl_acc   = get_acc_cl();
            auto* cl_coo   = s.template get<CLCooVec<T>>();
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            cl_dense_vec_resize(s.get_n_rows(), *cl_dense);
            cl_coo_vec_to_dense(s.get_n_rows(), cl_coo->values, *cl_coo, *cl_dense, cl_acc->get_queue_default());
        });
        manager.register_converter(Format::CLDenseVec, Format::CLCooVec, [](Storage& s) {
            auto* cl_acc   = get_acc_cl();
            auto* cl_dense = s.template get<CLDenseVec<T>>();
            auto* cl_coo   = s.template get<CLCooVec<T>>();
            cl_dense_vec_to_coo(s.get_n_rows(), *cl_dense, *cl_coo, cl_acc->get_queue_default());
        });
#endif
    }

}// namespace spla

#endif//SPLA_STORAGE_MAANGER_VECTOR_HPP
