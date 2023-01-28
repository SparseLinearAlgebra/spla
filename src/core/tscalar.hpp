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

#ifndef SPLA_TSCALAR_HPP
#define SPLA_TSCALAR_HPP

#include <spla/scalar.hpp>

#include <core/ttype.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     *
     * @tparam T
     */
    template<typename T>
    class TScalar final : public Scalar {
    public:
        TScalar() = default;
        explicit TScalar(T value);
        ~TScalar() override = default;

        ref_ptr<Type> get_type() override;
        Status        set_int(std::int32_t value) override;
        Status        set_uint(std::uint32_t value) override;
        Status        set_float(float value) override;
        Status        get_int(std::int32_t& value) override;
        Status        get_uint(std::uint32_t& value) override;
        Status        get_float(float& value) override;

        void               set_label(std::string label) override;
        const std::string& get_label() const override;

        T& get_value();
        T  get_value() const;

    private:
        std::string m_label;
        T           m_value = T();
    };

    template<typename T>
    TScalar<T>::TScalar(T value) : m_value(value) {
    }

    template<typename T>
    ref_ptr<Type> TScalar<T>::get_type() {
        return get_ttype<T>().template as<Type>();
    }

    template<typename T>
    Status TScalar<T>::set_int(std::int32_t value) {
        m_value = static_cast<T>(value);
        return Status::Ok;
    }
    template<typename T>
    Status TScalar<T>::set_uint(std::uint32_t value) {
        m_value = static_cast<T>(value);
        return Status::Ok;
    }
    template<typename T>
    Status TScalar<T>::set_float(float value) {
        m_value = static_cast<T>(value);
        return Status::Ok;
    }

    template<typename T>
    Status TScalar<T>::get_int(std::int32_t& value) {
        value = static_cast<std::int32_t>(m_value);
        return Status::Ok;
    }
    template<typename T>
    Status TScalar<T>::get_uint(std::uint32_t& value) {
        value = static_cast<std::uint32_t>(m_value);
        return Status::Ok;
    }
    template<typename T>
    Status TScalar<T>::get_float(float& value) {
        value = static_cast<float>(m_value);
        return Status::Ok;
    }

    template<typename T>
    void TScalar<T>::set_label(std::string label) {
        m_label = std::move(label);
    }

    template<typename T>
    const std::string& TScalar<T>::get_label() const {
        return m_label;
    }

    template<typename T>
    T& TScalar<T>::get_value() {
        return m_value;
    }
    template<typename T>
    T TScalar<T>::get_value() const {
        return m_value;
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TSCALAR_HPP
