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

#include <sequential/cpu_coo.hpp>
#include <sequential/cpu_csr.hpp>
#include <sequential/cpu_lil.hpp>

#include <memory>
#include <vector>

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

        ref_ptr<TDecoration<T>>&              get_dec(int index) { return m_decorations[index]; }
        ref_ptr<TDecoration<T>>&              get_dec(Format format) { return get_dec(static_cast<int>(format)); }
        ref_ptr<TDecoration<T>>&              get_dec_or_create(Format format);
        std::vector<ref_ptr<TDecoration<T>>>& get_decs() { return m_decorations; }

        template<typename Decorator>
        Decorator* get_dec_p() { return (Decorator*) (get_dec(Decorator::FORMAT).get()); }
        template<typename Decorator>
        Decorator* get_dec_or_create_p() { return (Decorator*) (get_dec_or_create(Decorator::FORMAT).get()); }

        bool is_valid();
        void validate();
        void invalidate();
        void update_version();
        void ensure_lil_format();
        void ensure_dok_format();

    private:
        uint      m_version    = 1;
        uint      m_n_rows     = 0;
        uint      m_n_cols     = 0;
        bool      m_valid      = false;
        StateHint m_state_hint = StateHint::Default;

        std::vector<ref_ptr<TDecoration<T>>> m_decorations;
        std::string                          m_label;
        ref_ptr<TOpBinary<T, T, T>>          m_reduce;
    };

    template<typename T>
    TMatrix<T>::TMatrix(uint n_rows, uint n_cols) {
        m_n_rows = n_rows;
        m_n_cols = n_cols;
        m_decorations.resize(static_cast<int>(Format::CountMatrix) + 1);
    }

    template<typename T>
    Status TMatrix<T>::hint_state(StateHint hint) {
        return Status::InvalidState;
    }

    template<typename T>
    uint TMatrix<T>::get_n_rows() {
        return m_n_rows;
    }

    template<typename T>
    uint TMatrix<T>::get_n_cols() {
        return m_n_cols;
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
        m_reduce = resolve_duplicates.template cast<TOpBinary<T, T, T>>();

        if (m_reduce) {
            if (auto p_vec = get_dec_p<CpuLil<T>>()) {
                p_vec->reduce = m_reduce->function;
            }
            if (auto p_vec = get_dec_p<CpuDok<T>>()) {
                p_vec->reduce = m_reduce->function;
            }
            return Status::Ok;
        }

        return Status::InvalidArgument;
    }

    template<typename T>
    ref_ptr<TDecoration<T>>& TMatrix<T>::get_dec_or_create(Format format) {
        auto index = static_cast<int>(format);

        if (m_decorations[index]) {
            return m_decorations[index];
        }

        if (format == Format::CpuLil) {
            return m_decorations[index] = make_ref<CpuLil<T>>();
        }
        if (format == Format::CpuDok) {
            return m_decorations[index] = make_ref<CpuDok<T>>();
        }
        if (format == Format::CpuCoo) {
            return m_decorations[index] = make_ref<CpuCoo<T>>();
        }
        if (format == Format::CpuCsr) {
            return m_decorations[index] = make_ref<CpuCsr<T>>();
        }
        if (format == Format::CpuCsc) {
            return m_decorations[index] = make_ref<CpuCsr<T>>();
        }

        LOG_MSG(Status::NotImplemented, "unable to create decoration of specified format");
        return m_decorations.back();
    }

    template<typename T>
    Status TMatrix<T>::set_byte(uint row_id, uint col_id, std::int8_t value) {
        ensure_lil_format();
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get_dec_p<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_int(uint row_id, uint col_id, std::int32_t value) {
        ensure_lil_format();
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get_dec_p<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_uint(uint row_id, uint col_id, std::uint32_t value) {
        ensure_lil_format();
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get_dec_p<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::set_float(uint row_id, uint col_id, float value) {
        ensure_lil_format();
        cpu_lil_add_element(row_id, col_id, static_cast<T>(value), *get_dec_p<CpuLil<T>>());
        return Status::Ok;
    }

    template<typename T>
    Status TMatrix<T>::get_byte(uint row_id, uint col_id, int8_t& value) {
        ensure_dok_format();

        auto& Ax    = get_dec_p<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<int8_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_int(uint row_id, uint col_id, int32_t& value) {
        ensure_dok_format();

        auto& Ax    = get_dec_p<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<int32_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_uint(uint row_id, uint col_id, uint32_t& value) {
        ensure_dok_format();

        auto& Ax    = get_dec_p<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<uint32_t>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::get_float(uint row_id, uint col_id, float& value) {
        ensure_dok_format();

        auto& Ax    = get_dec_p<CpuDok<T>>()->Ax;
        auto  entry = Ax.find(typename CpuDok<T>::Key(row_id, col_id));

        if (entry != Ax.end()) {
            value = static_cast<float>(entry->second);
            return Status::Ok;
        }

        return Status::NoValue;
    }

    template<typename T>
    Status TMatrix<T>::clear() {
        invalidate();
        update_version();
        return Status::Ok;
    }

    template<typename T>
    bool TMatrix<T>::is_valid() {
        return m_valid;
    }

    template<typename T>
    void TMatrix<T>::validate() {
        m_valid = true;
    }

    template<typename T>
    void TMatrix<T>::invalidate() {
        m_valid = false;
    }

    template<typename T>
    void TMatrix<T>::update_version() {
        ++m_version;
    }

    template<typename T>
    void TMatrix<T>::ensure_lil_format() {
        auto p_lil = get_dec_or_create_p<CpuLil<T>>();

        if (p_lil->get_version() < m_version) {
            if (is_valid()) {
                // todo: sync data
                LOG_MSG(Status::Error, "data invalidation, previous content lost");
                update_version();// todo: keep
                cpu_lil_resize(m_n_rows, *p_lil);
            } else {
                validate();
                update_version();
                cpu_lil_resize(m_n_rows, *p_lil);
            }

            p_lil->update_version(m_version);
            if (m_reduce) p_lil->reduce = m_reduce->function;
        }
    }

    template<typename T>
    void TMatrix<T>::ensure_dok_format() {
        auto p_dok = get_dec_or_create_p<CpuDok<T>>();

        if (p_dok->get_version() < m_version) {

            if (is_valid()) {
                auto p_lil = get_dec_p<CpuLil<T>>();
                assert(!p_lil || p_lil->get_version() == m_version && "only lil read supported");

                if (p_lil && p_lil->get_version() == m_version) {
                    LOG_MSG(Status::Ok, "copy data from lil, preserve content");
                    cpu_lil_to_dok(m_n_rows, *p_lil, *p_dok);
                }
            }

            p_dok->update_version(m_version);
            if (m_reduce) p_dok->reduce = m_reduce->function;
        }
    }

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_TMATRIX_HPP
