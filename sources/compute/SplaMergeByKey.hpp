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

#include <functional>

#include <boost/compute/container/vector.hpp>
#include <boost/compute/detail/meta_kernel.hpp>

namespace spla {

    namespace detail {
        class MergeByKeyPathKernel : public boost::compute::detail::meta_kernel {
        public:
            unsigned int tileSize;

            MergeByKeyPathKernel()
                : meta_kernel("merge_path"), tileSize(4) {}

            template<
                    typename OutputIterator1, typename OutputIterator2,
                    typename BinaryIndicesOperator>
            void SetRange(const std::size_t aCount,
                          const std::size_t bCount,
                          OutputIterator1 resultA,
                          OutputIterator2 resultB,
                          BinaryIndicesOperator isFirstGt) {
                namespace compute = boost::compute;
                using compute::uint_;

                mACount = aCount;
                mACountArg = add_arg<uint_>("a_count");

                mBCount = bCount;
                mBCountArg = add_arg<uint_>("b_count");

                *this << "uint i = get_global_id(0);\n"
                      << "uint target = (i+1)*" << tileSize << ";\n"
                      << "uint start = max(convert_int(0),convert_int(target)-convert_int(b_count));\n"
                      << "uint end = min(target,a_count);\n"
                      << "uint a_index, b_index;\n"
                      << "while(start<end)\n"
                      << "{\n"
                      << "   a_index = (start + end)/2;\n"
                      << "   b_index = target - a_index - 1;\n"
                      << "   if(!(";
                isFirstGt(*this, expr<uint_>("a_index"), expr<uint_>("b_index"));
                *this << "))\n"
                      << "       start = a_index + 1;\n"
                      << "   else end = a_index;\n"
                      << "}\n"
                      << resultA[expr<uint_>("i")] << " = start;\n"
                      << resultB[expr<uint_>("i")] << " = target - start;\n";
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                namespace compute = boost::compute;

                if ((mACount + mBCount) / tileSize == 0) {
                    return compute::event();
                }

                set_arg(mACountArg, compute::uint_(mACount));
                set_arg(mBCountArg, compute::uint_(mBCount));

                return exec_1d(queue, 0, (mACount + mBCount) / tileSize);
            }

        private:
            std::size_t mACount{};
            std::size_t mACountArg{};
            std::size_t mBCount{};
            std::size_t mBCountArg{};
        };

        class SerialMergeByKeyKernel : boost::compute::detail::meta_kernel {
        public:
            unsigned int tileSize;

            SerialMergeByKeyKernel()
                : meta_kernel("merge"),
                  tileSize(4) {}

            template<
                    typename InputIterator1,
                    typename InputIterator2,
                    typename Operator1,
                    typename Operator2,
                    typename Operator3>
            void SetRange(InputIterator1 tile_first1,
                          InputIterator1 tile_last1,
                          InputIterator2 tile_first2,
                          Operator1 writeIsFirstGt,
                          Operator2 writeAssignResultToFirst,
                          Operator3 writeAssignResultToSecond) {
                namespace compute = boost::compute;
                using compute::uint_;

                mCount = compute::detail::iterator_range_size(tile_first1, tile_last1) - 1;

                *this << "uint i = get_global_id(0);\n"
                      << "uint start1 = " << tile_first1[expr<uint_>("i")] << ";\n"
                      << "uint end1 = " << tile_first1[expr<uint_>("i+1")] << ";\n"
                      << "uint start2 = " << tile_first2[expr<uint_>("i")] << ";\n"
                      << "uint end2 = " << tile_first2[expr<uint_>("i+1")] << ";\n"
                      << "uint index = i*" << tileSize << ";\n"
                      << "while(start1<end1 && start2<end2)\n"
                      << "{\n"
                      << "   if(!(";
                writeIsFirstGt(*this, expr<uint_>("start1"), expr<uint_>("start2"));
                *this << "))\n"
                      << "   {\n";
                writeAssignResultToFirst(*this, expr<uint_>("index"), expr<uint_>("start1"));
                *this << ";\n"
                      << "       index++;\n"
                      << "       start1++;\n"
                      << "   }\n"
                      << "   else\n"
                      << "   {\n";
                writeAssignResultToSecond(*this, expr<uint_>("index"), expr<uint_>("start2"));
                *this << ";\n"
                      << "       index++;\n"
                      << "       start2++;\n"
                      << "   }\n"
                      << "}\n"
                      << "while(start1<end1)\n"
                      << "{\n";
                writeAssignResultToFirst(*this, expr<uint_>("index"), expr<uint_>("start1"));
                *this << ";\n"
                      << "   index++;\n"
                      << "   start1++;\n"
                      << "}\n"
                      << "while(start2<end2)\n"
                      << "{\n";
                writeAssignResultToSecond(*this, expr<uint_>("index"), expr<uint_>("start2"));
                *this << ";\n"
                      << "   index++;\n"
                      << "   start2++;\n"
                      << "}\n";
            }

