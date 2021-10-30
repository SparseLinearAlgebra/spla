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

    /**
     * @brief Merge value sequences by the given key
     *
     * @param keysABegin First keys begin iterator
     * @param keysAEnd First keys end iterator
     * @param valuesA First values begin iterator
     * @param keysBBegin Second keys begin iterator
     * @param keysBEnd Second keys end iterator
     * @param valuesB Second values end iterator
     * @param keysResult Merged keys begin iterator
     * @param valuesResult Merged values begin iterator
     * @param queue OpenCL command queue @p boost::compute::command_queue where operations will be executed
     * @return Pair of iterators. First points at the end of merged keys and
     * the second one at merged values.
     */
    template<
            typename ItKeysABegin,
            typename ItKeysAEnd,
            typename ItValuesA,
            typename ItKeysBBegin,
            typename ItKeysBEnd,
            typename ItValuesB,
            typename ItKeysResult,
            typename ItValuesResult>
    std::pair<ItKeysResult, ItValuesResult> MergeByKey(
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

    /**
     * @brief Merge two sorted key sequences
     *
     * @param keysABegin First keys begin iterator
     * @param keysAEnd First keys end iterator
     * @param keysBBegin Second keys begin iterator
     * @param keysBEnd Second keys end iterator
     * @param keysResult Merged keys begin iterator
     * @param queue OpenCL command queue @p boost::compute::command_queue where operations will be executed
     * @return Iterator which points at the end of merged keys sequence
     */
    template<
            typename ItKeysABegin,
            typename ItKeysAEnd,
            typename ItKeysBBegin,
            typename ItKeysBEnd,
            typename ItKeysResult>
    ItKeysResult MergeKeys(
            ItKeysABegin keysABegin,
            ItKeysAEnd keysAEnd,
            ItKeysBBegin keysBBegin,
            ItKeysBEnd keysBEnd,
            ItKeysResult keysResult,
            boost::compute::command_queue &queue) {

        using boost::compute::lambda::_1;
        using boost::compute::lambda::_2;

        return boost::compute::merge(
                keysABegin, keysAEnd,
                keysBBegin, keysBEnd,
                keysResult,
                _1 < _2,
                queue);
    }

    /**
     * @brief Merge values by composite key
     *
     * @param keysABeginFirst First set's first keys begin iterator
     * @param keysABeginSecond First set's second keys end iterator
     * @param keysAEndFirst First set's first keys end iterator
     * @param valuesA First set's values begin iterator
     * @param keysBBeginFirst Second set's first keys begin iterator
     * @param keysBBeginSecond Second set's second keys begin iterator
     * @param keysBEndFirst Second set's first keys end iterator
     * @param valuesB Second set's values begin iterator
     * @param keysResultFirst First merged keys begin iterator
     * @param keysResultSecond Second merged keys begin iterator
     * @param valuesResult Merged values begin iterator
     * @param queue OpenCL command queue @p boost::compute::command_queue where operations will be executed
     * @return Tuple of end iterators: {first_keys, second_keys, values}
     */
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
    std::tuple<ItKeysResult1, ItKeysResult2, ItValuesResult> MergeByPairKey(
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

        using ::boost::compute::lambda::_1;
        using ::boost::compute::lambda::_2;
        using ::boost::compute::lambda::get;

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
                (get<0>(_1) == get<0>(_2) && get<1>(_1) < get<1>(_2)) || get<0>(_1) < get<0>(_2),
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

        return std::tuple{keysResultFirst + resultSize,
                          keysResultSecond + resultSize,
                          valuesResult + resultSize};
    }

    /**
     * @brief Merge two composite keys sequences
     *
     * @param keysABeginFirst First set's first keys begin iterator
     * @param keysABeginSecond First set's second keys end iterator
     * @param keysAEndFirst First set's first keys end iterator
     * @param keysBBeginFirst Second set's first keys begin iterator
     * @param keysBBeginSecond Second set's second keys begin iterator
     * @param keysBEndFirst Second set's first keys end iterator
     * @param keysResultFirst First merged keys begin iterator
     * @param keysResultSecond Second merged keys begin iterator
     * @param queue OpenCL command queue @p boost::compute::command_queue where operations will be executed
     * @return Pair of end iterators: {first_keys, second_keys}
     */
    template<
            typename ItKeysABegin1,
            typename ItKeysABegin2,
            typename ItKeysAEnd1,
            typename ItKeysBBegin1,
            typename ItKeysBBegin2,
            typename ItKeysBEnd1,
            typename ItKeysResult1,
            typename ItKeysResult2>
    std::pair<ItKeysResult1, ItKeysResult2> MergePairKeys(
            ItKeysABegin1 keysABeginFirst,
            ItKeysABegin2 keysABeginSecond,
            ItKeysAEnd1 keysAEndFirst,
            ItKeysBBegin1 keysBBeginFirst,
            ItKeysBBegin2 keysBBeginSecond,
            ItKeysBEnd1 keysBEndFirst,
            ItKeysResult1 keysResultFirst,
            ItKeysResult2 keysResultSecond,
            boost::compute::command_queue &queue) {

        namespace compute = boost::compute;

        using Key1 = typename std::iterator_traits<ItKeysABegin1>::value_type;
        using Key2 = typename std::iterator_traits<ItKeysABegin2>::value_type;

        using CompKey = std::pair<Key1, Key2>;

        const compute::context &ctx = queue.get_context();

        const std::size_t aSize = std::distance(keysABeginFirst, keysAEndFirst);
        const std::size_t bSize = std::distance(keysBBeginFirst, keysBEndFirst);

        compute::vector<CompKey> compKeysA(aSize, ctx);
        compute::vector<CompKey> compKeysB(bSize, ctx);
        compute::vector<CompKey> compKeysRes(aSize + bSize, ctx);

        using ::boost::compute::lambda::_1;
        using ::boost::compute::lambda::_2;
        using ::boost::compute::lambda::get;

        // Copy A keys
        compute::copy(
                keysABeginFirst,
                keysAEndFirst,
                compute::make_transform_iterator(
                        compKeysA.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                keysABeginSecond,
                keysABeginSecond + aSize,
                compute::make_transform_iterator(
                        compKeysA.begin(),
                        get<1>(_1)),
                queue);

        // Copy B keys
        compute::copy(
                keysBBeginFirst,
                keysBEndFirst,
                compute::make_transform_iterator(
                        compKeysB.begin(),
                        get<0>(_1)),
                queue);

        compute::copy(
                keysBBeginSecond,
                keysBBeginSecond + bSize,
                compute::make_transform_iterator(
                        compKeysB.begin(),
                        get<1>(_1)),
                queue);

        auto itResEnd = compute::merge(
                compKeysA.begin(), compKeysA.end(),
                compKeysB.begin(), compKeysB.end(),
                compKeysRes.begin(),
                (get<0>(_1) == get<0>(_2) && get<1>(_1) < get<1>(_2)) || get<0>(_1) < get<0>(_2),
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        compKeysRes.begin(),
                        get<0>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<0>(_1)),
                keysResultFirst,
                queue);

        compute::copy(
                compute::make_transform_iterator(
                        compKeysRes.begin(),
                        get<1>(_1)),
                compute::make_transform_iterator(
                        itResEnd,
                        get<1>(_1)),
                keysResultSecond,
                queue);

        const std::size_t resultSize = std::distance(compKeysRes.begin(), itResEnd);

        return std::pair{keysResultFirst + resultSize,
                          keysResultSecond + resultSize};
    }

}// namespace spla

#endif//SPLA_SPLAMERGEBYKEY_HPP
