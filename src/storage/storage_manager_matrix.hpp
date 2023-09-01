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

#ifndef SPLA_STORAGE_MANAGER_MATRIX_HPP
#define SPLA_STORAGE_MANAGER_MATRIX_HPP

#include <storage/storage_manager.hpp>

#include <cpu/cpu_format_coo.hpp>
#include <cpu/cpu_format_csr.hpp>
#include <cpu/cpu_format_dok.hpp>
#include <cpu/cpu_format_lil.hpp>
#include <cpu/cpu_formats.hpp>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_format_csr.hpp>
    #include <opencl/cl_formats.hpp>
#endif

namespace spla {

    template<typename T>
    using StorageManagerMatrix = StorageManager<T, FormatMatrix, static_cast<int>(FormatMatrix::Count)>;

    template<typename T>
    void register_formats_matrix(StorageManagerMatrix<T>& manager) {
        using Storage = typename StorageManagerMatrix<T>::Storage;

        manager.register_constructor(FormatMatrix::CpuLil, [](Storage& s) {
            s.get_ref(FormatMatrix::CpuLil) = make_ref<CpuLil<T>>();
        });
        manager.register_constructor(FormatMatrix::CpuDok, [](Storage& s) {
            s.get_ref(FormatMatrix::CpuDok) = make_ref<CpuDok<T>>();
        });
        manager.register_constructor(FormatMatrix::CpuCoo, [](Storage& s) {
            s.get_ref(FormatMatrix::CpuCoo) = make_ref<CpuCoo<T>>();
        });
        manager.register_constructor(FormatMatrix::CpuCsr, [](Storage& s) {
            s.get_ref(FormatMatrix::CpuCsr) = make_ref<CpuCsr<T>>();
        });

        manager.register_validator_discard(FormatMatrix::CpuLil, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            cpu_lil_resize(s.get_n_rows(), *lil);
            cpu_lil_clear(*lil);
        });
        manager.register_validator_discard(FormatMatrix::CpuDok, [](Storage& s) {
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
        });
        manager.register_validator_discard(FormatMatrix::CpuCoo, [](Storage& s) {
            auto* coo = s.template get<CpuCoo<T>>();
            cpu_coo_clear(*coo);
        });

        manager.register_converter(FormatMatrix::CpuLil, FormatMatrix::CpuDok, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
            cpu_lil_to_dok(s.get_n_rows(), *lil, *dok);
        });
        manager.register_converter(FormatMatrix::CpuLil, FormatMatrix::CpuCoo, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            auto* coo = s.template get<CpuCoo<T>>();
            cpu_coo_resize(lil->values, *coo);
            cpu_lil_to_coo(s.get_n_rows(), *lil, *coo);
        });
        manager.register_converter(FormatMatrix::CpuLil, FormatMatrix::CpuCsr, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            auto* csr = s.template get<CpuCsr<T>>();
            cpu_csr_resize(s.get_n_rows(), lil->values, *csr);
            cpu_lil_to_csr(s.get_n_rows(), *lil, *csr);
        });

        manager.register_converter(FormatMatrix::CpuCoo, FormatMatrix::CpuLil, [](Storage& s) {
            auto* coo = s.template get<CpuCoo<T>>();
            auto* lil = s.template get<CpuLil<T>>();
            cpu_lil_clear(*lil);
            cpu_lil_resize(s.get_n_rows(), *lil);
            cpu_coo_to_lil(s.get_n_rows(), *coo, *lil);
        });
        manager.register_converter(FormatMatrix::CpuCoo, FormatMatrix::CpuDok, [](Storage& s) {
            auto* coo = s.template get<CpuCoo<T>>();
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
            cpu_coo_to_dok(*coo, *dok);
        });
        manager.register_converter(FormatMatrix::CpuCoo, FormatMatrix::CpuCsr, [](Storage& s) {
            auto* coo = s.template get<CpuCoo<T>>();
            auto* csr = s.template get<CpuCsr<T>>();
            cpu_csr_resize(s.get_n_rows(), coo->values, *csr);
            cpu_coo_to_csr(s.get_n_rows(), *coo, *csr);
        });

        manager.register_converter(FormatMatrix::CpuCsr, FormatMatrix::CpuDok, [](Storage& s) {
            auto* csr = s.template get<CpuCsr<T>>();
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
            cpu_csr_to_dok(s.get_n_rows(), *csr, *dok);
        });
        manager.register_converter(FormatMatrix::CpuCsr, FormatMatrix::CpuCoo, [](Storage& s) {
            auto* csr = s.template get<CpuCsr<T>>();
            auto* coo = s.template get<CpuCoo<T>>();
            cpu_coo_resize(csr->values, *coo);
            cpu_csr_to_coo(s.get_n_rows(), *csr, *coo);
        });

#if defined(SPLA_BUILD_OPENCL)
        manager.register_constructor(FormatMatrix::AccCsr, [](Storage& s) {
            s.get_ref(FormatMatrix::AccCsr) = make_ref<CLCsr<T>>();
        });

        manager.register_converter(FormatMatrix::CpuCsr, FormatMatrix::AccCsr, [](Storage& s) {
            auto* cpu_csr = s.template get<CpuCsr<T>>();
            auto* cl_csr  = s.template get<CLCsr<T>>();
            cl_csr_init(s.get_n_rows(), cpu_csr->values, cpu_csr->Ap.data(), cpu_csr->Aj.data(), cpu_csr->Ax.data(), *cl_csr);
        });

        manager.register_converter(FormatMatrix::AccCsr, FormatMatrix::CpuCsr, [](Storage& s) {
            auto* cl_acc  = get_acc_cl();
            auto* cl_csr  = s.template get<CLCsr<T>>();
            auto* cpu_csr = s.template get<CpuCsr<T>>();
            cpu_csr_resize(s.get_n_rows(), cl_csr->values, *cpu_csr);
            cl_csr_read(s.get_n_rows(), cl_csr->values, cpu_csr->Ap.data(), cpu_csr->Aj.data(), cpu_csr->Ax.data(), *cl_csr, cl_acc->get_queue_default());
        });
#endif
    }

}// namespace spla

#endif//SPLA_STORAGE_MANAGER_MATRIX_HPP
