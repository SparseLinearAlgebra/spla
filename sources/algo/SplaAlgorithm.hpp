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

#ifndef SPLA_SPLAALGORITHM_HPP
#define SPLA_SPLAALGORITHM_HPP

#include <algo/SplaAlgorithmParams.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class Algorithm
     * @brief Algorithm interface
     *
     * Provides interface to dynamically select and execute the best suitable algorithm
     * for required operation and input params. Operates on separate objects, such
     * as matrix or vector blocks and outputs single operation result.
     *
     * Inherit from this class to implement custom algorithms.
     *
     * @see AlgorithmParams
     * @see AlgorithmManager
     */
    class Algorithm : public RefCnt {
    public:
        ~Algorithm() override = default;

        /** General type of the algorithm */
        enum class Type {
            /** Matrix-matrix element wise addition */
            MatrixEWiseAdd,
            /** Matrix-matrix element wise multiplication */
            MatrixEWiseMult,
            /** Matrix nnz elements reduce to scalar */
            MatrixReduceScalar,
            /** Vector-scalar assignment */
            VectorAssign,
            /** Vector nnz elements reduce */
            VectorReduce,
            /** Vector-vector element wise addition */
            VectorEWiseAdd,
            /** Vector-vector element wise multiplication */
            VectorEWiseMult,
            /** Matrix-matrix multiplication */
            MxM,
            /** Matrix-vector multiplication */
            MxV,
            /** Vector-matrix multiplication */
            VxM,
            /** Matrix block transpose */
            Transpose
        };

        /**
         * Select this algorithm for provided params.
         * @param params Set of algorithm input/output params.
         * @return True if this algorithm can be invoked with this params.
         */
        virtual bool Select(const AlgorithmParams &params) const = 0;

        /**
         * Process specified params in this thread.
         * Invoked by algorithm manager.
         * @param params Set of algorithm input/output params.
         */
        virtual void Process(AlgorithmParams &params) = 0;

        /**
         * Get algorithm type.
         * @return Algorithm type.
         */
        virtual Type GetType() const = 0;

        /**
         * Get algorithm name.
         * @return Algorithm unique name.
         */
        virtual std::string GetName() const = 0;
    };

    namespace {
        /** @return String name of algorithm type */
        inline const char *AlgorithmTypeToStr(Algorithm::Type type) {
            switch (type) {
                case Algorithm::Type::MatrixEWiseAdd:
                    return "MatrixEWiseAdd";
                case Algorithm::Type::MatrixEWiseMult:
                    return "MatrixEWiseMult";
                case Algorithm::Type::VectorAssign:
                    return "VectorAssign";
                case Algorithm::Type::VectorEWiseAdd:
                    return "VectorEWiseAdd";
                case Algorithm::Type::VectorEWiseMult:
                    return "VectorEWiseMult";
                case Algorithm::Type::MxM:
                    return "MxM";
                case Algorithm::Type::MxV:
                    return "MxV";
                case Algorithm::Type::VxM:
                    return "VxM";
                case Algorithm::Type::Transpose:
                    return "Transpose";
                case Algorithm::Type::VectorReduce:
                    return "VectorReduce";
                case Algorithm::Type::MatrixReduceScalar:
                    return "MatrixReduceScalar";
            }
        }
    }// namespace

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAALGORITHM_HPP
