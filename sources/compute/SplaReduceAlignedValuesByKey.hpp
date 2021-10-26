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

#ifndef SPLA_SPLAREDUCEALIGNEDVALUESBYKEY_HPP
#define SPLA_SPLAREDUCEALIGNEDVALUESBYKEY_HPP

#include <boost/compute/detail/meta_kernel.hpp>
#include <boost/compute/functional.hpp>
#include <boost/compute/type_traits/type_name.hpp>

namespace spla {

    /**
     * @brief Reduces aligned values on two keys simultaneous equality
     *
     * Aligned values is an array of values that are arranged
     * is an array of values that are arranged in a sequence in memory.
     * And the size of each size is equal to @b valuesByteSize.
     *
     * Reduction of adjacent values by the function @p function is performed
     * if and only if the @p predicate from the pairs of two keys is fulfilled.
     * Reduction is left associative.
     *
     * @param keys1First Iterator at the first key sequence begin
     * @param keys1Last Iterator at the first key sequence end
     * @param keys2First Iterator at the second key sequence begin
     * @param valuesFirst Iterator at the second
     * @param valuesByteSize Size of each value in bytes
     * @param keys1Result Iterator at the first keys result output
     * @param keys2Result Iterator at the second keys result output
     * @param valuesResult Iterator at the values result output
     * @param function Binary function. It must accept two
     * @code{.cpp} std::pair<FirstKey, SecondKey> @endcode
     * It must return if key pairs are equal.
     * @param predicate A function, which accepts
     *
     *  @p __global @p ValueType* @p acc - Global pointer at already accumulated value
     *  @p __global @p ValueType* @p cur - Global pointer at current value
     *  @p ValueType* @p res - Pointer at result
     *
     *  It must put combination of two values (acc, cur) to the res.
     * @param queue OpenCL command queue
     * @return Tuple of iterators, which are represent ends of output each stream: {keys1End, keys2End, valuesEnd}
     *
     * @note Size of @p value_type of any value iterator must be 1 byte.
     * So the type of the value is either @p char or @p uchar.
     */
    template<
            typename InputKey1Iterator, typename InputKey2Iterator, typename InputValueIterator,
            typename OutputKey1Iterator, typename OutputKey2Iterator, typename OutputValueIterator,
            typename BinaryFunction, typename BinaryPredicate>
    inline std::tuple<
            OutputKey1Iterator,
            OutputKey2Iterator,
            OutputValueIterator>
    ReduceAlignedValuesByPairKey(
            InputKey1Iterator keys1First, InputKey1Iterator keys1Last, InputKey2Iterator keys2First,
            InputValueIterator valuesFirst, const std::size_t valuesByteSize,
            OutputKey1Iterator keys1Result, OutputKey2Iterator keys2Result,
            OutputValueIterator valuesResult,
            BinaryFunction function,
            BinaryPredicate predicate,
            boost::compute::command_queue &queue) {
        namespace compute = boost::compute;

        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputKey1Iterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputKey2Iterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputValueIterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputKey1Iterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputKey2Iterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputValueIterator>::value);

        using ValueType = typename std::iterator_traits<InputValueIterator>::value_type;
        using Key1Type = typename std::iterator_traits<InputKey1Iterator>::value_type;
        using Key2Type = typename std::iterator_traits<InputKey2Iterator>::value_type;

        static_assert(sizeof(ValueType) == 1, "Size of result type must be 1 byte");

        const compute::context &context = queue.get_context();
        std::size_t count = compute::detail::iterator_range_size(keys1First, keys1Last);

        auto MakeOutputLastIterators = [&](std::size_t resultLen) {
            return std::tuple{
                    keys1First + resultLen,
                    keys2First + resultLen,
                    valuesResult + resultLen * valuesByteSize};
        };

        if (count < 1) {
            return MakeOutputLastIterators(count);
        }

        compute::detail::meta_kernel k("reduce_aligned_values_by_pair_key");
        std::size_t countArg = k.add_arg<compute::uint_>("count");
        std::size_t resultSizeArg = k.add_arg<compute::uint_ *>(compute::memory_object::global_memory, "result_size");

