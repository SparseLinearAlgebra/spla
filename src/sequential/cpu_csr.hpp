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

#ifndef SPLA_CPU_CSR_HPP
#define SPLA_CPU_CSR_HPP

#include <spla/config.hpp>

#include <core/tdecoration.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CpuCsr
     * @brief CPU compressed sparse row matrix format
     *
     * @tparam T Type of elements
     */
    template<typename T>
    class CpuCsr : public TDecoration<T> {
    public:
        ~CpuCsr() override = default;

        void to_coo(std::vector<uint> &Ri, std::vector<uint> &Rj, std::vector<T> &Rx);

        std::vector<uint> Ap;
        std::vector<uint> Aj;
        std::vector<T>    Ax;
        uint              n_rows   = 0;
        uint              n_cols   = 0;
        uint              n_values = 0;
        uint              version  = 0;
    };

    template<typename T>
    void CpuCsr<T>::to_coo(std::vector<uint> &Ri, std::vector<uint> &Rj, std::vector<T> &Rx) {
        assert(Ri.size() == n_values);
        assert(Rj.size() == n_values);
        assert(Rx.size() == n_values);

        for (uint i = 0; i < n_rows; i++) {
            for (uint j = Ap[i]; j < Ap[i + 1]; j++) {
                Ri[j] = i;
                Rj[j] = Aj[j];
                Rx[j] = Ax[j];
            }
        }
    }


    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CPU_CSR_HPP
