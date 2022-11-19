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

#ifndef SPLA_STORAGE_MANAGER_MATRIX_HPP
#define SPLA_STORAGE_MANAGER_MATRIX_HPP

#include <storage/storage_manager.hpp>

#include <sequential/cpu_csr.hpp>
#include <sequential/cpu_dok.hpp>
#include <sequential/cpu_formats.hpp>
#include <sequential/cpu_lil.hpp>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_csr.hpp>
    #include <opencl/cl_formats.hpp>
#endif

namespace spla {

    template<typename T>
    using StorageManagerMatrix = StorageManager<T, static_cast<int>(Format::CountMatrix)>;

    template<typename T>
    void register_formats_matrix(StorageManagerMatrix<T>& manager) {
        using Storage = typename StorageManagerMatrix<T>::Storage;

        manager.register_constructor(Format::CpuLil, [](Storage& s) {
            s.get_ref(Format::CpuLil) = make_ref<CpuLil<T>>();
        });
        manager.register_constructor(Format::CpuDok, [](Storage& s) {
            s.get_ref(Format::CpuDok) = make_ref<CpuDok<T>>();
        });
        manager.register_constructor(Format::CpuCsr, [](Storage& s) {
            s.get_ref(Format::CpuCsr) = make_ref<CpuCsr<T>>();
        });

        manager.register_validator(Format::CpuLil, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            cpu_lil_resize(s.get_n_rows(), *lil);
            cpu_lil_clear(*lil);
        });
        manager.register_validator(Format::CpuDok, [](Storage& s) {
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
        });

        manager.register_converter(Format::CpuLil, Format::CpuDok, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            auto* dok = s.template get<CpuDok<T>>();
            cpu_dok_clear(*dok);
            cpu_lil_to_dok(s.get_n_rows(), *lil, *dok);
        });
        manager.register_converter(Format::CpuLil, Format::CpuCsr, [](Storage& s) {
            auto* lil = s.template get<CpuLil<T>>();
            auto* csr = s.template get<CpuCsr<T>>();
            cpu_csr_resize(s.get_n_rows(), lil->values, *csr);
            cpu_lil_to_csr(s.get_n_rows(), *lil, *csr);
        });

#if defined(SPLA_BUILD_OPENCL)
        manager.register_constructor(Format::CLCsr, [](Storage& s) {
            s.get_ref(Format::CLCsr) = make_ref<CLCsr<T>>();
        });

        manager.register_converter(Format::CpuCsr, Format::CLCsr, [](Storage& s) {
            auto* cpu_csr = s.template get<CpuCsr<T>>();
            auto* cl_csr  = s.template get<CLCsr<T>>();
            cl_csr_init(s.get_n_rows(), cpu_csr->values, cpu_csr->Ap.data(), cpu_csr->Aj.data(), cpu_csr->Ax.data(), *cl_csr);
        });
#endif
    }

}// namespace spla

#endif//SPLA_STORAGE_MANAGER_MATRIX_HPP
