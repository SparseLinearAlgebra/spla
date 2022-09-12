/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#ifndef SPLA_OP_HPP
#define SPLA_OP_HPP

#include "object.hpp"
#include "type.hpp"

#include <string>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Op
     * @brief An callable operation to parametrize execution of math computations
     */
    class Op : public Object {
    public:
        SPLA_API ~Op() override                 = default;
        SPLA_API virtual std::string get_name() = 0;
        SPLA_API virtual std::string get_key()  = 0;
    };

    /**
     * @class OpUnary
     * @brief Unary operation with 1-arity
     */
    class OpUnary : public Op {
    public:
        SPLA_API ~OpUnary() override                    = default;
        SPLA_API virtual ref_ptr<Type> get_type_arg_0() = 0;
        SPLA_API virtual ref_ptr<Type> get_type_res()   = 0;
    };

    /**
     * @class OpBinary
     * @brief Binary operation with 2-arity
     */
    class OpBinary : public Op {
    public:
        SPLA_API ~OpBinary() override                   = default;
        SPLA_API virtual ref_ptr<Type> get_type_arg_0() = 0;
        SPLA_API virtual ref_ptr<Type> get_type_arg_1() = 0;
        SPLA_API virtual ref_ptr<Type> get_type_res()   = 0;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_OP_HPP
