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

#ifndef SPLA_SPLAGATHER_HPP
#define SPLA_SPLAGATHER_HPP

#include <boost/compute.hpp>
#include <boost/compute/iterator/zip_iterator.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace {

        template<class InputIterator, class MapIterator, class OutputIterator>
        class GatherKernel : public boost::compute::detail::meta_kernel {
        public:
            GatherKernel() : meta_kernel("gather_byteSize") {}

            void SetRange(MapIterator first,
                          MapIterator last,
                          InputIterator input,
                          OutputIterator result,
                          size_t elementsInSequence) {
                mCount = boost::compute::detail::iterator_range_size(first, last);

                *this << "const uint i = get_global_id(0);\n"
                      << "const uint index = " << first[expr<boost::compute::uint_>("i")] << ";\n"
                      << "const uint dst = i * " << elementsInSequence << ";\n"
                      << "const uint src = index * " << elementsInSequence << ";\n"
                      << "for (uint k = 0; k < " << elementsInSequence << "; k++) {\n"
                      << result[expr<boost::compute::uint_>("dst + k")] << "=" << input[expr<boost::compute::uint_>("src + k")] << ";\n"
                      << "}";
            }

            boost::compute::event Exec(boost::compute::command_queue &queue) {
                if (mCount == 0) {
                    return boost::compute::event();
                }

                return exec_1d(queue, 0, mCount);
            }

        private:
            size_t mCount = 0;
        };

    }// namespace

    /**
     * Custom gather function. Allows to collect input values using permutation.
     * Uses `elementsInSequence` to collect several elements in row in sequence, i.e.
     * result[i + k] = input[permutation[i] + k] for k in 0..elementsInSequence for i in map range
     * 
     * @tparam InputIterator Type of input source values iterator
     * @tparam MapIterator Type of map (permutation iterator)
     * @tparam OutputIterator Type of output (result) gathered values iterator
     *
     * @param first Begin of map range
     * @param last End of map range
     * @param input Values to map
     * @param result Where to store result
     * @param elementsInSequence How much values in sequence to copy
     * @param queue Execution queue
     */
    template<class InputIterator, class MapIterator, class OutputIterator>
    inline boost::compute::event Gather(MapIterator first,
                                        MapIterator last,
                                        InputIterator input,
                                        OutputIterator result,
                                        std::size_t elementsInSequence,
                                        boost::compute::command_queue &queue) {
        using namespace boost;

        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<MapIterator>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputIterator>::value);

        GatherKernel<InputIterator, MapIterator, OutputIterator> kernel;

        kernel.SetRange(first, last, input, result, elementsInSequence);
        return kernel.Exec(queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAGATHER_HPP