            boost::compute::event exec(boost::compute::command_queue &queue) {
                if (mCount == 0) {
                    return boost::compute::event{};
                }

                return exec_1d(queue, 0, mCount);
            }

        private:
            std::size_t mCount{};
        };

        template<
                typename Operator1,
                typename Operator2,
                typename Operator3>
        std::ptrdiff_t MergeByKey(
                const std::size_t count1,
                const std::size_t count2,
                Operator1 compareFirstToSecond,
                Operator2 assignResultToFirst,
                Operator3 assignResultToSecond,
                boost::compute::command_queue &queue) {
            namespace compute = boost::compute;
            using compute::uint_;

            const std::size_t tileSize = 1024;

            compute::vector<uint_> tileA((count1 + count2 + tileSize - 1) / tileSize + 1, queue.get_context());
            compute::vector<uint_> tileB((count1 + count2 + tileSize - 1) / tileSize + 1, queue.get_context());

            // Tile the sets
            MergeByKeyPathKernel tilingKernel;
            tilingKernel.tileSize = static_cast<unsigned int>(tileSize);
            tilingKernel.SetRange(count1, count2, tileA.begin() + 1, tileB.begin() + 1, compareFirstToSecond);
            fill_n(tileA.begin(), 1, uint_(0), queue);
            fill_n(tileB.begin(), 1, uint_(0), queue);
            tilingKernel.exec(queue);

            fill_n(tileA.end() - 1, 1, static_cast<uint_>(count1), queue);
            fill_n(tileB.end() - 1, 1, static_cast<uint_>(count2), queue);

            // Merge
            SerialMergeByKeyKernel mergeKernel;
            mergeKernel.tileSize = static_cast<unsigned int>(tileSize);
            mergeKernel.SetRange(tileA.begin(), tileA.end(), tileB.begin(),
                                 compareFirstToSecond, assignResultToFirst, assignResultToSecond);

            mergeKernel.exec(queue);

            return static_cast<std::ptrdiff_t>(count1 + count2);
        }
    }// namespace detail

