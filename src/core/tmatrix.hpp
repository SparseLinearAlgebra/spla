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

#include <core/tdecoration.hpp>
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
     * @class MatrixDec
     * @brief Possible indexed matrix decoration enumerations
     */
    enum class MatrixDec {
        Coo    = 0,
        Csr    = 1,
        Csc    = 2,
        AccCoo = 3,
        AccCsr = 4,
        AccCsc = 5,
        Max    = 10
    };

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
        Status             hint_format(FormatHint hint) override;
        uint               get_n_rows() override;
        uint               get_n_cols() override;
        ref_ptr<Type>      get_type() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;

        ref_ptr<TDecoration<T>>&              get_decoration(MatrixDec dec) { return m_decorations[static_cast<int>(dec)]; }
        ref_ptr<TDecoration<T>>               get_decoration(int index) { return m_decorations[index]; }
        std::vector<ref_ptr<TDecoration<T>>>& get_decorations() { return m_decorations; }

    private:
        CpuLil<T> m_lil;

        uint       m_version     = 0;
        uint       m_n_rows      = 0;
        uint       m_n_cols      = 0;
        StateHint  m_state_hint  = StateHint::Default;
        FormatHint m_format_hint = FormatHint::Default;

        std::vector<std::shared_ptr<TDecoration<T>>> m_decorations;
        std::string                                  m_label;
    };

    template<typename T>
    TMatrix<T>::TMatrix(uint n_rows, uint n_cols) {
        m_n_rows = n_rows;
        m_n_cols = n_cols;
        m_decorations.resize(static_cast<int>(MatrixDec::Max));
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

    template<typename T>
    void TMatrix<T>::set_label(std::string label) {
        m_label = std::move(label);
    }

    template<typename T>
    const std::string& TMatrix<T>::get_label() const {
        return m_label;
    }

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_TMATRIX_HPP
