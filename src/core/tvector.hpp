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

#ifndef SPLA_TVECTOR_HPP
#define SPLA_TVECTOR_HPP

#include <spla/config.hpp>
#include <spla/vector.hpp>

#include <core/logger.hpp>
#include <core/tdecoration.hpp>
#include <core/top.hpp>
#include <core/ttype.hpp>

#include <storage/storage_manager.hpp>
#include <storage/storage_manager_vector.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TVector
     * @brief Vector interface implementation with type information bound
     *
     * @tparam T Type of stored elements
     */
    template<typename T>
    class TVector final : public Vector {
    public:
        explicit TVector(uint n_rows);
        ~TVector() override = default;
        Status             hint_state(StateHint hint) override;
        uint               get_n_rows() override;
        ref_ptr<Type>      get_type() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        Status             set_reduce(ref_ptr<OpBinary> resolve_duplicates) override;
        Status             set_byte(uint row_id, std::int8_t value) override;
        Status             set_int(uint row_id, std::int32_t value) override;
        Status             set_uint(uint row_id, std::uint32_t value) override;
        Status             set_float(uint row_id, float value) override;
        Status             get_byte(uint row_id, int8_t& value) override;
        Status             get_int(uint row_id, int32_t& value) override;
        Status             get_uint(uint row_id, uint32_t& value) override;
        Status             get_float(uint row_id, float& value) override;
        Status             clear() override;

        template<typename Decorator>
        Decorator* get() { return m_storage.template get<Decorator>(); }

        void validate_rw(Format format);
        void validate_rwd(Format format);
        void validate_wd(Format format);
        void validate_ctor(Format format);

        static StorageManagerVector<T>* get_storage_manager();

    private:
        typename StorageManagerVector<T>::Storage m_storage;
        std::string                               m_label;
    };

    template<typename T>
    TVector<T>::TVector(uint n_rows) {
        m_storage.set_dims(n_rows, 1);
    }

    template<typename T>
    Status TVector<T>::hint_state(StateHint hint) {
        return Status::InvalidState;
    }

    template<typename T>
    uint TVector<T>::get_n_rows() {
        return m_storage.get_n_rows();
    }

    template<typename T>
    ref_ptr<Type> TVector<T>::get_type() {
        return get_ttype<T>().template as<Type>();
    }

    template<typename T>
    void TVector<T>::set_label(std::string label) {
        m_label = std::move(label);
    }

    template<typename T>
    const std::string& TVector<T>::get_label() const {
        return m_label;
    }

    template<typename T>
    Status TVector<T>::set_reduce(ref_ptr<OpBinary> resolve_duplicates) {
        auto reduce = resolve_duplicates.template cast<TOpBinary<T, T, T>>();

        if (reduce) {
            validate_ctor(Format::CpuDokVec);
            auto* vec   = get<CpuDokVec<T>>();
            vec->reduce = reduce->function;
            return Status::Ok;
        }

        return Status::InvalidArgument;
    }

    template<typename T>
    Status TVector<T>::set_byte(uint row_id, std::int8_t value) {
        validate_rw(Format::CpuDokVec);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_int(uint row_id, std::int32_t value) {
        validate_rw(Format::CpuDokVec);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_uint(uint row_id, std::uint32_t value) {
        validate_rw(Format::CpuDokVec);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_float(uint row_id, float value) {
        validate_rw(Format::CpuDokVec);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_byte(uint row_id, int8_t& value) {
        validate_rw(Format::CpuDokVec);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = int8_t();

        if (entry != Ax.end()) {
            value = static_cast<T_BYTE>(entry->second);
        }

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_int(uint row_id, int32_t& value) {
        validate_rw(Format::CpuDokVec);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = int32_t();

        if (entry != Ax.end()) {
            value = static_cast<T_INT>(entry->second);
        }

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_uint(uint row_id, uint32_t& value) {
        validate_rw(Format::CpuDokVec);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = uint32_t();

        if (entry != Ax.end()) {
            value = static_cast<T_UINT>(entry->second);
        }

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_float(uint row_id, float& value) {
        validate_rw(Format::CpuDokVec);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = float();

        if (entry != Ax.end()) {
            value = static_cast<T_FLOAT>(entry->second);
        }

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::clear() {
        m_storage.invalidate();
        return Status::Ok;
    }

    template<typename T>
    void TVector<T>::validate_rw(Format format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_rw(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_rwd(Format format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_rwd(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_wd(Format format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_wd(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_ctor(Format format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_ctor(format, m_storage);
    }

    template<typename T>
    StorageManagerVector<T>* TVector<T>::get_storage_manager() {
        static std::unique_ptr<StorageManagerVector<T>> storage_manager;

        if (!storage_manager) {
            storage_manager = std::make_unique<StorageManagerVector<T>>();
            register_formats_vector(*storage_manager);
        }

        return storage_manager.get();
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TVECTOR_HPP
