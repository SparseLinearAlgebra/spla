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
#include <spla-cpp/SplaTypes.hpp>


namespace spla {

    /**
     * @class Functions
     * @brief Predefined functions for built-in types.
     * Allows to access standard plus, minus, min, max function for built-in types.
     */
    class SPLA_API Functions {
    public:
        static RefPtr<FunctionBinary> PlusInt8(Library &library);
        static RefPtr<FunctionBinary> MinusInt8(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusInt8(Library &library);
        static RefPtr<FunctionBinary> MultInt8(Library &library);
        static RefPtr<FunctionBinary> DivInt8(Library &library);
        static RefPtr<FunctionBinary> ReverseDivInt8(Library &library);
        static RefPtr<FunctionBinary> TakeFirstInt8(Library &library);
        static RefPtr<FunctionBinary> TakeSecondInt8(Library &library);
        static RefPtr<FunctionBinary> PlusInt16(Library &library);
        static RefPtr<FunctionBinary> MinusInt16(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusInt16(Library &library);
        static RefPtr<FunctionBinary> MultInt16(Library &library);
        static RefPtr<FunctionBinary> DivInt16(Library &library);
        static RefPtr<FunctionBinary> ReverseDivInt16(Library &library);
        static RefPtr<FunctionBinary> TakeFirstInt16(Library &library);
        static RefPtr<FunctionBinary> TakeSecondInt16(Library &library);
        static RefPtr<FunctionBinary> PlusInt32(Library &library);
        static RefPtr<FunctionBinary> MinusInt32(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusInt32(Library &library);
        static RefPtr<FunctionBinary> MultInt32(Library &library);
        static RefPtr<FunctionBinary> DivInt32(Library &library);
        static RefPtr<FunctionBinary> ReverseDivInt32(Library &library);
        static RefPtr<FunctionBinary> TakeFirstInt32(Library &library);
        static RefPtr<FunctionBinary> TakeSecondInt32(Library &library);
        static RefPtr<FunctionBinary> PlusInt64(Library &library);
        static RefPtr<FunctionBinary> MinusInt64(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusInt64(Library &library);
        static RefPtr<FunctionBinary> MultInt64(Library &library);
        static RefPtr<FunctionBinary> DivInt64(Library &library);
        static RefPtr<FunctionBinary> ReverseDivInt64(Library &library);
        static RefPtr<FunctionBinary> TakeFirstInt64(Library &library);
        static RefPtr<FunctionBinary> TakeSecondInt64(Library &library);

        static RefPtr<FunctionBinary> OrInt8(Library &library);
        static RefPtr<FunctionBinary> AndInt8(Library &library);
        static RefPtr<FunctionBinary> XorInt8(Library &library);
        static RefPtr<FunctionBinary> OrInt16(Library &library);
        static RefPtr<FunctionBinary> AndInt16(Library &library);
        static RefPtr<FunctionBinary> XorInt16(Library &library);
        static RefPtr<FunctionBinary> OrInt32(Library &library);
        static RefPtr<FunctionBinary> AndInt32(Library &library);
        static RefPtr<FunctionBinary> XorInt32(Library &library);
        static RefPtr<FunctionBinary> OrInt64(Library &library);
        static RefPtr<FunctionBinary> AndInt64(Library &library);
        static RefPtr<FunctionBinary> XorInt64(Library &library);

        static RefPtr<FunctionBinary> MinInt8(Library &library);
        static RefPtr<FunctionBinary> MaxInt8(Library &library);
        static RefPtr<FunctionBinary> MinInt16(Library &library);
        static RefPtr<FunctionBinary> MaxInt16(Library &library);
        static RefPtr<FunctionBinary> MinInt32(Library &library);
        static RefPtr<FunctionBinary> MaxInt32(Library &library);
        static RefPtr<FunctionBinary> MinInt64(Library &library);
        static RefPtr<FunctionBinary> MaxInt64(Library &library);

        static RefPtr<FunctionBinary> PlusUInt8(Library &library);
        static RefPtr<FunctionBinary> MinusUInt8(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusUInt8(Library &library);
        static RefPtr<FunctionBinary> MultUInt8(Library &library);
        static RefPtr<FunctionBinary> DivUInt8(Library &library);
        static RefPtr<FunctionBinary> ReverseDivUInt8(Library &library);
        static RefPtr<FunctionBinary> TakeFirstUInt8(Library &library);
        static RefPtr<FunctionBinary> TakeSecondUInt8(Library &library);
        static RefPtr<FunctionBinary> PlusUInt16(Library &library);
        static RefPtr<FunctionBinary> MinusUInt16(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusUInt16(Library &library);
        static RefPtr<FunctionBinary> MultUInt16(Library &library);
        static RefPtr<FunctionBinary> DivUInt16(Library &library);
        static RefPtr<FunctionBinary> ReverseDivUInt16(Library &library);
        static RefPtr<FunctionBinary> TakeFirstUInt16(Library &library);
        static RefPtr<FunctionBinary> TakeSecondUInt16(Library &library);
        static RefPtr<FunctionBinary> PlusUInt32(Library &library);
        static RefPtr<FunctionBinary> MinusUInt32(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusUInt32(Library &library);
        static RefPtr<FunctionBinary> MultUInt32(Library &library);
        static RefPtr<FunctionBinary> DivUInt32(Library &library);
        static RefPtr<FunctionBinary> ReverseDivUInt32(Library &library);
        static RefPtr<FunctionBinary> TakeFirstUInt32(Library &library);
        static RefPtr<FunctionBinary> TakeSecondUInt32(Library &library);
        static RefPtr<FunctionBinary> PlusUInt64(Library &library);
        static RefPtr<FunctionBinary> MinusUInt64(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusUInt64(Library &library);
        static RefPtr<FunctionBinary> MultUInt64(Library &library);
        static RefPtr<FunctionBinary> DivUInt64(Library &library);
        static RefPtr<FunctionBinary> ReverseDivUInt64(Library &library);
        static RefPtr<FunctionBinary> TakeFirstUInt64(Library &library);
        static RefPtr<FunctionBinary> TakeSecondUInt64(Library &library);

        static RefPtr<FunctionBinary> OrUInt8(Library &library);
        static RefPtr<FunctionBinary> AndUInt8(Library &library);
        static RefPtr<FunctionBinary> XorUInt8(Library &library);
        static RefPtr<FunctionBinary> OrUInt16(Library &library);
        static RefPtr<FunctionBinary> AndUInt16(Library &library);
        static RefPtr<FunctionBinary> XorUInt16(Library &library);
        static RefPtr<FunctionBinary> OrUInt32(Library &library);
        static RefPtr<FunctionBinary> AndUInt32(Library &library);
        static RefPtr<FunctionBinary> XorUInt32(Library &library);
        static RefPtr<FunctionBinary> OrUInt64(Library &library);
        static RefPtr<FunctionBinary> AndUInt64(Library &library);
        static RefPtr<FunctionBinary> XorUInt64(Library &library);

        static RefPtr<FunctionBinary> MinUInt8(Library &library);
        static RefPtr<FunctionBinary> MaxUInt8(Library &library);
        static RefPtr<FunctionBinary> MinUInt16(Library &library);
        static RefPtr<FunctionBinary> MaxUInt16(Library &library);
        static RefPtr<FunctionBinary> MinUInt32(Library &library);
        static RefPtr<FunctionBinary> MaxUInt32(Library &library);
        static RefPtr<FunctionBinary> MinUInt64(Library &library);
        static RefPtr<FunctionBinary> MaxUInt64(Library &library);

        static RefPtr<FunctionBinary> PlusFloat32(Library &library);
        static RefPtr<FunctionBinary> MinusFloat32(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusFloat32(Library &library);
        static RefPtr<FunctionBinary> MultFloat32(Library &library);
        static RefPtr<FunctionBinary> DivFloat32(Library &library);
        static RefPtr<FunctionBinary> ReverseDivFloat32(Library &library);
        static RefPtr<FunctionBinary> TakeFirstFloat32(Library &library);
        static RefPtr<FunctionBinary> TakeSecondFloat32(Library &library);

        static RefPtr<FunctionBinary> PlusFloat64(Library &library);
        static RefPtr<FunctionBinary> MinusFloat64(Library &library);
        static RefPtr<FunctionBinary> ReverseMinusFloat64(Library &library);
        static RefPtr<FunctionBinary> MultFloat64(Library &library);
        static RefPtr<FunctionBinary> DivFloat64(Library &library);
        static RefPtr<FunctionBinary> ReverseDivFloat64(Library &library);
        static RefPtr<FunctionBinary> TakeFirstFloat64(Library &library);
        static RefPtr<FunctionBinary> TakeSecondFloat64(Library &library);

        static RefPtr<FunctionBinary> MinFloat32(Library &library);
        static RefPtr<FunctionBinary> MaxFloat32(Library &library);

        static RefPtr<FunctionBinary> MinFloat64(Library &library);
        static RefPtr<FunctionBinary> MaxFloat64(Library &library);

    private:
        // friend class Library;
        // todo: static void InitFunctions(Library &library);
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAFUNCTIONS_HPP
