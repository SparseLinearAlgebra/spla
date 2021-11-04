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

#include <tuple>

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

    template<
            typename ItKeysABegin1,
            typename ItKeysABegin2,
            typename ItKeysAEnd1,
            typename ItValuesA,
            typename ItKeysBBegin1,
            typename ItKeysBBegin2,
            typename ItKeysBEnd1,
            typename ItValuesB,
            typename ItKeysResult1,
            typename ItKeysResult2,
            typename ItValuesResult>
    auto MergeByPairKey(
            ItKeysABegin1 keysABeginFirst,
            ItKeysABegin2 keysABeginSecond,
            ItKeysAEnd1 keysAEndFirst,
            ItValuesA valuesA,
            ItKeysBBegin1 keysBBeginFirst,
            ItKeysBBegin2 keysBBeginSecond,
            ItKeysBEnd1 keysBEndFirst,
            ItValuesB valuesB,
            ItKeysResult1 keysResultFirst,
            ItKeysResult2 keysResultSecond,
            ItValuesResult valuesResult,
            boost::compute::command_queue &queue) {
        namespace compute = boost::compute;

        using Key1 = typename std::iterator_traits<ItKeysABegin1>::value_type;
        using Key2 = typename std::iterator_traits<ItKeysABegin2>::value_type;
        using Value = typename std::iterator_traits<ItValuesA>::value_type;

        using KeyValue = boost::tuple<Key1, Key2, Value>;

        const compute::context &ctx = queue.get_context();

        const std::size_t aSize = std::distance(keysABeginFirst, keysAEndFirst);
        const std::size_t bSize = std::distance(keysBBeginFirst, keysBEndFirst);

        compute::vector<KeyValue> keyValuesA(aSize, ctx);
        compute::vector<KeyValue> keyValuesB(bSize, ctx);
        compute::vector<KeyValue> keyValuesRes(aSize + bSize, ctx);

        using compute::lambda::_1;
        using compute::lambda::_2;
        using compute::lambda::get;

        // Copy A keys
        compute::copy(
                keysABeginFirst,
                keysAEndFirst,
                compute::make_transform_iterator(
                        keyValuesA.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                keysABeginSecond,
                keysABeginSecond + aSize,
                compute::make_transform_iterator(
                        keyValuesA.begin(),
                        get<1>(_1)),
                queue);

        // Copy A values
        compute::copy(
                valuesA,
                valuesA + aSize,
                compute::make_transform_iterator(
                        keyValuesA.begin(),
                        get<2>(_1)),
                queue);

        // Copy B keys
        compute::copy(
                keysBBeginFirst,
                keysBEndFirst,
                compute::make_transform_iterator(
                        keyValuesB.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                keysBBeginSecond,
                keysBBeginSecond + bSize,
                compute::make_transform_iterator(
                        keyValuesB.begin(),
                        get<1>(_1)),
                queue);

        // Copy B values
        compute::copy(
                valuesB,
                valuesB + bSize,
                compute::make_transform_iterator(
                        keyValuesB.begin(),
                        get<2>(_1)),
                queue);

        auto itResEnd = compute::merge(
                keyValuesA.begin(), keyValuesA.end(),
                keyValuesB.begin(), keyValuesB.end(),
                keyValuesRes.begin(),
                get<0>(_1) < get<0>(_2) || (get<0>(_1) == get<0>(_2) && get<1>(_1) < get<1>(_2)),
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        keyValuesRes.begin(),
                        get<0>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<0>(_1)),
                keysResultFirst,
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        keyValuesRes.begin(),
                        get<1>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<1>(_1)),
                keysResultSecond,
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        keyValuesRes.begin(),
                        get<2>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<2>(_1)),
                valuesResult,
                queue);

        const std::size_t resultSize = std::distance(keyValuesRes.begin(), itResEnd);

        return std::tuple{
                keysResultFirst + resultSize,
                keysResultSecond + resultSize,
                valuesResult + resultSize};
    }

}// namespace spla

#endif//SPLA_SPLAMERGEBYKEY_HPP
