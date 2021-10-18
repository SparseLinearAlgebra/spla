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

#include <algo/SplaAlgorithmManager.hpp>
#include <cassert>
#include <core/SplaError.hpp>

spla::AlgorithmManager::AlgorithmManager(Library &library) : mLibrary(library) {
}

void spla::AlgorithmManager::Register(const spla::RefPtr<spla::Algorithm> &algo) {
    CHECK_RAISE_ERROR(algo.IsNotNull(), InvalidArgument, "Passed null processor");

    Algorithm::Type type = algo->GetType();
    auto list = mAlgorithms.find(type);

    if (list == mAlgorithms.end())
        list = mAlgorithms.emplace(type, AlgorithmList()).first;

    list->second.push_back(algo);
}

void spla::AlgorithmManager::Dispatch(spla::Algorithm::Type type, const spla::RefPtr<spla::AlgorithmParams> &params) {
    assert(params.IsNotNull());
    auto algorithm = SelectAlgorithm(type, params);
    algorithm->Process(*params);
}

tf::Task spla::AlgorithmManager::Dispatch(spla::Algorithm::Type type, const spla::RefPtr<spla::AlgorithmParams> &params, tf::Taskflow &taskflow) {
    assert(params.IsNotNull());
    auto algorithm = SelectAlgorithm(type, params);
    return taskflow.emplace([=]() {
        algorithm->Process(*params);
    });
}

spla::RefPtr<spla::Algorithm> spla::AlgorithmManager::SelectAlgorithm(spla::Algorithm::Type type, const spla::RefPtr<spla::AlgorithmParams> &params) {
    auto iter = mAlgorithms.find(type);

    CHECK_RAISE_ERROR(iter != mAlgorithms.end(), InvalidState,
                      "No algorithms for such type=" << AlgorithmTypeToStr(type));

    const auto &algorithms = iter->second;

    // NOTE: Iterate through all processors for this operation and
    // select the first one, which meets requirements
    for (auto &algorithm : algorithms)
        if (algorithm->Select(*params))
            return algorithm;

    RAISE_ERROR(InvalidState,
                "Failed to find suitable algorithm for the type=" << AlgorithmTypeToStr(type));
}
