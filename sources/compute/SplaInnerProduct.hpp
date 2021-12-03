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
#ifndef SPLA_SPLAINNERPRODUCT_HPP
#define SPLA_SPLAINNERPRODUCT_HPP

#include <boost/compute/algorithm/accumulate.hpp>

namespace spla {

    /**
     * @brief Calculates the inner product of two iterators, whose elements
     * are combined with function @p transformFunction and then accumulated
     * with function @p transformFunction.
     *
     * @note This is just a slightly changed function of @p boost::compute::inner_product.
     * The one and only difference is that intermediate vector has a type of
     * @p transformFunction resulting type, unlike in@p boost::compute::inner_product,
     * where intermediate vector has the same type as underlying type of @p InputIterator1.
     *
     * @tparam T Resulting type of @p transformFunction
     * @param first1 Iterator at the beginning of first collection
     * @param last1 Iterator at the ending of the first collection
     * @param first2 Iterator at the beginning of the second collection
     * @param init Initial value for accumulation of type @p T
     * @param accumulateFunction A binary function, which accepts accumulated value
     * and the next value of type @p T from the intermediate vector
     * @param transformFunction A binary function, which accepts element from
     * the first collection and an element from the second collections and returns
     * intermediate value of type @p T
     * @param queue OpenCL command queue
     * @return Accumulated value
     */
    template<class InputIterator1,
             class InputIterator2,
             class T,
             class BinaryAccumulateFunction,
             class BinaryTransformFunction>
    inline T InnerProduct(InputIterator1 first1,
                          InputIterator1 last1,
                          InputIterator2 first2,
                          T init,
                          BinaryAccumulateFunction accumulateFunction,
                          BinaryTransformFunction transformFunction,
                          boost::compute::command_queue &queue) {
        namespace compute = boost::compute;

        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator1>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator2>::value);

        size_t count = compute::detail::iterator_range_size(first1, last1);
        compute::vector<T> result(count, queue.get_context());
        compute::transform(first1,
                           last1,
                           first2,
                           result.begin(),
                           transformFunction,
                           queue);

        return ::boost::compute::accumulate(result.begin(),
                                            result.end(),
                                            init,
                                            accumulateFunction,
                                            queue);
    }

}// namespace spla

#endif//SPLA_SPLAINNERPRODUCT_HPP