        auto accumulatedResultPtr = std::string("&") + k.template get_buffer_identifier<ValueType>(valuesResult.get_buffer()) + "[(size - 1) * " + std::to_string(valuesByteSize) + "]";
        auto currentValuePtr = std::string("&") + k.template get_buffer_identifier<ValueType>(valuesFirst.get_buffer()) + "[i * " + std::to_string(valuesByteSize) + "]";

        auto CopyLastFunctionCallToAccum = [&]() -> compute::detail::meta_kernel & {
            std::string byteIndexAccum = std::string("(size - 1) * ") + std::to_string(valuesByteSize) + " + byte_i";
            k << "for(uint byte_i = 0; byte_i < " << valuesByteSize << "; byte_i++) {\n"
              << valuesResult[k.var<compute::uint_>(byteIndexAccum)] << " = reduce_call_buffer[" << k.var<compute::uint_>("byte_i") << "];\n"
              << "}";
            return k;
        };

        auto InitAccumWithCurValue = [&](const std::string &i) {
            std::string byteIndexAccum = std::string("(size - 1) * ") + std::to_string(valuesByteSize) + " + byte_i";
            std::string byteIndexCurValue = std::string(i + " * ") + std::to_string(valuesByteSize) + " + byte_i";
            k << "for(uint byte_i = 0; byte_i < " << valuesByteSize << "; byte_i++) {\n"
              << valuesResult[k.var<compute::uint_>(byteIndexAccum)] << " = " << valuesFirst[k.var<compute::uint_>(byteIndexCurValue)] << ";\n"
              << "}";
        };

        auto predicateCall = predicate(
                k.var<std::pair<Key1Type, Key2Type>>(std::string("boost_make_pair(") +
                                                     compute::type_name<Key1Type>() + ", previous_key1, " +
                                                     compute::type_name<Key2Type>() + ", previous_key2)"),
                k.var<std::pair<Key1Type, Key2Type>>(std::string("boost_make_pair(") +
                                                     compute::type_name<Key1Type>() + ", key1, " +
                                                     compute::type_name<Key2Type>() + ", key2)"));

        k << k.decl<Key1Type>("previous_key1") << " = " << keys1First[0] << ";\n"
          << k.decl<Key1Type>("previous_key2") << " = " << keys2First[0] << ";\n"
          << k.decl<Key1Type>("key1") << ";\n"
          << k.decl<Key2Type>("key2") << ";\n"
          << k.decl<compute::uint_>("size") << " = 1;\n"
          << k.decl<compute::uchar_>("reduce_call_buffer") << " [" << valuesByteSize << "];\n"
          << keys1Result[0] << " = previous_key1;\n"
          << keys2Result[0] << " = previous_key2;\n";
        InitAccumWithCurValue("0");
        k << "\n"
          << "for (ulong i = 1; i < count; i++) {\n"
          << "    key1 = " << keys1First[k.var<compute::uint_>("i")] << ";\n"
          << "    key2 = " << keys2First[k.var<compute::uint_>("i")] << ";\n"
          << "    "
          << "    if (" << predicateCall << ") {\n"
          << "        " << function(accumulatedResultPtr, currentValuePtr, k.var<compute::uchar_>("reduce_call_buffer")) << ";\n";
        CopyLastFunctionCallToAccum();
        k << "\n"
          << "    }\n "
          << "    else { \n"
          << keys1Result[k.var<compute::uint_>("size - 1")] << " = previous_key1;\n"
          << keys2Result[k.var<compute::uint_>("size - 1")] << " = previous_key2;\n"
          << "        size++;\n";
        InitAccumWithCurValue("i");
        k << "\n"
          << "    } \n"
          << "    previous_key1 = key1;\n"
          << "    previous_key2 = key2;\n"
          << "}\n"
          << keys1Result[k.var<compute::uint_>("size - 1")] << " = previous_key1;\n"
          << keys2Result[k.var<compute::uint_>("size - 1")] << " = previous_key2;\n"
          << "*result_size = size;";

        compute::kernel kernel = k.compile(context);

        compute::detail::scalar<compute::uint_> resultSize(context);
        kernel.set_arg(resultSizeArg, resultSize.get_buffer());
        kernel.set_arg(countArg, static_cast<compute::uint_>(count));

        queue.enqueue_task(kernel);

        return MakeOutputLastIterators(resultSize.read(queue));
    }

}// namespace spla

#endif//SPLA_SPLAREDUCEALIGNEDVALUESBYKEY_HPP
