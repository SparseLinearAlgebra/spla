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

#ifndef SPLA_TTYPE_HPP
#define SPLA_TTYPE_HPP

#include <spla/type.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TType
     * @brief Type interface implementation with actual type info bound
     *
     * @tparam T Actual type information
     */
    template<typename T>
    class TType final : public Type {
    public:
        SPLA_API ~TType() override = default;
        SPLA_API const std::string &get_name() override;
        SPLA_API const std::string &get_code() override;
        SPLA_API const std::string &get_description() override;
        SPLA_API int                get_size() override;
        SPLA_API int                get_id() override;

        static ref_ptr<Type> make_type(std::string name, std::string code, std::string desc, int id);

    private:
        std::string m_name;
        std::string m_code;
        std::string m_desc;
        int         m_size = -1;
        int         m_id   = -1;
    };

    template<typename T>
    const std::string &TType<T>::get_name() {
        return m_name;
    }

    template<typename T>
    const std::string &TType<T>::get_code() {
        return m_code;
    }

    template<typename T>
    const std::string &TType<T>::get_description() {
        return m_desc;
    }

    template<typename T>
    int TType<T>::get_size() {
        return m_size;
    }

    template<typename T>
    int TType<T>::get_id() {
        return m_id;
    }

    template<typename T>
    ref_ptr<Type> TType<T>::make_type(std::string name, std::string code, std::string desc, int id) {
        ref_ptr<TType<T>> t(new TType<T>());
        t->m_name = std::move(name);
        t->m_code = std::move(code);
        t->m_desc = std::move(desc);
        t->m_size = static_cast<int>(sizeof(T));
        t->m_id   = id;
        return t.template as<Type>();
    }

    template<typename T>
    static ref_ptr<TType<T>> get_ttype() {
        assert(false && "not supported type");
        return ref_ptr<TType<T>>();
    }

    template<>
    ref_ptr<TType<std::int8_t>> get_ttype() {
        return BYTE.cast<TType<std::int8_t>>();
    }

    template<>
    ref_ptr<TType<std::int32_t>> get_ttype() {
        return INT.cast<TType<std::int32_t>>();
    }

    template<>
    ref_ptr<TType<std::uint32_t>> get_ttype() {
        return UINT.cast<TType<std::uint32_t>>();
    }

    template<>
    ref_ptr<TType<float>> get_ttype() {
        return FLOAT.cast<TType<float>>();
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TTYPE_HPP