    /**
     * @brief Merges two sorted (by key) sequences of values by given keys.
     *
     * @param keysABegin Begin of the first key sequence
     * @param keysAEnd End of the first key sequence
     * @param valuesA Begin of the first value sequence
     * @param keysBBegin Begin of the second key sequence
     * @param keysBEnd End of the second key sequence
     * @param valuesB Begin of the second value sequence
     * @param keysResult Begin of the keys result
     * @param valuesResult Begin of the values result
     * @param queue OpenCL Command queue
     * @return Size of the merged sequence
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
    std::ptrdiff_t MergeByKey(
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
        using compute::uint_;

        auto isFirstGt = [&](compute::detail::meta_kernel &k,
                             const compute::detail::meta_kernel_variable<uint_> &iFst,
                             const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysABegin[iFst] << " > " << keysBBegin[iSnd];
        };

        auto assignResultToFirst = [&](compute::detail::meta_kernel &k,
                                       const compute::detail::meta_kernel_variable<uint_> &iRes,
                                       const compute::detail::meta_kernel_variable<uint_> &iFst) {
            k << keysResult[iRes] << " = " << keysABegin[iFst] << ";";
            k << valuesResult[iRes] << " = " << valuesA[iFst];
        };

        auto assignResultToSecond = [&](compute::detail::meta_kernel &k,
                                        const compute::detail::meta_kernel_variable<uint_> &iRes,
                                        const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysResult[iRes] << " = " << keysBBegin[iSnd] << ";";
            k << valuesResult[iRes] << " = " << valuesB[iSnd];
        };

        return detail::MergeByKey(
                std::distance(keysABegin, keysAEnd),
                std::distance(keysBBegin, keysBEnd),
                isFirstGt,
                assignResultToFirst,
                assignResultToSecond,
                queue);
    }

    /**
     * @brief Merges two sorted sequences.
     *
     * @param keysABegin Begin of the first sequence
     * @param keysAEnd End of the first sequence
     * @param keysBBegin Begin of the second sequence
     * @param keysBEnd End of the second sequence
     * @param keysResult Begin of the result
     * @param queue OpenCL Command queue
     * @return Size of the merged sequence
     */
    template<
            typename ItKeysABegin,
            typename ItKeysAEnd,
            typename ItKeysBBegin,
            typename ItKeysBEnd,
            typename ItKeysResult>
    std::ptrdiff_t MergeKeys(
            ItKeysABegin keysABegin,
            ItKeysAEnd keysAEnd,
            ItKeysBBegin keysBBegin,
            ItKeysBEnd keysBEnd,
            ItKeysResult keysResult,
            boost::compute::command_queue &queue) {
        namespace compute = boost::compute;
        using compute::uint_;

        auto isFirstGt = [&](compute::detail::meta_kernel &k,
                             const compute::detail::meta_kernel_variable<uint_> &iFst,
                             const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysABegin[iFst] << " > " << keysBBegin[iSnd];
        };

        auto assignResultToFirst = [&](compute::detail::meta_kernel &k,
                                       const compute::detail::meta_kernel_variable<uint_> &iRes,
                                       const compute::detail::meta_kernel_variable<uint_> &iFst) {
            k << keysResult[iRes] << " = " << keysABegin[iFst];
        };

        auto assignResultToSecond = [&](compute::detail::meta_kernel &k,
                                        const compute::detail::meta_kernel_variable<uint_> &iRes,
                                        const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysResult[iRes] << " = " << keysBBegin[iSnd];
        };

        return detail::MergeByKey(
                std::distance(keysABegin, keysAEnd),
                std::distance(keysBBegin, keysBEnd),
                isFirstGt,
                assignResultToFirst,
                assignResultToSecond,
                queue);
    }

    /**
     * @brief Merges two sorted (by pair key)
     *
     * @desc In this function, key pair sequence is actually
     * two sequences of keys. Hence, nth key of the first
     * sequence is
     * {@p keysFirstABegin[n], @p keysSecondABegin[n]},
     * and of the second second is
     * {@p keysFirstBBegin[n], @p keysSecondBBegin[n]}.
     *
     * @return Size of the merged sequence.
     *
     * @note Pair key - a key, which consists of two keys.
     * It is compared lexicographically: first it compares
     * by the first element, and then by the second.
     */
    template<
            typename ItInput1,
            typename ItInput2,
            typename ItInput3,
            typename ItInput4,
            typename ItInput5,
            typename ItInput6,
            typename ItOutput1,
            typename ItOutput2,
            typename ItOutput3>
    std::ptrdiff_t MergeByPairKey(
            ItInput1 keysFirstABegin, ItInput1 keysFirstAEnd, ItInput2 keysSecondABegin, ItInput3 valuesA,
            ItInput4 keysFirstBBegin, ItInput4 keysFirstBEnd, ItInput5 keysSecondBBegin, ItInput6 valuesB,
            ItOutput1 keysFirstOut, ItOutput2 keysSecondOut, ItOutput3 valuesOut,
            boost::compute::command_queue &queue) {
        namespace compute = boost::compute;
        using compute::uint_;

        auto isFirstGt = [&](compute::detail::meta_kernel &k,
                             const compute::detail::meta_kernel_variable<uint_> &iFst,
                             const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysFirstABegin[iFst] << " > " << keysFirstBBegin[iSnd] << " || "
              << "("
              << keysFirstABegin[iFst] << " <= " << keysFirstBBegin[iSnd] << " && "
              << keysSecondABegin[iFst] << " > " << keysSecondBBegin[iSnd]
              << ")";
        };

        auto assignResultToFirst = [&](compute::detail::meta_kernel &k,
                                       const compute::detail::meta_kernel_variable<uint_> &iRes,
                                       const compute::detail::meta_kernel_variable<uint_> &iFst) {
            k << keysFirstOut[iRes] << " = " << keysFirstABegin[iFst] << ";";
            k << keysSecondOut[iRes] << " = " << keysSecondABegin[iFst] << ";";
            k << valuesOut[iRes] << " = " << valuesA[iFst];
        };

        auto assignResultToSecond = [&](compute::detail::meta_kernel &k,
                                        const compute::detail::meta_kernel_variable<uint_> &iRes,
                                        const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysFirstOut[iRes] << " = " << keysFirstBBegin[iSnd] << ";";
            k << keysSecondOut[iRes] << " = " << keysSecondBBegin[iSnd] << ";";
            k << valuesOut[iRes] << " = " << valuesB[iSnd];
        };

        return detail::MergeByKey(
                std::distance(keysFirstABegin, keysFirstAEnd),
                std::distance(keysFirstBBegin, keysFirstBEnd),
                isFirstGt,
                assignResultToFirst,
                assignResultToSecond,
                queue);
    }


