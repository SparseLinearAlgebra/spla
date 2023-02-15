/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_CL_FORMATS_HPP
#define SPLA_CL_FORMATS_HPP

#include <spla/config.hpp>

#include <core/tdecoration.hpp>
#include <util/pair_hash.hpp>

#include <opencl/cl_accelerator.hpp>

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
     * @class CLDenseVec
     * @brief OpenCL one-dim array for dense vector representation
     *
     * @tparam T Type of values stored
     */
    template<typename T>
    class CLDenseVec : public TDecoration<T> {
    public:
        static constexpr FormatVector FORMAT = FormatVector::AccDense;

        ~CLDenseVec() override = default;

        cl::Buffer Ax;
    };

    /**
     * @class CLCooVec
     * @brief OpenCL list-of-coordinates sparse vector representation
     *
     * @tparam T Type of values stored
     */
    template<typename T>
    class CLCooVec : public TDecoration<T> {
    public:
        static constexpr FormatVector FORMAT = FormatVector::AccCoo;

        ~CLCooVec() override = default;

        cl::Buffer Ai;
        cl::Buffer Ax;
    };

    /**
     * @class CLCsr
     * @brief OpenCL compressed sparse row matrix representation
     *
     * @tparam T Type of values stored
     */
    template<typename T>
    class CLCsr : public TDecoration<T> {
    public:
        static constexpr FormatMatrix FORMAT = FormatMatrix::AccCsr;

        ~CLCsr() override = default;

        cl::Buffer Ap;
        cl::Buffer Aj;
        cl::Buffer Ax;
    };


    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_FORMATS_HPP
