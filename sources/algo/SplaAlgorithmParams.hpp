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
#include <spla-cpp/SplaData.hpp>
#include <spla-cpp/SplaDescriptor.hpp>
#include <spla-cpp/SplaFunctionBinary.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <storage/SplaMatrixBlock.hpp>
#include <storage/SplaScalarStorage.hpp>
#include <storage/SplaScalarValue.hpp>
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

        /** Node desc */
        RefPtr<Descriptor> desc;
        /** Device id for execution */
        DeviceManager::DeviceId deviceId;
    };

    /** Blocked element-wise matrix addition params */
    class ParamsMatrixEWiseAdd final : public AlgorithmParams {
    public:
        ~ParamsMatrixEWiseAdd() override = default;

        bool hasMask = false;
        RefPtr<MatrixBlock> w;
        RefPtr<MatrixBlock> mask;
        RefPtr<FunctionBinary> op;
        RefPtr<MatrixBlock> a;
        RefPtr<MatrixBlock> b;
        RefPtr<Type> type;
    };

    /** Blocked element-wise vector addition params */
    class ParamsVectorEWiseAdd final : public AlgorithmParams {
    public:
        ~ParamsVectorEWiseAdd() override = default;

        bool hasMask = false;
        RefPtr<VectorBlock> w;
        RefPtr<VectorBlock> mask;
        RefPtr<FunctionBinary> op;
        RefPtr<VectorBlock> a;
        RefPtr<VectorBlock> b;
        RefPtr<Type> type;
    };

    /** Scalar addition params */
    class ParamsScalarEWiseAdd final : public AlgorithmParams {
    public:
        ~ParamsScalarEWiseAdd() override = default;

        RefPtr<ScalarValue> w;    // destination scalar of type tw
        RefPtr<FunctionBinary> op;// 'plus' function of type ta x tb -> tw
        RefPtr<ScalarValue> a;    // left applicant scalar of type ta
        RefPtr<ScalarValue> b;    // right applicant scalar of type tb
        RefPtr<Type> tw;          // the type tw
    };

    /** Blocked matrix-matrix multiply params */
    class ParamsMxM final : public AlgorithmParams {
    public:
        ~ParamsMxM() override = default;

        bool hasMask = false;       // true if must apply mask
        RefPtr<MatrixBlock> w;      // tw
        RefPtr<MatrixBlock> mask;   // if has mask, must apply this
        RefPtr<FunctionBinary> mult;// f: ta x tb -> tw
        RefPtr<FunctionBinary> add; // f: tw x tw -> tw
        RefPtr<MatrixBlock> a;      // ta
        RefPtr<MatrixBlock> b;      // tb
        RefPtr<Type> ta;
        RefPtr<Type> tb;
        RefPtr<Type> tw;
    };

    /** Blocked vector-matrix multiply params */
    class ParamsVxM final : public AlgorithmParams {
    public:
        ~ParamsVxM() override = default;

        bool hasMask = false;       // true if must apply mask
        RefPtr<VectorBlock> w;      // tw
        RefPtr<VectorBlock> mask;   // if has mask, must apply this
        RefPtr<FunctionBinary> mult;// f: ta x tb -> tw
        RefPtr<FunctionBinary> add; // f: tw x tw -> tw
        RefPtr<VectorBlock> a;      // ta
        RefPtr<MatrixBlock> b;      // tb
        RefPtr<Type> ta;
        RefPtr<Type> tb;
        RefPtr<Type> tw;
    };

    /** Blocked vector-scalar assignment params */
    class ParamsVectorAssign final : public AlgorithmParams {
    public:
        ~ParamsVectorAssign() override = default;

        std::size_t size;        // size of block
        bool hasMask = false;    // if must apply mask
        RefPtr<VectorBlock> w;   // type
        RefPtr<VectorBlock> mask;// if has mask, must apply this
        RefPtr<ScalarValue> s;   // type (or null)
        RefPtr<Type> type;
    };

    /** Read vector block data */
    class ParamsVectorRead final : public AlgorithmParams {
    public:
        ~ParamsVectorRead() override = default;

        std::size_t offset = 0;  // offset of rows in d to write
        std::size_t byteSize = 0;// size of values
        std::size_t baseI = 0;   // base row index to add to each v index
        RefPtr<VectorBlock> v;   // block to read
        RefPtr<DataVector> d;    // destination
    };

    /** Transpose matrix block */
    class ParamsTranspose final : public AlgorithmParams {
    public:
        ~ParamsTranspose() override = default;

        bool hasMask = false;    // true if must apply mask
        RefPtr<MatrixBlock> mask;// apply if required to the a
        RefPtr<MatrixBlock> w;   // of type t
        RefPtr<MatrixBlock> a;   // of type t
        RefPtr<Type> type;       // t
    };

    /** Vector reduce params */
    class ParamsVectorReduce final : public AlgorithmParams {
    public:
        ~ParamsVectorReduce() override = default;

        RefPtr<VectorBlock> vec;      // vector block to reduce of type t
        RefPtr<FunctionBinary> reduce;// reduce operation of type t x t -> t
        RefPtr<ScalarValue> scalar;   // scalar of type t
        RefPtr<Type> type;            // the type t itself
    };

    /** Matrix reduce to scalar params */
    class ParamsMatrixReduceScalar final : public AlgorithmParams {
    public:
        ~ParamsMatrixReduceScalar() override = default;

        bool hasMask = false;         // true when must apply mask
        RefPtr<MatrixBlock> matrix;   // matrix block to reduce of type t
        RefPtr<MatrixBlock> mask;     // mask to apply on matrix
        RefPtr<FunctionBinary> reduce;// reduce function of type t x t -> t
        RefPtr<ScalarValue> scalar;   // destination scalar of type t
        RefPtr<Type> type;            // the type t itself
    };

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_SPLAALGORITHMPARAMS_HPP
