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

#include <core/ttype.hpp>

#include <sequential/cpu_coo.hpp>
#include <sequential/cpu_csr.hpp>
#include <sequential/cpu_lil.hpp>

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
        SPLA_API TMatrix(uint n_rows, uint n_cols);
        SPLA_API ~TMatrix() override = default;
        SPLA_API Status hint_state(StateHint hint) override;
        SPLA_API Status hint_format(FormatHint hint) override;
        SPLA_API uint   get_n_rows() override;
        SPLA_API uint   get_n_cols() override;
        SPLA_API ref_ptr<Type> get_type() override;

        void _ensure_cpu_lil();
        void _ensure_cpu_coo();
        void _ensure_cpu_csr();
        void _ensure_acc();

    private:
        CpuLil<T> m_lil;

        uint m_n_rows = 0;
        uint m_n_cols = 0;

        StateHint  m_state_hint  = StateHint::Default;
        FormatHint m_format_hint = FormatHint::Default;

        uint m_version = 0;
    };

    template<typename T>
    TMatrix<T>::TMatrix(uint n_rows, uint n_cols) {
        m_n_rows = n_rows;
        m_n_cols = n_cols;
    }

    template<typename T>
    Status TMatrix<T>::hint_state(StateHint hint) {
        if (m_state_hint == hint) {
            return Status::Ok;
        }


        return Status::InvalidState;
    }

    template<typename T>
    Status TMatrix<T>::hint_format(FormatHint hint) {
        return Status::Ok;
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

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_TMATRIX_HPP
