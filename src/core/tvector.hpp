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

#ifndef SPLA_TVECTOR_HPP
#define SPLA_TVECTOR_HPP

#include <spla/config.hpp>
#include <spla/vector.hpp>

#include <core/tdecoration.hpp>
#include <core/ttype.hpp>

#include <sequential/cpu_dense_vec.hpp>

#include <memory>
#include <vector>

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
        Status             set_byte(uint row_id, std::int8_t value) override;
        Status             set_int(uint row_id, std::int32_t value) override;
        Status             set_uint(uint row_id, std::uint32_t value) override;
        Status             set_float(uint row_id, float value) override;
        Status             get_byte(uint row_id, int8_t& value) override;
        Status             get_int(uint row_id, int32_t& value) override;
        Status             get_uint(uint row_id, uint32_t& value) override;
        Status             get_float(uint row_id, float& value) override;

        ref_ptr<TDecoration<T>>&              get_dec(int index) { return m_decorations[index]; }
        ref_ptr<TDecoration<T>>&              get_dec(Format format) { return get_dec(static_cast<int>(format)); }
        ref_ptr<TDecoration<T>>&              get_dec_or_create(Format format);
        std::vector<ref_ptr<TDecoration<T>>>& get_decs() { return m_decorations; }

        template<typename Decorator>
        Decorator* get_dec_p() { return (Decorator*) (get_dec(Decorator::FORMAT).get()); }
        template<typename Decorator>
        Decorator* get_dec_or_create_p() { return (Decorator*) (get_dec_or_create(Decorator::FORMAT).get()); }

        void update_version();
        void ensure_dense_format();

    private:
        uint      m_version    = 1;
        uint      m_n_rows     = 0;
        StateHint m_state_hint = StateHint::Default;

        std::vector<ref_ptr<TDecoration<T>>> m_decorations;
        std::string                          m_label;
    };

    template<typename T>
    TVector<T>::TVector(uint n_rows) {
        m_n_rows = n_rows;
        m_decorations.resize(static_cast<int>(Format::CountVector) + 1);
    }

    template<typename T>
    Status TVector<T>::hint_state(StateHint hint) {
        return Status::InvalidState;
    }

    template<typename T>
    uint TVector<T>::get_n_rows() {
        return m_n_rows;
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
    ref_ptr<TDecoration<T>>& TVector<T>::get_dec_or_create(Format format) {
        auto index = static_cast<int>(format);

        if (m_decorations[index]) {
            return m_decorations[index];
        }

        if (format == Format::CpuDenseVec) {
            return m_decorations[index] = make_ref<CpuDenseVec<T>>();
        }
        if (format == Format::CpuCooVec) {
            return m_decorations[index] = make_ref<CpuCooVec<T>>();
        }

        LOG_MSG(Status::NotImplemented, "unable to create decoration of specified format");
        return m_decorations.back();
    }

    template<typename T>
    Status TVector<T>::set_byte(uint row_id, std::int8_t value) {
        ensure_dense_format();
        cpu_dense_vec_add_element(row_id, static_cast<T>(value), *get_dec_p<CpuDenseVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_int(uint row_id, std::int32_t value) {
        ensure_dense_format();
        cpu_dense_vec_add_element(row_id, static_cast<T>(value), *get_dec_p<CpuDenseVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_uint(uint row_id, std::uint32_t value) {
        ensure_dense_format();
        cpu_dense_vec_add_element(row_id, static_cast<T>(value), *get_dec_p<CpuDenseVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::set_float(uint row_id, float value) {
        ensure_dense_format();
        cpu_dense_vec_add_element(row_id, static_cast<T>(value), *get_dec_p<CpuDenseVec<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_byte(uint row_id, int8_t& value) {
        ensure_dense_format();
        value = static_cast<int8_t>(get_dec_p<CpuDenseVec<T>>()->Ax[row_id]);
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_int(uint row_id, int32_t& value) {
        ensure_dense_format();
        value = static_cast<int32_t>(get_dec_p<CpuDenseVec<T>>()->Ax[row_id]);
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_uint(uint row_id, uint32_t& value) {
        ensure_dense_format();
        value = static_cast<uint32_t>(get_dec_p<CpuDenseVec<T>>()->Ax[row_id]);
        return Status::Ok;
    }

    template<typename T>
    Status TVector<T>::get_float(uint row_id, float& value) {
        ensure_dense_format();
        value = static_cast<float>(get_dec_p<CpuDenseVec<T>>()->Ax[row_id]);
        return Status::Ok;
    }

    template<typename T>
    void TVector<T>::update_version() {
        ++m_version;
    }

    template<typename T>
    void TVector<T>::ensure_dense_format() {
        auto p_vec = get_dec_or_create_p<CpuDenseVec<T>>();

        if (p_vec->get_version() < m_version) {
            LOG_MSG(Status::Error, "data invalidation, previous content lost");
            update_version();
            cpu_dense_vec_resize(m_n_rows, *p_vec);
            p_vec->update_version(m_version);
        }
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TVECTOR_HPP
