/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
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

#ifndef SPLA_TARRAY_HPP
#define SPLA_TARRAY_HPP

#include <spla/array.hpp>
#include <spla/config.hpp>

#include <core/logger.hpp>
#include <core/tdecoration.hpp>
#include <core/ttype.hpp>

#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TArray
     * @brief Array interface implementation with type information bound
     *
     * @tparam T Type of stored elements
     */
    template<typename T>
    class TArray final : public Array {
    public:
        explicit TArray(uint n_values);

        ~TArray() override = default;

        uint               get_n_values() override;
        ref_ptr<Type>      get_type() override;
        Status             set_int(uint i, T_INT value) override;
        Status             set_uint(uint i, T_UINT value) override;
        Status             set_float(uint i, T_FLOAT value) override;
        Status             get_int(uint i, T_INT& value) override;
        Status             get_uint(uint i, T_UINT& value) override;
        Status             get_float(uint i, T_FLOAT& value) override;
        Status             resize(uint n_values) override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;

        const std::vector<T>& data() const { return m_data; }
        std::vector<T>&       data() { return m_data; }

    private:
        std::vector<T> m_data;
        std::string    m_label;
    };

    template<typename T>
    TArray<T>::TArray(uint n_values) {
        m_data.resize(n_values);
    }

    template<typename T>
    uint TArray<T>::get_n_values() {
        return uint(m_data.size());
    }
    template<typename T>
    ref_ptr<Type> TArray<T>::get_type() {
        return get_ttype<T>().template cast<Type>();
    }

    template<typename T>
    Status TArray<T>::set_int(uint i, T_INT value) {
        assert(i < get_n_values());
        m_data[i] = T(value);
        return Status::Ok;
    }
    template<typename T>
    Status TArray<T>::set_uint(uint i, T_UINT value) {
        assert(i < get_n_values());
        m_data[i] = T(value);
        return Status::Ok;
    }
    template<typename T>
    Status TArray<T>::set_float(uint i, T_FLOAT value) {
        assert(i < get_n_values());
        m_data[i] = T(value);
        return Status::Ok;
    }

    template<typename T>
    Status TArray<T>::get_int(uint i, T_INT& value) {
        assert(i < get_n_values());
        value = T_INT(m_data[i]);
        return Status::Ok;
    }
    template<typename T>
    Status TArray<T>::get_uint(uint i, T_UINT& value) {
        assert(i < get_n_values());
        value = T_UINT(m_data[i]);
        return Status::Ok;
    }
    template<typename T>
    Status TArray<T>::get_float(uint i, T_FLOAT& value) {
        assert(i < get_n_values());
        value = T_FLOAT(m_data[i]);
        return Status::Ok;
    }

    template<typename T>
    Status TArray<T>::resize(uint n_values) {
        m_data.resize(n_values);
        return Status::Ok;
    }

    template<typename T>
    void TArray<T>::set_label(std::string label) {
        m_label = std::move(label);
        LOG_MSG(Status::Ok, "set label '" << m_label << "' to " << (void*) this);
    }
    template<typename T>
    const std::string& TArray<T>::get_label() const {
        return m_label;
    }


    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TARRAY_HPP
