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
#include <boost/compute/detail/iterator_range_size.hpp>
#include <boost/compute/iterator/zip_iterator.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace detail {

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

                m_a_count = boost::compute::detail::iterator_range_size(first1, last1);
                m_a_count_arg = add_arg<uint_>("a_count");

                m_b_count = boost::compute::detail::iterator_range_size(first2, last2);
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
            std::size_t m_a_count;
            std::size_t m_a_count_arg;
            std::size_t m_b_count;
            std::size_t m_b_count_arg;
        };

        /** Serial pre-process intersection kernel to count actual results count (but do not intersect) */
        class SerialIntersectionCountKernel : boost::compute::detail::meta_kernel {
        public:
            SerialIntersectionCountKernel() : meta_kernel("__spla_serial_intersection_count") {
            }

            template<class InputIterator1,
                     class InputIterator2,
                     class InputIterator3,
                     class InputIterator4,
                     class OutputIterator,
                     class Compare,
                     class Equals>
            void set_range(InputIterator1 maskFirst,
                           InputIterator2 keyFirsts,
                           InputIterator3 tile_first1,
                           InputIterator3 tile_last1,
                           InputIterator4 tile_first2,
                           OutputIterator counts,
                           Compare compare,
                           Equals equals,
                           bool complement) {
                using uint_ = boost::compute::uint_;
                m_count = boost::compute::detail::iterator_range_size(tile_first1, tile_last1) - 1;

                *this << "uint i = get_global_id(0);\n"
                      << "uint start1 = " << tile_first1[expr<uint_>("i")] << ";\n"
                      << "uint end1 = " << tile_first1[expr<uint_>("i+1")] << ";\n"
                      << "uint start2 = " << tile_first2[expr<uint_>("i")] << ";\n"
                      << "uint end2 = " << tile_first2[expr<uint_>("i+1")] << ";\n"
                      << "uint count = 0;\n"
                      << "while(start1<end1 && start2<end2)\n"
                      << "{\n";

                if (complement) {
                    *this << "   if(" << equals(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "   {\n"
                          << "       start1++; start2++;\n"
                          << "   }\n"
                          << "   else if(" << compare(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "       start1++;\n"
                          << "   else\n"
                          << "   {\n"
                          << "       count++; start2++;\n"
                          << "   }\n";
                } else {
                    *this << "   if(" << equals(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "   {\n"
                          << "       count++;\n"
                          << "       start1++; start2++;\n"
                          << "   }\n"
                          << "   else if(" << compare(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "       start1++;\n"
                          << "   else start2++;\n";
                }

                *this << "}\n";

                if (complement)
                    *this << counts[expr<uint_>("i")] << " = count + (end2 - start2);\n";
                else
                    *this << counts[expr<uint_>("i")] << " = count;\n";
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                if (m_count == 0) {
                    return boost::compute::event();
                }

                return exec_1d(queue, 0, m_count);
            }

        private:
            std::size_t m_count = 0;
        };

        class SerialIntersectionKernel : boost::compute::detail::meta_kernel {
        public:
            SerialIntersectionKernel() : meta_kernel("__spla_serial_intersection") {
            }

            template<class InputIterator1,
                     class InputIterator2,
                     class InputIterator3,
                     class InputIterator4,
                     class InputIterator5,
                     class Compare,
                     class Equals,
                     class AssignResult>
            void set_range(InputIterator1 maskFirst,
                           InputIterator2 keyFirsts,
                           InputIterator3 tile_first1,
                           InputIterator3 tile_last1,
                           InputIterator4 tile_first2,
                           InputIterator5 counts,
                           Compare compare,
                           Equals equals,
                           AssignResult assignResult,
                           bool complement) {
                using uint_ = boost::compute::uint_;
                m_count = boost::compute::detail::iterator_range_size(tile_first1, tile_last1) - 1;

                *this << "uint i = get_global_id(0);\n"
                      << "uint start1 = " << tile_first1[expr<uint_>("i")] << ";\n"
                      << "uint end1 = " << tile_first1[expr<uint_>("i+1")] << ";\n"
                      << "uint start2 = " << tile_first2[expr<uint_>("i")] << ";\n"
                      << "uint end2 = " << tile_first2[expr<uint_>("i+1")] << ";\n"
                      << "uint count = " << counts[expr<uint_>("i")] << ";\n"
                      << "while(start1<end1 && start2<end2)\n"
                      << "{\n";

                if (complement) {
                    *this << "   if(" << equals(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "   {\n"
                          << "       start1++; start2++;\n"
                          << "   }\n"
                          << "   else if(" << compare(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "       start1++;\n"
                          << "   else\n"
                          << "   {\n";
                    assignResult(*this, expr<uint_>("count"), expr<uint_>("start2"));
                    *this << "       count++; start2++;\n"
                          << "   }\n";
                } else {
                    *this << "   if(" << equals(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "   {\n";
                    assignResult(*this, expr<uint_>("count"), expr<uint_>("start2"));
                    *this << "       count++;\n"
                          << "       start1++; start2++;\n"
                          << "   }\n"
                          << "   else if(" << compare(maskFirst[expr<uint_>("start1")], keyFirsts[expr<uint_>("start2")]) << ")\n"
                          << "       start1++;\n"
                          << "   else start2++;\n";
                }

                *this << "}\n";

                if (complement) {
                    *this << "while (start2 < end2)\n"
                          << "{\n";
                    assignResult(*this, expr<uint_>("count"), expr<uint_>("start2"));
                    *this << "    count++; start2++;"
                          << "}\n";
                }
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                if (m_count == 0) {
                    return boost::compute::event();
                }

                return exec_1d(queue, 0, m_count);
            }

        private:
            std::size_t m_count = 0;
        };

        template<
                typename InputMask,
                typename InputKey,
                typename Compare,
                typename Equals,
                typename ResizeResult,
                typename AssignResult>
        std::size_t MaskByKey(InputMask maskFirst,
                              InputKey keyFirst,
                              Compare compare,
                              Equals equals,
                              ResizeResult resizeResult,
                              AssignResult assignResult,
                              std::size_t maskCount,
                              std::size_t keyCount,
                              bool complement,
                              boost::compute::command_queue &queue) {
            using namespace boost;
            typedef typename std::iterator_traits<InputMask>::value_type key_type;

            std::size_t tileSize = 1024;

            compute::vector<compute::uint_> tileA((maskCount + keyCount + tileSize - 1) / tileSize + 1, queue.get_context());
            compute::vector<compute::uint_> tileB((maskCount + keyCount + tileSize - 1) / tileSize + 1, queue.get_context());

            // Tile the sets
            detail::BalancedPathKernel balancedPathKernel;
            balancedPathKernel.tile_size = tileSize;
            balancedPathKernel.set_range(maskFirst, maskFirst + maskCount,
                                         keyFirst, keyFirst + keyCount,
                                         tileA.begin() + 1, tileB.begin() + 1,
                                         compare);
            fill_n(tileA.begin(), 1, 0, queue);
            fill_n(tileB.begin(), 1, 0, queue);
            balancedPathKernel.exec(queue);

            fill_n(tileA.end() - 1, 1, maskCount, queue);
            fill_n(tileB.end() - 1, 1, keyCount, queue);

            compute::vector<compute::uint_> counts((maskCount + keyCount + tileSize - 1) / tileSize + 1, queue.get_context());
            compute::fill_n(counts.end() - 1, 1, 0, queue);

            // Find result intersections count and offsets to write result of each tile
            detail::SerialIntersectionCountKernel intersectionCountKernel;
            intersectionCountKernel.set_range(maskFirst, keyFirst,
                                              tileA.begin(), tileA.end(),
                                              tileB.begin(),
                                              counts.begin(),
                                              compare, equals, complement);
            intersectionCountKernel.exec(queue);

            // Compute actual counts offsets
            exclusive_scan(counts.begin(), counts.end(), counts.begin(), queue);

            // Get result count and resize buffers
            std::size_t resultCount = (counts.end() - 1).read(queue);
            resizeResult(resultCount);

            // Find result intersections
            detail::SerialIntersectionKernel intersectionKernel;
            intersectionKernel.set_range(maskFirst, keyFirst, tileA.begin(), tileA.end(),
                                         tileB.begin(),
                                         counts.begin(),
                                         compare, equals, assignResult, complement);
            intersectionKernel.exec(queue);

            return resultCount;
        }

    }// namespace detail

    /**
     * @brief Mask intersection algorithm.
     *
     * Finds the intersection of the sorted mask range with the sorted
     * keys range and stores it in range starting at resultKeys.
     *
     * @note Automatically resizes result containers to result count size.
     * @note Use complement flag to apply direct or inverse (complement) mask
     *
     * @param mask Mask elements
     * @param keys Keys elements
     * @param resultKeys Result keys elements
     * @param complement Pass true to apply !mask (complementary mask)
     * @param queue Command queue to perform operations on
     *
     * @return Count of values in intersected region
     */
    inline std::size_t MaskKeys(const boost::compute::vector<unsigned int> &mask,
                                const boost::compute::vector<unsigned int> &keys,
                                boost::compute::vector<unsigned int> &resultKeys,
                                bool complement,
                                boost::compute::command_queue &queue) {
        using namespace boost;
        using MetaKernel = boost::compute::detail::meta_kernel;
        using MetaIdx = boost::compute::detail::meta_kernel_variable<boost::compute::uint_>;

        auto resizeResult = [&](std::size_t size) {
            resultKeys.resize(size);
        };

        auto assignResult = [&](MetaKernel &kernel, const MetaIdx &dst, const MetaIdx &src) {
            kernel << resultKeys.begin()[dst] << " = " << keys.begin()[src] << ";\n";
        };

        if (mask.empty() || keys.empty()) {
            resizeResult(0);
            return 0;
        }

        using compute::lambda::_1;
        using compute::lambda::_2;

        return detail::MaskByKey(mask.begin(),
                                 keys.begin(),
                                 _1 < _2,
                                 _1 == _2,
                                 resizeResult,
                                 assignResult,
                                 mask.size(),
                                 keys.size(),
                                 complement,
                                 queue);
    }

    /**
     * @brief Mask intersection algorithm.
     *
     * Finds the intersection of the sorted mask range with the sorted
     * keys range and stores it in range starting at resultKeys.
     *
     * @note Manages associated values with keys.
     * @note Automatically resizes result containers to result count size.
     * @note Use complement flag to apply direct or inverse (complement) mask
     *
     * @param mask Mask elements
     * @param keys Keys elements
     * @param values Associated with keys values
     * @param resultKeys Result keys elements
     * @param resultValues Result values associated with result keys.
     * @param complement Pass true to apply !mask (complementary mask)
     * @param queue Command queue to perform operations on
     *
     * @return Count of values in intersected region
     */
    inline std::size_t MaskByKeys(const boost::compute::vector<unsigned int> &mask,
                                  const boost::compute::vector<unsigned int> &keys,
                                  const boost::compute::vector<unsigned int> &values,
                                  boost::compute::vector<unsigned int> &resultKeys,
                                  boost::compute::vector<unsigned int> &resultValues,
                                  bool complement,
                                  boost::compute::command_queue &queue) {
        using namespace boost;
        using MetaKernel = boost::compute::detail::meta_kernel;
        using MetaIdx = boost::compute::detail::meta_kernel_variable<boost::compute::uint_>;

        assert(keys.size() == values.size());

        auto resizeResult = [&](std::size_t size) {
            resultKeys.resize(size);
            resultValues.resize(size);
        };

        auto assignResult = [&](MetaKernel &kernel, const MetaIdx &dst, const MetaIdx &src) {
            kernel << resultKeys.begin()[dst] << " = " << keys.begin()[src] << ";\n"
                   << resultValues.begin()[dst] << " = " << values.begin()[src] << ";\n";
        };

        if (mask.empty() || keys.empty()) {
            resizeResult(0);
            return 0;
        }

        using compute::lambda::_1;
        using compute::lambda::_2;

        return detail::MaskByKey(mask.begin(),
                                 keys.begin(),
                                 _1 < _2,
                                 _1 == _2,
                                 resizeResult,
                                 assignResult,
                                 mask.size(),
                                 keys.size(),
                                 complement,
                                 queue);
    }

    /**
     * @brief Mask intersection algorithm.
     *
     * Finds the intersection of the sorted pair mask range with the sorted
     * pair keys range and stores it in range starting at resultKeys.
     *
     * @note Interprets keys as pairs, where first and second elements stored in separate arrays.
     * @note Automatically resizes result containers to result count size.
     * @note Use complement flag to apply direct or inverse (complement) mask
     *
     * @param mask1 Mask first elements
     * @param mask2 Mask second elements
     * @param keys1 Keys first elements
     * @param keys2 Keys second elements
     * @param resultKeys1 Result keys first elements
     * @param resultKeys2 Result keys second elements
     * @param complement Pass true to apply !mask (complementary mask)
     * @param queue Command queue to perform operations on
     *
     * @return Count of values in intersected region
     */
    inline std::size_t MaskPairKeys(const boost::compute::vector<unsigned int> &mask1,
                                    const boost::compute::vector<unsigned int> &mask2,
                                    const boost::compute::vector<unsigned int> &keys1,
                                    const boost::compute::vector<unsigned int> &keys2,
                                    boost::compute::vector<unsigned int> &resultKeys1,
                                    boost::compute::vector<unsigned int> &resultKeys2,
                                    bool complement,
                                    boost::compute::command_queue &queue) {
        using namespace boost;
        using MetaKernel = boost::compute::detail::meta_kernel;
        using MetaIdx = boost::compute::detail::meta_kernel_variable<boost::compute::uint_>;

        assert(mask1.size() == mask2.size());
        assert(keys1.size() == keys2.size());

        auto resizeResult = [&](std::size_t size) {
            resultKeys1.resize(size);
            resultKeys2.resize(size);
        };

        auto assignResult = [&](MetaKernel &kernel, const MetaIdx &dst, const MetaIdx &src) {
            kernel << resultKeys1.begin()[dst] << " = " << keys1.begin()[src] << ";\n"
                   << resultKeys2.begin()[dst] << " = " << keys2.begin()[src] << ";\n";
        };

        if (mask1.empty() || keys1.empty()) {
            resizeResult(0);
            return 0;
        }

        using compute::lambda::_1;
        using compute::lambda::_2;
        using compute::lambda::get;

        return detail::MaskByKey(compute::make_zip_iterator(boost::make_tuple(mask1.begin(), mask2.begin())),
                                 compute::make_zip_iterator(boost::make_tuple(keys1.begin(), keys2.begin())),
                                 get<0>(_1) < get<0>(_2) || (get<0>(_1) == get<0>(_2) && get<1>(_1) < get<1>(_2)),
                                 get<0>(_1) == get<0>(_2) && get<1>(_1) == get<1>(_2),
                                 resizeResult,
                                 assignResult,
                                 mask1.size(),
                                 keys1.size(),
                                 complement,
                                 queue);
    }

    /**
     * @brief Mask intersection algorithm.
     *
     * Finds the intersection of the sorted pair mask range with the sorted
     * pair keys range and stores it in range starting at resultKeys.
     *
     * @note Manages associated values with keys.
     * @note Interprets keys as pairs, where first and second elements stored in separate arrays.
     * @note Automatically resizes result containers to result count size.
     * @note Use complement flag to apply direct or inverse (complement) mask
     *
     * @param mask1 Mask first elements
     * @param mask2 Mask second elements
     * @param keys1 Keys first elements
     * @param keys2 Keys second elements
     * @param values Associated with keys values
     * @param resultKeys1 Result keys first elements
     * @param resultKeys2 Result keys second elements
     * @param resultValues Result values associated with result keys.
     * @param complement Pass true to apply !mask (complementary mask)
     * @param queue Command queue to perform operations on
     *
     * @return Count of values in intersected region
     */
    inline std::size_t MaskByPairKeys(const boost::compute::vector<unsigned int> &mask1,
                                      const boost::compute::vector<unsigned int> &mask2,
                                      const boost::compute::vector<unsigned int> &keys1,
                                      const boost::compute::vector<unsigned int> &keys2,
                                      const boost::compute::vector<unsigned int> &values,
                                      boost::compute::vector<unsigned int> &resultKeys1,
                                      boost::compute::vector<unsigned int> &resultKeys2,
                                      boost::compute::vector<unsigned int> &resultValues,
                                      bool complement,
                                      boost::compute::command_queue &queue) {
        using namespace boost;
        using MetaKernel = boost::compute::detail::meta_kernel;
        using MetaIdx = boost::compute::detail::meta_kernel_variable<boost::compute::uint_>;

        assert(mask1.size() == mask2.size());
        assert(keys1.size() == keys2.size());
        assert(keys1.size() == values.size());

        auto resizeResult = [&](std::size_t size) {
            resultKeys1.resize(size);
            resultKeys2.resize(size);
            resultValues.resize(size);
        };

        auto assignResult = [&](MetaKernel &kernel, const MetaIdx &dst, const MetaIdx &src) {
            kernel << resultKeys1.begin()[dst] << " = " << keys1.begin()[src] << ";\n"
                   << resultKeys2.begin()[dst] << " = " << keys2.begin()[src] << ";\n"
                   << resultValues.begin()[dst] << " = " << values.begin()[src] << ";\n";
        };

        if (mask1.empty() || keys1.empty()) {
            resizeResult(0);
            return 0;
        }

        using compute::lambda::_1;
        using compute::lambda::_2;
        using compute::lambda::get;

        return detail::MaskByKey(compute::make_zip_iterator(boost::make_tuple(mask1.begin(), mask2.begin())),
                                 compute::make_zip_iterator(boost::make_tuple(keys1.begin(), keys2.begin())),
                                 get<0>(_1) < get<0>(_2) || (get<0>(_1) == get<0>(_2) && get<1>(_1) < get<1>(_2)),
                                 get<0>(_1) == get<0>(_2) && get<1>(_1) == get<1>(_2),
                                 resizeResult,
                                 assignResult,
                                 mask1.size(),
                                 keys1.size(),
                                 complement,
                                 queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAMASKBYKEY_HPP