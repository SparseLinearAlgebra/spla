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

#ifndef SPLA_SPLAMATRIXCOO_HPP
#define SPLA_SPLAMATRIXCOO_HPP

#include <boost/compute.hpp>
#include <storage/SplaMatrixBlock.hpp>
#include <string>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    class MatrixCOO final : public MatrixBlock {
    public:
        using Indices = boost::compute::vector<unsigned int>;
        using Values = boost::compute::vector<unsigned char>;

        ~MatrixCOO() override = default;

        [[nodiscard]] const Indices &GetRows() const noexcept;

        [[nodiscard]] const Indices &GetCols() const noexcept;

        [[nodiscard]] const Values &GetVals() const noexcept;

        [[nodiscard]] std::string ToString() const;

        static RefPtr<MatrixCOO> Make(size_t nrows, size_t ncols, size_t nvals, Indices rows, Indices cols, Values vals);

    private:
        MatrixCOO(size_t nrows, size_t ncols, size_t nvals, Indices rows, Indices cols, Values vals);

        Indices mRows;
        Indices mCols;
        Values mVals;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAMATRIXCOO_HPP
