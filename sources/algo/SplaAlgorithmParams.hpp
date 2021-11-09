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

#ifndef SPLA_SPLAALGORITHMPARAMS_HPP
#define SPLA_SPLAALGORITHMPARAMS_HPP

#include <core/SplaDeviceManager.hpp>
#include <spla-cpp/SplaFunctionBinary.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <storage/SplaVectorBlock.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class AlgorithmParams
     * @brief Params passed as input to algorithm for execution.
     *
     * Base class used to pack algorithm input/output params to
     * pass to algorithm manager for algorithm selection and execution.
     * Inherit from this class to extend params set for custom algorithms implementation.
     */
    class AlgorithmParams : public RefCnt {
    public:
        ~AlgorithmParams() override = default;

        /** Library instance */
        std::shared_ptr<LibraryPrivate> library;
        /** Node desc */
        RefPtr<Descriptor> desc;
        /** Device id for execution */
        DeviceManager::DeviceId deviceId;
    };

    class ParamsVectorEWiseAdd final : public AlgorithmParams {
    public:
        ~ParamsVectorEWiseAdd() override = default;

        bool hasMask;
        RefPtr<VectorBlock> w;
        RefPtr<VectorBlock> mask;
        RefPtr<FunctionBinary> op;
        RefPtr<VectorBlock> a;
        RefPtr<VectorBlock> b;
        RefPtr<Type> type;
    };

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_SPLAALGORITHMPARAMS_HPP