    /**
     * @brief Merges two sorted (by pair key)
     *
     * @desc In this function, key pair sequence is actually
     * two sequences of keys. Hence, nth key of the first
     * sequence is
     * {@p keysFirstABegin[n], @p keysSecondABegin[n]},
     * and of the second second is
     * {@p keysFirstBBegin[n], @p keysSecondBBegin[n]}.
     *
     * @return Size of the merged sequence.
     *
     * @note Pair key - a key, which consists of two keys.
     * It is compared lexicographically: first it compares
     * by the first element, and then by the second.
     */
    template<
            typename ItInput1,
            typename ItInput2,
            typename ItInput4,
            typename ItInput5,
            typename ItOutput1,
            typename ItOutput2>
    std::ptrdiff_t MergePairKeys(
            ItInput1 keysFirstABegin, ItInput1 keysFirstAEnd, ItInput2 keysSecondABegin,
            ItInput4 keysFirstBBegin, ItInput4 keysFirstBEnd, ItInput5 keysSecondBBegin,
            ItOutput1 keysFirstOut, ItOutput2 keysSecondOut,
            boost::compute::command_queue &queue) {
        namespace compute = boost::compute;
        using compute::uint_;

        auto isFirstGt = [&](compute::detail::meta_kernel &k,
                             const compute::detail::meta_kernel_variable<uint_> &iFst,
                             const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysFirstABegin[iFst] << " > " << keysFirstBBegin[iSnd] << " || "
              << "("
              << "!(" << keysFirstABegin[iFst] << " > " << keysFirstBBegin[iSnd] << ") && "
              << keysSecondABegin[iFst] << " > " << keysSecondBBegin[iSnd]
              << ")";
        };

        auto assignResultToFirst = [&](compute::detail::meta_kernel &k,
                                       const compute::detail::meta_kernel_variable<uint_> &iRes,
                                       const compute::detail::meta_kernel_variable<uint_> &iFst) {
            k << keysFirstOut[iRes] << " = " << keysFirstABegin[iFst] << ";";
            k << keysSecondOut[iRes] << " = " << keysSecondABegin[iFst];
        };

        auto assignResultToSecond = [&](compute::detail::meta_kernel &k,
                                        const compute::detail::meta_kernel_variable<uint_> &iRes,
                                        const compute::detail::meta_kernel_variable<uint_> &iSnd) {
            k << keysFirstOut[iRes] << " = " << keysFirstBBegin[iSnd] << ";";
            k << keysSecondOut[iRes] << " = " << keysSecondBBegin[iSnd];
        };

        return detail::MergeByKey(
                std::distance(keysFirstABegin, keysFirstAEnd),
                std::distance(keysFirstBBegin, keysFirstBEnd),
                isFirstGt,
                assignResultToFirst,
                assignResultToSecond,
                queue);
    }

}// namespace spla

#endif//SPLA_SPLAMERGEBYKEY_HPP
