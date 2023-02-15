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

#ifndef SPLA_T_DECORATION_HPP
#define SPLA_T_DECORATION_HPP

#include <spla/ref.hpp>

#include <array>
#include <bitset>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TDecoration
     * @brief Base class for typed decoration for storage object
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class TDecoration : public RefCnt {
    public:
        ~TDecoration() override = default;

        /** @return Number of value in decoration */
        [[nodiscard]] virtual uint get_n_values() const { return values; }

    public:
        uint values = 0;
    };

    /**
     * @class TDecorationStorage
     * @brief Storage for decorators with data of a particular vector or matrix object
     *
     * @tparam T Type of elements stored in decorators
     * @tparam F Format of stored data
     * @tparam capacity Static storage capacity to allocate decorators
     */
    template<typename T, typename F, int capacity>
    class TDecorationStorage {
    public:
        ref_ptr<TDecoration<T>>& get_ref_i(int index) { return m_decorations[index]; }
        TDecoration<T>*          get_ptr_i(int index) { return m_decorations[index].get(); }
        ref_ptr<TDecoration<T>>& get_ref(F format) { return get_ref_i(static_cast<int>(format)); }
        TDecoration<T>*          get_ptr(F format) { return get_ptr_i(static_cast<int>(format)); }

        template<class DecorationClass>
        DecorationClass* get() { return dynamic_cast<DecorationClass*>(get_ptr(DecorationClass::FORMAT)); }

        [[nodiscard]] bool is_valid_any() const { return m_is_valid.any(); }
        [[nodiscard]] bool is_valid_i(int index) const { return m_is_valid.test(index); }
        [[nodiscard]] bool is_valid(F format) const { return is_valid_i(static_cast<int>(format)); }
        void               validate(F format) { m_is_valid.set(static_cast<int>(format), true); }
        void               invalidate() { m_is_valid.reset(); }

        void set_fill_value(T value) { m_fill_value = value; }
        T    get_fill_value() const { return m_fill_value; }

        [[nodiscard]] uint get_n_rows() const { return m_n_rows; }
        [[nodiscard]] uint get_n_cols() const { return m_n_cols; }

        void set_dims(uint n_rows, uint n_cols) {
            m_n_rows = n_rows;
            m_n_cols = n_cols;
        }

    private:
        std::array<ref_ptr<TDecoration<T>>, capacity> m_decorations;
        std::bitset<capacity>                         m_is_valid;
        uint                                          m_n_rows     = 0;
        uint                                          m_n_cols     = 0;
        T                                             m_fill_value = T();
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_T_DECORATION_HPP
