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

#ifndef SPLA_SPLAFUNCTIONS_HPP
#define SPLA_SPLAFUNCTIONS_HPP

#include <spla-cpp/SplaFunctionBinary.hpp>
#include <spla-cpp/SplaFunctionUnary.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class Functions
     * @brief Predefined functions for built-in types.
     * Allows to access standard plus, minus, min, max function for built-in types.
     */
    class SPLA_API Functions {
    public:
        /**
         * Function c = a + b for Float32 type.
         * @return Plus function.
         */
        static RefPtr<FunctionBinary> PlusFloat32(Library &library);

        /**
         * Function c = a + b for Float64 type.
         * @return Plus function.
         */
        static RefPtr<FunctionBinary> PlusFloat64(Library &library);

    private:
        // friend class Library;
        // todo: static void InitFunctions(Library &library);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONS_HPP
