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
#include <core/tarray.hpp>
#include <core/tdecoration.hpp>
#include <core/top.hpp>
#include <core/ttype.hpp>

#include <storage/storage_manager.hpp>
#include <storage/storage_manager_vector.hpp>

#include <algorithm>
#include <random>

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

        uint               get_n_rows() override;
        ref_ptr<Type>      get_type() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        Status             set_format(FormatVector format) override;
        Status             set_fill_value(const ref_ptr<Scalar>& value) override;
        Status             set_reduce(ref_ptr<OpBinary> resolve_duplicates) override;
        Status             set_int(uint row_id, std::int32_t value) override;
        Status             set_uint(uint row_id, std::uint32_t value) override;
        Status             set_float(uint row_id, float value) override;
        Status             get_int(uint row_id, int32_t& value) override;
        Status             get_uint(uint row_id, uint32_t& value) override;
        Status             get_float(uint row_id, float& value) override;
        Status             fill_noize(uint seed) override;
        Status             fill_with(const ref_ptr<Scalar>& value) override;
        Status             build(const ref_ptr<MemView>& keys, const ref_ptr<MemView>& values) override;
        Status             read(ref_ptr<MemView>& keys, ref_ptr<MemView>& values) override;
        Status             clear() override;

        template<typename Decorator>
        Decorator* get() { return m_storage.template get<Decorator>(); }

        void validate_rw(FormatVector format);
        void validate_rwd(FormatVector format);
        void validate_wd(FormatVector format);
        void validate_ctor(FormatVector format);
        bool is_valid(FormatVector format) const;
        T    get_fill_value() const { return m_storage.get_fill_value(); }

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
        LOG_MSG(Status::Ok, "set label '" << m_label << "' to " << (void*) this);
    }
    template<typename T>
    const std::string& TVector<T>::get_label() const {
        return m_label;
    }

    template<typename T>
    Status TVector<T>::set_format(FormatVector format) {
        validate_rw(format);
        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::set_fill_value(const ref_ptr<Scalar>& value) {
        if (value) {
            m_storage.invalidate();

            if constexpr (std::is_same<T, T_INT>::value) m_storage.set_fill_value(value->as_int());
            if constexpr (std::is_same<T, T_UINT>::value) m_storage.set_fill_value(value->as_uint());
            if constexpr (std::is_same<T, T_FLOAT>::value) m_storage.set_fill_value(value->as_float());

            return Status::Ok;
        }

        return Status::InvalidArgument;
    }
    template<typename T>
    Status TVector<T>::set_reduce(ref_ptr<OpBinary> resolve_duplicates) {
        auto reduce = resolve_duplicates.template cast_safe<TOpBinary<T, T, T>>();

        if (reduce) {
            validate_ctor(FormatVector::CpuDok);
            auto* vec   = get<CpuDokVec<T>>();
            vec->reduce = reduce->function;
            return Status::Ok;
        }

        return Status::InvalidArgument;
    }

    template<typename T>
    Status TVector<T>::set_int(uint row_id, std::int32_t value) {
        if (is_valid(FormatVector::CpuDense)) {
            validate_rwd(FormatVector::CpuDense);
            get<CpuDenseVec<T>>()->Ax[row_id] = static_cast<T>(value);
            return Status::Ok;
        }

        validate_rwd(FormatVector::CpuDok);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::set_uint(uint row_id, std::uint32_t value) {
        if (is_valid(FormatVector::CpuDense)) {
            validate_rwd(FormatVector::CpuDense);
            get<CpuDenseVec<T>>()->Ax[row_id] = static_cast<T>(value);
            return Status::Ok;
        }

        validate_rwd(FormatVector::CpuDok);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::set_float(uint row_id, float value) {
        if (is_valid(FormatVector::CpuDense)) {
            validate_rwd(FormatVector::CpuDense);
            get<CpuDenseVec<T>>()->Ax[row_id] = static_cast<T>(value);
            return Status::Ok;
        }

        validate_rwd(FormatVector::CpuDok);
        cpu_dok_vec_add_element(row_id, static_cast<T>(value), *get<CpuDokVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_int(uint row_id, int32_t& value) {
        validate_rw(FormatVector::CpuDok);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = m_storage.get_fill_value();

        if (entry != Ax.end()) {
            value = static_cast<T_INT>(entry->second);
        }

        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::get_uint(uint row_id, uint32_t& value) {
        validate_rw(FormatVector::CpuDok);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = m_storage.get_fill_value();

        if (entry != Ax.end()) {
            value = static_cast<T_UINT>(entry->second);
        }

        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::get_float(uint row_id, float& value) {
        validate_rw(FormatVector::CpuDok);

        const auto& Ax    = get<CpuDokVec<T>>()->Ax;
        const auto  entry = Ax.find(row_id);
        value             = m_storage.get_fill_value();

        if (entry != Ax.end()) {
            value = static_cast<T_FLOAT>(entry->second);
        }

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::fill_noize(uint seed) {
        validate_wd(FormatVector::CpuDense);
        auto& Ax     = get<CpuDenseVec<T>>()->Ax;
        auto  engine = std::default_random_engine(seed);

        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist;
            for (auto& x : Ax) x = dist(engine);
        }
        if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist;
            for (auto& x : Ax) x = dist(engine);
        }

        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::fill_with(const ref_ptr<Scalar>& value) {
        assert(value);

        T t = T();

        if constexpr (std::is_same<T, T_INT>::value) t = value->as_int();
        if constexpr (std::is_same<T, T_UINT>::value) t = value->as_uint();
        if constexpr (std::is_same<T, T_FLOAT>::value) t = value->as_float();

        validate_wd(FormatVector::CpuDense);
        auto& Ax = get<CpuDenseVec<T>>()->Ax;
        std::fill(Ax.begin(), Ax.end(), t);

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::build(const ref_ptr<MemView>& keys, const ref_ptr<MemView>& values) {
        assert(keys);
        assert(values);

        const auto key_size       = sizeof(uint);
        const auto value_size     = sizeof(T);
        const auto elements_count = keys->get_size() / key_size;

        if (elements_count != values->get_size() / value_size) {
            return Status::InvalidArgument;
        }
        if (elements_count * key_size != keys->get_size()) {
            return Status::InvalidArgument;
        }

        validate_rwd(FormatVector::CpuCoo);
        CpuCooVec<T>& coo = *get<CpuCooVec<T>>();

        coo.Ai.resize(elements_count);
        coo.Ax.resize(elements_count);
        coo.values = uint(elements_count);

        keys->read(0, key_size * elements_count, coo.Ai.data());
        values->read(0, value_size * elements_count, coo.Ax.data());

        return Status::Ok;
    }
    template<typename T>
    Status TVector<T>::read(ref_ptr<MemView>& keys, ref_ptr<MemView>& values) {
        const auto key_size   = sizeof(uint);
        const auto value_size = sizeof(T);

        validate_rw(FormatVector::CpuCoo);
        CpuCooVec<T>& coo = *get<CpuCooVec<T>>();

        const auto elements_count = coo.Ai.size();

        keys   = MemView::make(coo.Ai.data(), key_size * elements_count, false);
        values = MemView::make(coo.Ax.data(), value_size * elements_count, false);

        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::clear() {
        m_storage.invalidate();
        return Status::Ok;
    }

    template<typename T>
    void TVector<T>::validate_rw(FormatVector format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_rw(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_rwd(FormatVector format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_rwd(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_wd(FormatVector format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_wd(format, m_storage);
    }

    template<typename T>
    void TVector<T>::validate_ctor(FormatVector format) {
        StorageManagerVector<T>* manager = get_storage_manager();
        manager->validate_ctor(format, m_storage);
    }

    template<typename T>
    bool TVector<T>::is_valid(FormatVector format) const {
        return m_storage.is_valid(format);
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
