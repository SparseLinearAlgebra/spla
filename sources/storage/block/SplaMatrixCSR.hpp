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

#ifndef SPLA_SPLAMATRIXCSR_HPP
#define SPLA_SPLAMATRIXCSR_HPP

#include <storage/block/SplaMatrixCOO.hpp>

#include <boost/compute/command_queue.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    class MatrixCSR final : public MatrixCOO {
    public:
        using Indices = MatrixCOO::Indices;
        using Values = MatrixCOO::Values;

        ~MatrixCSR() override = default;

        [[nodiscard]] const Indices &GetRowsOffsets() const noexcept;

        [[nodiscard]] const Indices &GetRowLengths() const noexcept;

        static RefPtr<MatrixCSR> Make(std::size_t nrows, std::size_t ncols, std::size_t nvals, Indices rows, Indices cols, Values vals, boost::compute::command_queue& queue);
        static RefPtr<MatrixCSR> Make(const RefPtr<MatrixCOO> &block, boost::compute::command_queue& queue);

    private:
        MatrixCSR(std::size_t nrows, std::size_t ncols, std::size_t nvals, Indices rows, Indices cols, Values vals, boost::compute::command_queue& queue);

        Indices mRowOffsets;
        Indices mRowLengths;
    };

    /**
     * @}
     */

}

#endif//SPLA_SPLAMATRIXCSR_HPP
