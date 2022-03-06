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

#ifndef SPLA_BINARY_OP_HPP
#define SPLA_BINARY_OP_HPP

#include <sstream>
#include <string>

#define SPLA_MAKE_TYPE_DECL(type, decl)                         \
    template<>                                                  \
    struct MakeTypeDecl<type> {                                 \
        inline std::string operator()() const { return #decl; } \
    };

#define SPLA_BINARY_OP(func_name, type, arguments, ...)                                    \
    class func_name {                                                                      \
    public:                                                                                \
        static type invoke_host arguments __VA_ARGS__;                                     \
        static const std::string &name() {                                                 \
            static const std::string s_name = #func_name;                                  \
            return s_name;                                                                 \
        }                                                                                  \
        static const std::string &body() {                                                 \
            static const std::string s_body = #__VA_ARGS__;                                \
            return s_body;                                                                 \
        }                                                                                  \
        static const std::string &declaration() {                                          \
            static const std::string s_declaration =                                       \
                    detail::MakeFunctionDeclaration<type arguments>()(name(), #arguments); \
            return s_declaration;                                                          \
        }                                                                                  \
        static const std::string &source() {                                               \
            static const std::string s_source = declaration() + body();                    \
            return s_source;                                                               \
        }                                                                                  \
    };

namespace spla::binary_op {
    namespace detail {
        template<typename T>
        struct MakeTypeDecl {};

        SPLA_MAKE_TYPE_DECL(char, char);
        SPLA_MAKE_TYPE_DECL(short, short);
        SPLA_MAKE_TYPE_DECL(int, int);

        SPLA_MAKE_TYPE_DECL(unsigned char, uchar);
        SPLA_MAKE_TYPE_DECL(unsigned short, ushort);
        SPLA_MAKE_TYPE_DECL(unsigned int, uint);

        SPLA_MAKE_TYPE_DECL(float, float);
        SPLA_MAKE_TYPE_DECL(double, double);

        template<typename Signature>
        struct MakeFunctionDeclaration {};

        template<typename R, typename A, typename B>
        struct MakeFunctionDeclaration<R(A, B)> {
            std::string operator()(const std::string &name, std::string signature) const {
                std::stringstream declaration;

                signature.replace(signature.find_first_of(','), 1, " ");
                signature.replace(signature.find_first_of('('), 1, " ");
                signature.replace(signature.find_first_of(')'), 1, " ");

                std::stringstream signature_ss(signature);
                std::string arg1;
                std::string arg2;

                signature_ss >> arg1 >> arg1;
                signature_ss >> arg2 >> arg2;

                declaration << MakeTypeDecl<R>()() << " "
                            << name
                            << "("
                            << MakeTypeDecl<A>()() << " " << arg1 << ", "
                            << MakeTypeDecl<B>()() << " " << arg2
                            << ")";

                return declaration.str();
            }
        };
    }// namespace detail

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @brief Plus binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(plus, T, (T a, T b), { return a + b; })

    /**
     * @brief Minus binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(minus, T, (T a, T b), { return a - b; })

    /**
     * @brief Times (product) binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(times, T, (T a, T b), { return a * b; })

    /**
     * @brief Division binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(div, T, (T a, T b), { return a / b; })

    /**
     * @brief First of two args binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(first, T, (T a, T b), { return a; })

    /**
     * @brief Second of two binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(second, T, (T a, T b), { return b; })

    /**
     * @brief Constant one binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(one, T, (T a, T b), { return 1; })

    /**
     * @brief Reversed minus binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(rminus, T, (T a, T b), { return b - a; })

    /**
     * @brief Reversed division binary op
     * @tparam T Type of op values
     */
    template<typename T>
    SPLA_BINARY_OP(rdiv, T, (T a, T b), { return b / a; })

    /**
     * @}
     */

}// namespace spla::binary_op

#endif//SPLA_BINARY_OP_HPP
