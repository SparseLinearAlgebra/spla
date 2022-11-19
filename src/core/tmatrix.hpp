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

#ifndef SPLA_TMATRIX_HPP
#define SPLA_TMATRIX_HPP

#include <spla/config.hpp>
#include <spla/matrix.hpp>

#include <core/logger.hpp>
#include <core/tdecoration.hpp>
#include <core/top.hpp>
#include <core/ttype.hpp>

#include <storage/storage_manager.hpp>
#include <storage/storage_manager_matrix.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TMatrix
     * @brief Matrix interface implementation with type information bound
     *
     * @tparam T Type of stored elements
     */
    template<typename T>
    class TMatrix final : public Matrix {
    public:
        TMatrix(uint n_rows, uint n_cols);
        ~TMatrix() override = default;
        Status             hint_state(StateHint hint) override;
        uint               get_n_rows() override;
        uint               get_n_cols() override;
        ref_ptr<Type>      get_type() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        Status             set_reduce(ref_ptr<OpBinary> resolve_duplicates) override;
        Status             set_byte(uint row_id, uint col_id, std::int8_t value) override;
        Status             set_int(uint row_id, uint col_id, std::int32_t value) override;
        Status             set_uint(uint row_id, uint col_id, std::uint32_t value) override;
        Status             set_float(uint row_id, uint col_id, float value) override;
        Status             get_byte(uint row_id, uint col_id, int8_t& value) override;
        Status             get_int(uint row_id, uint col_id, int32_t& value) override;
        Status             get_uint(uint row_id, uint col_id, uint32_t& value) override;
        Status             get_float(uint row_id, uint col_id, float& value) override;
        Status             clear() override;

        template<typename Decorator>
        Decorator* get() { return m_storage.template get<Decorator>(); }

        void validate_rw(Format format);
        void validate_rwd(Format format);
        void validate_wd(Format format);
        void validate_ctor(Format format);

        static StorageManagerMatrix<T>* get_storage_manager();

    private:
        typename StorageManagerMatrix<T>::Storage m_storage;
        std::string                               m_label;
    };

    template<typename T>
    TMatrix<T>::TMatrix(uint n_rows, uint n_cols) {
        m_storage.set_dims(n_rows, n_cols);
    }

    template<typename T>
    Status TMatrix<T>::hint_state(StateHint hint) {
        return Status::InvalidState;
    }

    template<typename T>
    uint TMatrix<T>::get_n_rows() {
        return m_storage.get_n_rows();
    }

    template<typename T>
    uint TMatrix<T>::get_n_cols() {
        return m_storage.get_n_cols();
    }

    template<typename T>
    ref_ptr<Type> TMatrix<T>::get_type() {
        return get_ttype<T>().template as<Type>();
    }

    template<typename T>
    void TMatrix<T>::set_label(std::string label) {
        m_label = std::move(label);
    }

    template<typename T>
    const std::string& TMatrix<T>::get_label() const {
        return m_label;
    }

    template<typename T>
    Status TMatrix<T>::set_reduce(ref_ptr<OpBinary> resolve_duplicates) {
        auto reduce = resolve_duplicates.template cast<TOpBinary<T, T, T>>();

        if (reduce) {
            validate_ctor(Format::CpuLil);
            get<CpuLil<T>>()->reduce = reduce->function;
            validate_ctor(Format::CpuDok);
            get<CpuDok<T>>()->reduce = reduce->function;
        }

        return Status::InvalidArgument;
    }

    template<typename T>
    Status TMatrix<T>::set_byte(uint row_id, uint col_id, std::int8_t value) {
        validate_rw(Format::CpuLil);
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_int(uint row_id, uint col_id, std::int32_t value) {
        validate_rw(Format::CpuLil);
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_uint(uint row_id, uint col_id, std::uint32_t value) {
        validate_rw(Format::CpuLil);
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_float(uint row_id, uint col_id, float value) {
        validate_rw(Format::CpuLil);
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::get_byte(uint row_id, uint col_id, int8_t& value) {
        validate_rw(Format::CpuDok);

        auto& Ax    = get<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<int8_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_int(uint row_id, uint col_id, int32_t& value) {
        validate_rw(Format::CpuDok);

        auto& Ax    = get<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<int32_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_uint(uint row_id, uint col_id, uint32_t& value) {
        validate_rw(Format::CpuDok);

        auto& Ax    = get<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<uint32_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_float(uint row_id, uint col_id, float& value) {
        validate_rw(Format::CpuDok);

        auto& Ax    = get<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<float>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::clear() {
        m_storage.invalidate();
        return Status::Ok;
    }

    template<typename T>
    void TMatrix<T>::validate_rw(Format format) {
        StorageManagerMatrix<T>* manager = get_storage_manager();
        manager->validate_rw(format, m_storage);
    }

    template<typename T>
    void TMatrix<T>::validate_rwd(Format format) {
        StorageManagerMatrix<T>* manager = get_storage_manager();
        manager->validate_rwd(format, m_storage);
    }

    template<typename T>
    void TMatrix<T>::validate_wd(Format format) {
        StorageManagerMatrix<T>* manager = get_storage_manager();
        manager->validate_wd(format, m_storage);
    }

    template<typename T>
    void TMatrix<T>::validate_ctor(Format format) {
        StorageManagerMatrix<T>* manager = get_storage_manager();
        manager->validate_ctor(format, m_storage);
    }

    template<typename T>
    StorageManagerMatrix<T>* TMatrix<T>::get_storage_manager() {
        static std::unique_ptr<StorageManagerMatrix<T>> storage_manager;

        if (!storage_manager) {
            storage_manager = std::make_unique<StorageManagerMatrix<T>>();
            register_formats_matrix(*storage_manager);
        }

        return storage_manager.get();
    }

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_TMATRIX_HPP
