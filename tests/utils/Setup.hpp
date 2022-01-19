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

#ifndef SPLA_SETUP_HPP
#define SPLA_SETUP_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <spla-cpp/Spla.hpp>

namespace utils {

    template<typename Callable>
    inline void testBlocks(const std::vector<std::size_t> &blocksSizes, const std::string &platform, std::size_t workersCount, Callable callable) {
        for (std::size_t blockSize : blocksSizes) {
            spla::Library::Config config;

            config.SetDeviceType(spla::Library::Config::GPU);
            config.SetBlockSize(blockSize);

            if (!platform.empty())
                config.SetPlatform(platform);
            if (workersCount)
                config.SetWorkersCount(workersCount);

            spla::Library library(config);
            callable(library);
        }
    }

    template<typename Callable>
    inline void testBlocks(const std::vector<std::size_t> &blocksSizes, Callable callable) {
        testBlocks(blocksSizes, "", 0, std::forward<Callable>(callable));
    }

    template<typename T>
    inline spla::RefPtr<spla::DataScalar> GetData(T s, spla::Library &library) {
        T *scalar = new T(s);
        auto data = spla::DataScalar::Make(library);
        data->SetValue(scalar);
        data->SetReleaseProc([=](void *scalar) { delete ((T *) scalar); });
        return data;
    }

}// namespace utils

#endif//SPLA_SETUP_HPP
