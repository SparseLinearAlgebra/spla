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

#ifndef SPLA_CPU_FORMATS_HPP
#define SPLA_CPU_FORMATS_HPP

#include <spla/config.hpp>

#include <core/tdecoration.hpp>
#include <util/pair_hash.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CpuDenseVec
     * @brief CPU one-dim array for dense vector representation
     *
     * @tparam T
     */
    template<typename T>
    class CpuDenseVec : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuDenseVec;

        ~CpuDenseVec() override = default;

        using Reduce = std::function<T(T accum, T added)>;

        std::vector<T> Ax{};
        Reduce         reduce = [](T, T a) { return a; };
    };

    /**
     * @class CpuCooVec
     * @brief CPU list-of-coordinates sparse vector representation
     *
     * @tparam T
     */
    template<typename T>
    class CpuCooVec : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuCooVec;

        ~CpuCooVec() override = default;

        using Reduce = std::function<T(T accum, T added)>;

        std::vector<uint> Ai;
        std::vector<T>    Ax;
    };

    /**
     * @class CpuLil
     * @brief CPU list-of-list matrix format for fast incremental build
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuLil : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuLil;

        ~CpuLil() override = default;

        using Entry  = std::pair<uint, T>;
        using Row    = std::vector<Entry>;
        using Reduce = std::function<T(T accum, T added)>;

        std::vector<Row> Ar{};
        Reduce           reduce = [](T, T a) { return a; };
    };

    /**
     * @class CpuDok
     * @brief Dictionary of keys sparse matrix format
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuDok : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuDok;

        ~CpuDok() override = default;

        using Key    = std::pair<uint, uint>;
        using Reduce = std::function<T(T accum, T added)>;

        std::unordered_map<Key, T, pair_hash> Ax;
        Reduce                                reduce = [](T, T a) { return a; };
    };

    /**
     * @class CpuCoo
     * @brief CPU list of coordinates matrix format
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuCoo : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuCoo;

        ~CpuCoo() override = default;

        std::vector<uint> Ai;
        std::vector<uint> Aj;
        std::vector<T>    Ax;
    };

    /**
     * @class CpuCsr
     * @brief CPU compressed sparse row matrix format
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuCsr : public TDecoration<T> {
    public:
        static constexpr Format FORMAT = Format::CpuCsr;

        ~CpuCsr() override = default;

        std::vector<uint> Ap;
        std::vector<uint> Aj;
        std::vector<T>    Ax;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_FORMATS_HPP
