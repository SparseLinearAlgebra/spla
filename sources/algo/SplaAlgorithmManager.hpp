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

#ifndef SPLA_SPLAALGORITHMMANAGER_HPP
#define SPLA_SPLAALGORITHMMANAGER_HPP

#include <algo/SplaAlgorithm.hpp>
#include <core/SplaTaskBuilder.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <unordered_map>
#include <vector>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class AlgorithmManager
     * @brief Manages particular algorithms implementations.
     *
     * Allows to register a number of different algorithms for a particular
     * processing task. Provides functionality to dispatch algorithm in the same
     * thread or to create askfloat task with selected algorithm.
     *
     * For example, for MxM operation of matrix blocks can store several algorithms,
     * such as MxM_csr, MxM_coo and etc. and choose suitable for provided params.
     */
    class AlgorithmManager final : public RefCnt {
    public:
        explicit AlgorithmManager(Library &library);
        ~AlgorithmManager() override = default;

        void Register(const RefPtr<Algorithm> &algo);
        void Dispatch(Algorithm::Type type, AlgorithmParams &params);
        void Dispatch(Algorithm::Type type, const RefPtr<AlgorithmParams> &params);
        tf::Task Dispatch(Algorithm::Type type, const RefPtr<AlgorithmParams> &params, TaskBuilder &builder);

    private:
        RefPtr<Algorithm> SelectAlgorithm(Algorithm::Type type, const AlgorithmParams &params);

    private:
        using AlgorithmList = std::vector<RefPtr<Algorithm>>;
        using AlgorithmMap = std::unordered_map<Algorithm::Type, AlgorithmList>;

        AlgorithmMap mAlgorithms;
        Library &mLibrary;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAALGORITHMMANAGER_HPP