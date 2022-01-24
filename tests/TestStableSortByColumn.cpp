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

#include <Testing.hpp>
#include <compute/SplaStableSortByColumn.hpp>

using Key = std::uint32_t;
using Val = std::uint8_t;

void TestStableSortByColumn(
        const std::vector<Key> &rows,
        const std::vector<Key> &cols,
        const std::vector<Val> &values,

        const std::vector<Key> &rowsExpected,
        const std::vector<Key> &colsExpected,
        const std::vector<Val> &valsExpected) {
    namespace compute = boost::compute;

    // get the default compute device
    compute::device gpu = compute::system::default_device();

    // create a compute context and command queue
    compute::context ctx(gpu);
    compute::command_queue queue(ctx, gpu);

    compute::vector<Key> dRows(rows, queue);
    compute::vector<Key> dCols(cols, queue);
    compute::vector<Val> dVals(values, queue);

    spla::StableSortByColumn(
            dRows, dCols, dVals,
            sizeof(Val),
            queue);

    std::vector<Key> rowsActual(rows.size());
    std::vector<Key> colsActual(cols.size());
    std::vector<Val> valsActual(values.size());

    compute::copy(dRows.begin(), dRows.end(), rowsActual.begin(), queue);
    compute::copy(dCols.begin(), dCols.end(), colsActual.begin(), queue);
    compute::copy(dVals.begin(), dVals.end(), valsActual.begin(), queue);

    queue.finish();

    EXPECT_EQ(rowsExpected, rowsActual);
    EXPECT_EQ(colsExpected, colsActual);
    EXPECT_EQ(valsExpected, valsActual);
}

TEST(TestStableSortByColumn, Basic) {
    TestStableSortByColumn(
            {0, 0, 0, 0, 1, 1, 1, 1, 2},
            {3, 4, 2, 4, 2, 1, 5, 2, 2},
            {1, 2, 3, 4, 5, 6, 7, 8, 9},

            {0, 0, 0, 0, 1, 1, 1, 1, 2},
            {2, 3, 4, 4, 1, 2, 2, 5, 2},
            {3, 1, 2, 4, 6, 5, 8, 7, 9});
}

TEST(TestStableSortByColumn, Empty) {
    TestStableSortByColumn(
            {},
            {},
            {},
            {},
            {},
            {});
}

SPLA_GTEST_MAIN
