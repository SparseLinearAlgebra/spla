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

#include <sequential/cpu_array.hpp>

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
    enum class VectorDec {
        Dense    = 0,
        Coo      = 1,
        AccDense = 2,
        AccCoo   = 3,
        Max      = 4
    };

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
        Status             hint_format(FormatHint hint) override;
        uint               get_n_rows() override;
        ref_ptr<Type>      get_type() override;
        void               set_label(std::string label) override;
        const std::string& get_label() const override;

        ref_ptr<TDecoration<T>>&              get_decoration(VectorDec dec) { return m_decorations[static_cast<int>(dec)]; }
        ref_ptr<TDecoration<T>>               get_decoration(int index) { return m_decorations[index]; }
        std::vector<ref_ptr<TDecoration<T>>>& get_decorations() { return m_decorations; }

    private:
        uint       m_version     = 0;
        uint       m_n_rows      = 0;
        StateHint  m_state_hint  = StateHint::Default;
        FormatHint m_format_hint = FormatHint::Default;

        std::vector<std::shared_ptr<TDecoration<T>>> m_decorations;
        std::string                                  m_label;
    };

    template<typename T>
    TVector<T>::TVector(uint n_rows) {
        m_n_rows = n_rows;
        m_decorations.resize(static_cast<int>(VectorDec::Max));
    }

    template<typename T>
    Status TVector<T>::hint_state(StateHint hint) {
        return Status::InvalidState;
    }

    template<typename T>
    Status TVector<T>::hint_format(FormatHint hint) {
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

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TVECTOR_HPP
