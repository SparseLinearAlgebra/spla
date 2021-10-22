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

#ifndef SPLA_SPLAMERGEBYKEY_HPP
#define SPLA_SPLAMERGEBYKEY_HPP

#include <boost/compute.hpp>

namespace spla {

    template<
            typename ItKeysABegin,
            typename ItKeysAEnd,
            typename ItValuesA,
            typename ItKeysBBegin,
            typename ItKeysBEnd,
            typename ItValuesB,
            typename ItKeysResult,
            typename ItValuesResult>
    auto MergeByKey(
            ItKeysABegin keysABegin,
            ItKeysAEnd keysAEnd,
            ItValuesA valuesA,
            ItKeysBBegin keysBBegin,
            ItKeysBEnd keysBEnd,
            ItValuesB valuesB,
            ItKeysResult keysResult,
            ItValuesResult valuesResult,
            boost::compute::command_queue &queue) {

        namespace compute = boost::compute;

        using Key = typename std::iterator_traits<ItKeysABegin>::value_type;
        using Value = typename std::iterator_traits<ItValuesA>::value_type;

        const compute::context &ctx = queue.get_context();

        const std::size_t aSize = std::distance(keysABegin, keysAEnd);
        const std::size_t bSize = std::distance(keysBBegin, keysBEnd);

        compute::vector<std::pair<Key, Value>> keyValuesA(aSize, ctx);
        compute::vector<std::pair<Key, Value>> keyValuesB(bSize, ctx);
        compute::vector<std::pair<Key, Value>> keyValuesRes(aSize + bSize, ctx);

        using ::boost::compute::lambda::_1;
        using ::boost::compute::lambda::_2;
        using ::boost::compute::lambda::get;

        compute::copy(
                keysABegin,
                keysAEnd,
                compute::make_transform_iterator(
                        keyValuesA.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                valuesA,
                valuesA + aSize,
                compute::make_transform_iterator(
                        keyValuesA.begin(),
                        get<1>(_1)),
                queue);

        compute::copy(
                keysBBegin,
                keysBEnd,
                compute::make_transform_iterator(
                        keyValuesB.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                valuesB,
                valuesB + bSize,
                compute::make_transform_iterator(
                        keyValuesB.begin(),
                        get<1>(_1)),
                queue);

        auto itResEnd = compute::merge(
                keyValuesA.begin(), keyValuesA.end(),
                keyValuesB.begin(), keyValuesB.end(),
                keyValuesRes.begin(),
                get<0>(_1) < get<0>(_2),
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        keyValuesRes.begin(),
                        get<0>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<0>(_1)),
                keysResult,
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        keyValuesRes.begin(),
                        get<1>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<1>(_1)),
                valuesResult,
                queue);

        const std::size_t resultSize = std::distance(keyValuesRes.begin(), itResEnd);

        return std::pair{
                keysResult + resultSize,
                valuesResult + resultSize};
    }

}// namespace spla

#endif//SPLA_SPLAMERGEBYKEY_HPP
