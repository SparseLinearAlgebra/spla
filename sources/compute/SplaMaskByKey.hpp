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

#ifndef SPLA_SPLAMASKBYKEY_HPP
#define SPLA_SPLAMASKBYKEY_HPP

#include <boost/compute.hpp>
#include <boost/compute/iterator/zip_iterator.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace {

        class BalancedPathKernel : public boost::compute::detail::meta_kernel {
        public:
            unsigned int tile_size;

            BalancedPathKernel() : meta_kernel("__spla_balanced_path") {
                tile_size = 4;
            }

            template<class InputIterator1, class InputIterator2,
                     class OutputIterator1, class OutputIterator2,
                     class Compare>
            void set_range(InputIterator1 first1,
                           InputIterator1 last1,
                           InputIterator2 first2,
                           InputIterator2 last2,
                           OutputIterator1 result_a,
                           OutputIterator2 result_b,
                           Compare comp) {
                using namespace boost::compute;

                typedef typename std::iterator_traits<InputIterator1>::value_type value_type;

                m_a_count = detail::iterator_range_size(first1, last1);
                m_a_count_arg = add_arg<uint_>("a_count");

                m_b_count = detail::iterator_range_size(first2, last2);
                m_b_count_arg = add_arg<uint_>("b_count");

                *this << "uint i = get_global_id(0);\n"
                      << "uint target = (i+1)*" << tile_size << ";\n"
                      << "uint start = max(convert_int(0),convert_int(target)-convert_int(b_count));\n"
                      << "uint end = min(target,a_count);\n"
                      << "uint a_index, b_index;\n"
                      << "while(start<end)\n"
                      << "{\n"
                      << "   a_index = (start + end)/2;\n"
                      << "   b_index = target - a_index - 1;\n"
                      << "   if(!(" << comp(first2[expr<uint_>("b_index")], first1[expr<uint_>("a_index")]) << "))\n"
                      << "       start = a_index + 1;\n"
                      << "   else end = a_index;\n"
                      << "}\n"
                      << "a_index = start;\n"
                      << "b_index = target - start;\n"
                      << "if(b_index < b_count)\n"
                      << "{\n"
                      << "   " << decl<const value_type>("x") << " = " << first2[expr<uint_>("b_index")] << ";\n"
                      << "   uint a_start = 0, a_end = a_index, a_mid;\n"
                      << "   uint b_start = 0, b_end = b_index, b_mid;\n"
                      << "   while(a_start<a_end)\n"
                      << "   {\n"
                      << "       a_mid = (a_start + a_end)/2;\n"
                      << "       if(" << comp(first1[expr<uint_>("a_mid")], expr<value_type>("x")) << ")\n"
                      << "           a_start = a_mid+1;\n"
                      << "       else a_end = a_mid;\n"
                      << "   }\n"
                      << "   while(b_start<b_end)\n"
                      << "   {\n"
                      << "       b_mid = (b_start + b_end)/2;\n"
                      << "       if(" << comp(first2[expr<uint_>("b_mid")], expr<value_type>("x")) << ")\n"
                      << "           b_start = b_mid+1;\n"
                      << "       else b_end = b_mid;\n"
                      << "   }\n"
                      << "   uint a_run = a_index - a_start;\n"
                      << "   uint b_run = b_index - b_start;\n"
                      << "   uint x_count = a_run + b_run;\n"
                      << "   uint b_advance = max(x_count / 2, x_count - a_run);\n"
                      << "   b_end = min(b_count, b_start + b_advance + 1);\n"
                      << "   uint temp_start = b_index, temp_end = b_end, temp_mid;"
                      << "   while(temp_start < temp_end)\n"
                      << "   {\n"
                      << "       temp_mid = (temp_start + temp_end + 1)/2;\n"
                      << "       if(" << comp(expr<value_type>("x"), first2[expr<uint_>("temp_mid")]) << ")\n"
                      << "           temp_end = temp_mid-1;\n"
                      << "       else temp_start = temp_mid;\n"
                      << "   }\n"
                      << "   b_run = temp_start - b_start + 1;\n"
                      << "   b_advance = min(b_advance, b_run);\n"
                      << "   uint a_advance = x_count - b_advance;\n"
                      << "   uint star = convert_uint((a_advance == b_advance + 1) "
                      << "&& (b_advance < b_run));\n"
                      << "   a_index = a_start + a_advance;\n"
                      << "   b_index = target - a_index + star;\n"
                      << "}\n"
                      << result_a[expr<uint_>("i")] << " = a_index;\n"
                      << result_b[expr<uint_>("i")] << " = b_index;\n";
            }

            template<class InputIterator1, class InputIterator2,
                     class OutputIterator1, class OutputIterator2>
            void set_range(InputIterator1 first1,
                           InputIterator1 last1,
                           InputIterator2 first2,
                           InputIterator2 last2,
                           OutputIterator1 result_a,
                           OutputIterator2 result_b) {
                typedef typename std::iterator_traits<InputIterator1>::value_type value_type;
                ::boost::compute::less<value_type> less_than;
                set_range(first1, last1, first2, last2, result_a, result_b, less_than);
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                using namespace boost::compute;

                if ((m_a_count + m_b_count) / tile_size == 0) {
                    return boost::compute::event();
                }

                set_arg(m_a_count_arg, uint_(m_a_count));
                set_arg(m_b_count_arg, uint_(m_b_count));

                return exec_1d(queue, 0, (m_a_count + m_b_count) / tile_size);
            }

        private:
            size_t m_a_count;
            size_t m_a_count_arg;
            size_t m_b_count;
            size_t m_b_count_arg;
        };

        class SerialIntersectionKernel : boost::compute::detail::meta_kernel {
        public:
            unsigned int tile_size;

            SerialIntersectionKernel() : meta_kernel("__spla_serial_intersection") {
                tile_size = 4;
            }

            template<class InputIterator1,
                     class InputIterator2,
                     class InputIterator3,
                     class InputIterator4,
                     class InputIterator5,
                     class OutputIterator1,
                     class OutputIterator2,
                     class OutputIterator3,
                     class Compare,
                     class Equals>
            void set_range(InputIterator1 maskFirst,
                           InputIterator2 keyFirsts,
                           InputIterator3 tile_first1,
                           InputIterator3 tile_last1,
                           InputIterator4 tile_first2,
                           InputIterator5 values,
                           OutputIterator1 resultKeys,
                           OutputIterator2 resultValues,
                           OutputIterator3 counts,
                           Compare compare,
                           Equals equals) {
                using uint_ = boost::compute::uint_;
                m_count = boost::compute::detail::iterator_range_size(tile_first1, tile_last1) - 1;

                *this << "uint i = get_global_id(0);\n"
                      << "uint start1 = " << tile_first1[expr<uint_>("i")] << ";\n"
                      << "uint end1 = " << tile_first1[expr<uint_>("i+1")] << ";\n"
                      << "uint start2 = " << tile_first2[expr<uint_>("i")] << ";\n"
                      << "uint end2 = " << tile_first2[expr<uint_>("i+1")] << ";\n"
                      << "uint index = i*" << tile_size << ";\n"
                      << "uint count = 0;\n"
                      << "while(start1<end1 && start2<end2)\n"
                      << "{\n"
                      << "   if(" << equals(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                      << "   {\n"
                      << "       " << resultKeys[expr<uint_>("index")] << " = " << keyFirsts[expr<uint_>("start2")] << ";\n"
                      << "       " << resultValues[expr<uint_>("index")] << " = " << values[expr<uint_>("start2")] << ";\n"
                      << "       index++; count++;\n"
                      << "       start1++; start2++;\n"
                      << "   }\n"
                      << "   else if(" << compare(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                      << "       start1++;\n"
                      << "   else start2++;\n"
                      << "}\n"
                      << counts[expr<uint_>("i")] << " = count;\n";
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                if (m_count == 0) {
                    return boost::compute::event();
                }

                return exec_1d(queue, 0, m_count);
            }

        private:
            size_t m_count = 0;
        };
    }// namespace

    /**
     * @brief Mask intersection algorithm
     *
     * Finds the intersection of the sorted mask range [maskFirst, maskLast) with the sorted
     * keys range [keyFirsts, keyLast) and stores it in range starting at resultKeys with values associated to key range.
     *
     * @code
     *      vector<uint> mask = { 0, 3, 5, 10 };
     *      vector<uint> keys = { 0, 1, 2, 3, 7, 10 };
     *      vector<uint> values = { 10, 1, 2, 53, 4, 775 };
     *
     *      MaskByKey(mask.begin(), mask.end(), key.begin(), key.end(), values.begin(),
     *                keys.begin(), values.begin(), queue);
     *
     *      // keys = { 0, 3, 10 }
     *      // values = {10, 2, 53, 775 }
     * @endcode
     *
     * @tparam InputIterator1 Type of mask iterator
     * @tparam InputIterator2 Type of keys iterator
     * @tparam InputIterator3 Type of values iterator
     * @tparam OutputIterator1 Type of result keys iterator
     * @tparam OutputIterator2 Type of result values iterator
     * @tparam Compare Type of mask/keys '<' functor
     * @tparam Equals Type of mask/keys '==' functor
     *
     * @param maskFirst Begin of the mask
     * @param maskLast End of the mask
     * @param keyFirsts Begin of keys
     * @param keyLast End of keys
     * @param valueFirst Begin of values (range the same as for keys)
     * @param resultKeys Keys result begin
     * @param resultValues Values result begin
     * @param compare Mask/keys compare function
     * @param equals Mask/keys compare function
     * @param queue Queue to execute
     *
     * @return Count of values in intersected region
     */
    template<
            class InputIterator1,
            class InputIterator2,
            class InputIterator3,
            class OutputIterator1,
            class OutputIterator2,
            class Compare,
            class Equals>
    inline std::size_t MaskByKey(InputIterator1 maskFirst,
                                 InputIterator1 maskLast,
                                 InputIterator2 keyFirsts,
                                 InputIterator2 keyLast,
                                 InputIterator3 valueFirst,
                                 OutputIterator1 resultKeys,
                                 OutputIterator2 resultValues,
                                 Compare compare,
                                 Equals equals,
                                 boost::compute::command_queue &queue) {
        using namespace boost;

        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator1>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator2>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<InputIterator3>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputIterator1>::value);
        BOOST_STATIC_ASSERT(compute::is_device_iterator<OutputIterator2>::value);

        typedef typename std::iterator_traits<InputIterator1>::value_type key_type;
        typedef typename std::iterator_traits<InputIterator3>::value_type value_type;

        int tile_size = 1024;

        int count1 = compute::detail::iterator_range_size(maskFirst, maskLast);
        int count2 = compute::detail::iterator_range_size(keyFirsts, keyLast);

        compute::vector<compute::uint_> tile_a((count1 + count2 + tile_size - 1) / tile_size + 1, queue.get_context());
        compute::vector<compute::uint_> tile_b((count1 + count2 + tile_size - 1) / tile_size + 1, queue.get_context());

        // Tile the sets
        BalancedPathKernel tiling_kernel;
        tiling_kernel.tile_size = tile_size;
        tiling_kernel.set_range(maskFirst, maskLast, keyFirsts, keyLast,
                                tile_a.begin() + 1, tile_b.begin() + 1, compare);
        fill_n(tile_a.begin(), 1, 0, queue);
        fill_n(tile_b.begin(), 1, 0, queue);
        tiling_kernel.exec(queue);

        fill_n(tile_a.end() - 1, 1, count1, queue);
        fill_n(tile_b.end() - 1, 1, count2, queue);

        compute::vector<key_type> temp_resultKeys(count1 + count2, queue.get_context());
        compute::vector<value_type> temp_resultValues(count1 + count2, queue.get_context());
        compute::vector<compute::uint_> counts((count1 + count2 + tile_size - 1) / tile_size + 1, queue.get_context());
        compute::fill_n(counts.end() - 1, 1, 0, queue);

        // Find individual intersections
        SerialIntersectionKernel intersection_kernel;
        intersection_kernel.tile_size = tile_size;
        intersection_kernel.set_range(maskFirst, keyFirsts, tile_a.begin(), tile_a.end(),
                                      tile_b.begin(), valueFirst,
                                      temp_resultKeys.begin(),
                                      temp_resultValues.begin(),
                                      counts.begin(),
                                      compare, equals);

        intersection_kernel.exec(queue);

        exclusive_scan(counts.begin(), counts.end(), counts.begin(), queue);

        // Compact the results keys
        compute::detail::compact_kernel compactKey_kernel;
        compactKey_kernel.tile_size = tile_size;
        compactKey_kernel.set_range(temp_resultKeys.begin(), counts.begin(), counts.end(), resultKeys);
        compactKey_kernel.exec(queue);

        // Compact the results values
        compute::detail::compact_kernel compactValues_kernel;
        compactValues_kernel.tile_size = tile_size;
        compactValues_kernel.set_range(temp_resultValues.begin(), counts.begin(), counts.end(), resultValues);
        compactValues_kernel.exec(queue);

        return (counts.end() - 1).read(queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAMASKBYKEY_HPP
