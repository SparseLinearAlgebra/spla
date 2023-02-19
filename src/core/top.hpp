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

#ifndef SPLA_TOP_HPP
#define SPLA_TOP_HPP

#include <spla/config.hpp>
#include <spla/op.hpp>

#include <core/ttype.hpp>

#include <functional>
#include <sstream>
#include <string>

namespace spla {

#define DECL_OP_BIN(fname, key_prefix, A0, A1, R, ...)      \
    {                                                       \
        auto func = make_ref<TOpBinary<A0, A1, R>>();       \
                                                            \
        func->function = [](A0 a, A1 b) -> R __VA_ARGS__;   \
        func->name     = #fname;                            \
                                                            \
        std::stringstream source_builder;                   \
        source_builder << "("                               \
                       << func->get_type_arg_0()->get_cpp() \
                       << " a, "                            \
                       << func->get_type_arg_1()->get_cpp() \
                       << " b)" #__VA_ARGS__;               \
                                                            \
        func->source = source_builder.str();                \
                                                            \
        std::stringstream key_builder;                      \
        key_builder << #key_prefix << "_"                   \
                    << func->get_type_arg_0()->get_code()   \
                    << func->get_type_arg_1()->get_code()   \
                    << func->get_type_res()->get_code();    \
        func->key = key_builder.str();                      \
                                                            \
        fname = func.as<OpBinary>();                        \
    }

#define DECL_OP_BIN_S(name, key_prefix, T, ...) \
    DECL_OP_BIN(name, key_prefix, T, T, T, __VA_ARGS__)

#define DECL_OP_SELECT(fname, key_prefix, A0, ...)          \
    {                                                       \
        auto func = make_ref<TOpSelect<A0>>();              \
                                                            \
        func->function = [](A0 a) -> bool __VA_ARGS__;      \
        func->name     = #fname;                            \
                                                            \
        std::stringstream source_builder;                   \
        source_builder << "("                               \
                       << func->get_type_arg_0()->get_cpp() \
                       << " a)" #__VA_ARGS__;               \
                                                            \
        func->source = source_builder.str();                \
                                                            \
        std::stringstream key_builder;                      \
        key_builder << #key_prefix << "_"                   \
                    << func->get_type_arg_0()->get_code();  \
        func->key = key_builder.str();                      \
                                                            \
        fname = func.as<OpSelect>();                        \
    }

    /**
     * @addtogroup internal
     * @{
     */

    template<typename A0, typename R>
    class TOpUnary : public OpUnary {
    public:
        ~TOpUnary() override = default;

        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        std::string        get_name() override;
        std::string        get_source() override;
        std::string        get_key() override;
        ref_ptr<Type>      get_type_arg_0() override;
        ref_ptr<Type>      get_type_res() override;

        std::function<R(A0)> function;
        std::string          name;
        std::string          source;
        std::string          key;
        std::string          label;
    };

    template<typename A0, typename R>
    void TOpUnary<A0, R>::set_label(std::string new_label) {
        label = std::move(new_label);
    }
    template<typename A0, typename R>
    const std::string& TOpUnary<A0, R>::get_label() const {
        return label;
    }
    template<typename A0, typename R>
    std::string TOpUnary<A0, R>::get_name() {
        return name;
    }
    template<typename A0, typename R>
    std::string TOpUnary<A0, R>::get_source() {
        return source;
    }
    template<typename A0, typename R>
    std::string TOpUnary<A0, R>::get_key() {
        return key;
    }
    template<typename A0, typename R>
    ref_ptr<Type> TOpUnary<A0, R>::get_type_arg_0() {
        return get_ttype<A0>().template as<Type>();
    }
    template<typename A0, typename R>
    ref_ptr<Type> TOpUnary<A0, R>::get_type_res() {
        return get_ttype<R>().template as<Type>();
    }

    template<typename A0, typename A1, typename R>
    class TOpBinary : public OpBinary {
    public:
        ~TOpBinary() override = default;

        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        std::string        get_name() override;
        std::string        get_source() override;
        std::string        get_key() override;
        ref_ptr<Type>      get_type_arg_0() override;
        ref_ptr<Type>      get_type_arg_1() override;
        ref_ptr<Type>      get_type_res() override;

        std::function<R(A0, A1)> function;
        std::string              name;
        std::string              source;
        std::string              key;
        std::string              label;
    };

    template<typename A0, typename A1, typename R>
    void TOpBinary<A0, A1, R>::set_label(std::string new_label) {
        label = std::move(new_label);
    }
    template<typename A0, typename A1, typename R>
    const std::string& TOpBinary<A0, A1, R>::get_label() const {
        return label;
    }
    template<typename A0, typename A1, typename R>
    std::string TOpBinary<A0, A1, R>::get_name() {
        return name;
    }
    template<typename A0, typename A1, typename R>
    std::string TOpBinary<A0, A1, R>::get_source() {
        return source;
    }
    template<typename A0, typename A1, typename R>
    std::string TOpBinary<A0, A1, R>::get_key() {
        return key;
    }
    template<typename A0, typename A1, typename R>
    ref_ptr<Type> TOpBinary<A0, A1, R>::get_type_arg_0() {
        return get_ttype<A0>().template as<Type>();
    }
    template<typename A0, typename A1, typename R>
    ref_ptr<Type> TOpBinary<A0, A1, R>::get_type_arg_1() {
        return get_ttype<A1>().template as<Type>();
    }
    template<typename A0, typename A1, typename R>
    ref_ptr<Type> TOpBinary<A0, A1, R>::get_type_res() {
        return get_ttype<R>().template as<Type>();
    }

    template<typename A0>
    class TOpSelect : public OpSelect {
    public:
        ~TOpSelect() override = default;

        void               set_label(std::string label) override;
        const std::string& get_label() const override;
        std::string        get_name() override;
        std::string        get_source() override;
        std::string        get_key() override;
        ref_ptr<Type>      get_type_arg_0() override;
        ref_ptr<Type>      get_type_res() override;

        std::function<bool(A0)> function;
        std::string             name;
        std::string             source;
        std::string             key;
        std::string             label;
    };

    template<typename A0>
    void TOpSelect<A0>::set_label(std::string new_label) {
        label = std::move(new_label);
    }
    template<typename A0>
    const std::string& TOpSelect<A0>::get_label() const {
        return label;
    }
    template<typename A0>
    std::string TOpSelect<A0>::get_name() {
        return name;
    }
    template<typename A0>
    std::string TOpSelect<A0>::get_source() {
        return source;
    }
    template<typename A0>
    std::string TOpSelect<A0>::get_key() {
        return key;
    }
    template<typename A0>
    ref_ptr<Type> TOpSelect<A0>::get_type_arg_0() {
        return get_ttype<A0>().template as<Type>();
    }
    template<typename A0>
    ref_ptr<Type> TOpSelect<A0>::get_type_res() {
        return BOOL.template as<Type>();
    }

    /**
     * @brief Register all ops on library initialization
     */
    void register_ops();

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TOP_HPP
