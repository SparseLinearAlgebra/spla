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

#ifndef SPLA_SPLAREDUCEDUPLICATES_HPP
#define SPLA_SPLAREDUCEDUPLICATES_HPP

#include <boost/compute.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace detail {

        class CopyReduceDuplicates : public boost::compute::detail::meta_kernel {
        public:
            CopyReduceDuplicates() : meta_kernel("__spla_reduce_copy_duplicates") {}

            template<
                    class InputIterator,
                    class OffsetIterator,
                    class MaskIterator,
                    class OutputIterator>
            void SetRange(MaskIterator mask,
                          OffsetIterator offset,
                          InputIterator input,
                          OutputIterator result,
                          std::size_t elementsInSequence,
                          std::size_t count,
                          const std::string &reduceOp) {
                using namespace boost;
                mCount = count;

                set_arg(add_arg<cl_ulong>("count"), static_cast<cl_long>(mCount));

                std::stringstream _spla_reduce_op;
                _spla_reduce_op << "void _spla_reduce_op(__global void* vp_a, __global void* vp_b, __global void* vp_c) {\n"
                                << "#define _ACCESS_A __global\n"
                                << "#define _ACCESS_B __global\n"
                                << "#define _ACCESS_C __global\n"
                                << "   " << reduceOp << "\n"
                                << "#undef _ACCESS_A\n"
                                << "#undef _ACCESS_B\n"
                                << "#undef _ACCESS_C\n"
                                << "}";

                add_function("_spla_reduce_op", _spla_reduce_op.str());

                *this << "const uint i = get_global_id(0);\n"
                      << "const uint m = " << mask[expr<compute::uint_>("i")] << ";\n"
                      << "if (m) {\n"
                      << "  const uint offset = " << offset[expr<compute::uint_>("i")] << ";\n"
                      << "  if (i + 1 < count && " << mask[expr<compute::uint_>("i+1")] << " == 0) {\n"
                      << "    const uint arg1 = i * " << elementsInSequence << "\n;"
                      << "    const uint arg2 = (i + 1) * " << elementsInSequence << "\n;"
                      << "    const uint arg3 = offset * " << elementsInSequence << "\n;"
                      << "    _spla_reduce_op(&" << input[expr<compute::uint_>("arg1")] << ", &" << input[expr<compute::uint_>("arg2")] << ", &" << result[expr<compute::uint_>("arg3")] << ");\n"
                      << "  } else {\n"
                      << "    const uint dst = offset * " << elementsInSequence << ";\n"
                      << "    const uint src = i * " << elementsInSequence << ";\n"
                      << "    for (uint k = 0; k < " << elementsInSequence << "; k++)\n"
                      << "      " << result[expr<boost::compute::uint_>("dst + k")] << " = " << input[expr<boost::compute::uint_>("src + k")] << ";\n"
                      << "  }\n"
                      << "}";
            }

            boost::compute::event Exec(boost::compute::command_queue &queue) {
                if (mCount == 0) {
                    return boost::compute::event();
                }

                return exec_1d(queue, 0, mCount);
            }

        private:
            std::size_t mCount = 0;
        };

        inline std::size_t ReduceDuplicatesImpl(const boost::compute::vector<unsigned int> &inputIndices1,
                                                const boost::compute::vector<unsigned int> &inputIndices2,
                                                const boost::compute::vector<unsigned char> &inputValues,
                                                boost::compute::vector<unsigned int> &resultIndices1,
                                                boost::compute::vector<unsigned int> &resultIndices2,
                                                boost::compute::vector<unsigned char> &resultValues,
                                                std::size_t elementsInSequence,
                                                const std::string &reduceOp,
                                                boost::compute::command_queue &queue) {

            using namespace boost;
            auto ctx = queue.get_context();
            auto count = inputIndices1.size();
            auto bothIndices = !inputIndices1.empty() && !inputIndices2.empty();

            if (!count)
                return 0;

            // Store 1 for unique entry, and 0 for duplicated
            compute::vector<unsigned int> unique(count + 1, ctx);
            unique.begin().write(1u, queue);

            // For each entry starting from 1 check if is unique, first is always unique
            if (bothIndices) {
                BOOST_COMPUTE_CLOSURE(
                        unsigned int, findUnique, (unsigned int i), (inputIndices1, inputIndices2), {
                            const uint row = inputIndices1[i];
                            const uint col = inputIndices2[i];
                            const uint rowPrev = inputIndices1[i - 1];
                            const uint colPrev = inputIndices2[i - 1];
                            return rowPrev == row && colPrev == col ? 0 : 1;
                        });

                compute::transform(compute::counting_iterator<unsigned int>(1),
                                   compute::counting_iterator<unsigned int>(count),
                                   unique.begin() + 1,
                                   findUnique,
                                   queue);
            } else {
                BOOST_COMPUTE_CLOSURE(
                        unsigned int, findUnique, (unsigned int i), (inputIndices1), {
                            const uint row = inputIndices1[i];
                            const uint rowPrev = inputIndices1[i - 1];
                            return rowPrev == row ? 0 : 1;
                        });

                compute::transform(compute::counting_iterator<unsigned int>(1),
                                   compute::counting_iterator<unsigned int>(count),
                                   unique.begin() + 1,
                                   findUnique,
                                   queue);
            }

            // Define write offsets (where to write value in result buffer) for each unique value
            compute::vector<unsigned int> offsets(unique.size(), ctx);
            compute::exclusive_scan(unique.begin(), unique.end(), offsets.begin(), 0, queue);

            // Count number of unique values to allocate storage
            std::size_t resultNvals = (offsets.end() - 1).read(queue);

            if (!inputIndices1.empty())
                resultIndices1.resize(resultNvals, queue);
            if (!inputIndices2.empty())
                resultIndices2.resize(resultNvals, queue);

            // NOTE: Edge case - if result nnz is the same as input nnz,
            // then nothing to do, simply copy result
            if (resultNvals == inputIndices1.size()) {
                if (!inputIndices1.empty())
                    compute::copy(inputIndices1.begin(), inputIndices1.end(), resultIndices1.begin(), queue);
                if (!inputIndices2.empty())
                    compute::copy(inputIndices2.begin(), inputIndices2.end(), resultIndices2.begin(), queue);
                if (!inputValues.empty()) {
                    resultValues.resize(resultNvals * elementsInSequence, queue);
                    compute::copy(inputValues.begin(), inputValues.end(), resultValues.begin(), queue);
                }
                return resultNvals;
            }

            // Copy indices
            if (bothIndices) {
                BOOST_COMPUTE_CLOSURE(
                        void, copyIndices, (unsigned int i), (unique, offsets, resultIndices1, inputIndices1, resultIndices2, inputIndices2), {
                            if (unique[i]) {
                                const uint offset = offsets[i];
                                resultIndices1[offset] = inputIndices1[i];
                                resultIndices2[offset] = inputIndices2[i];
                            }
                        });

                compute::for_each_n(compute::counting_iterator<unsigned int>(0),
                                    count,
                                    copyIndices,
                                    queue);
            } else {
                BOOST_COMPUTE_CLOSURE(
                        void, copyIndices, (unsigned int i), (unique, offsets, resultIndices1, inputIndices1), {
                            if (unique[i]) {
                                const uint offset = offsets[i];
                                resultIndices1[offset] = inputIndices1[i];
                            }
                        });

                compute::for_each_n(compute::counting_iterator<unsigned int>(0),
                                    count,
                                    copyIndices,
                                    queue);
            }

            // Copy values
            if (!inputValues.empty()) {
                resultValues.resize(resultNvals * elementsInSequence, queue);

                CopyReduceDuplicates kernel;
                kernel.SetRange(unique.begin(),
                                offsets.begin(),
                                inputValues.begin(),
                                resultValues.begin(),
                                elementsInSequence,
                                count,
                                reduceOp);
                kernel.Exec(queue);
            }

            return resultNvals;
        }

    }// namespace detail

    /**
     * @brief Reduce duplicates in sorted sequence of elements
     * Reduces duplicated values in sorted sequence of key elements,
     * where one or two duplicated keys are presented.
     *
     * @note Keys interpreted as pairs stored in separate containers.
     * @note Reduces values associated with keys using provided reduceOp.
     * @note Result containers automatically resized to store result count values.
     *
     * @param inputIndices1 Keys first elements
     * @param inputIndices2 Keys second elements
     * @param inputValues Input values associated with keys
     * @param resultIndices1 Result keys first elements
     * @param resultIndices2 Result keys second elements
     * @param resultValues Result reduced values
     * @param elementsInSequence Size in byte of single value in raw values buffer
     * @param reduceOp Binary operation used to reduce duplicated values
     * @param queue Queue to perform operation on
     *
     * @return Count of elements in the result
     */
    inline std::size_t ReduceDuplicates(const boost::compute::vector<unsigned int> &inputIndices1,
                                        const boost::compute::vector<unsigned int> &inputIndices2,
                                        const boost::compute::vector<unsigned char> &inputValues,
                                        boost::compute::vector<unsigned int> &resultIndices1,
                                        boost::compute::vector<unsigned int> &resultIndices2,
                                        boost::compute::vector<unsigned char> &resultValues,
                                        std::size_t elementsInSequence,
                                        const std::string &reduceOp,
                                        boost::compute::command_queue &queue) {
        return detail::ReduceDuplicatesImpl(inputIndices1, inputIndices2, inputValues,
                                            resultIndices1, resultIndices2, resultValues,
                                            elementsInSequence,
                                            reduceOp,
                                            queue);
    }

    /**
     * @brief Reduce duplicates in sorted sequence of elements
     * Reduces duplicated values in sorted sequence of key elements,
     * where one or two duplicated keys are presented.
     *
     * @note Keys interpreted as pairs stored in separate containers.
     * @note Result containers automatically resized to store result count values.
     *
     * @param inputIndices1 Keys first elements
     * @param inputIndices2 Keys second elements
     * @param resultIndices1 Result keys first elements
     * @param resultIndices2 Result keys second elements
     * @param queue Queue to perform operation on
     *
     * @return Count of elements in the result
     */
    inline std::size_t ReduceDuplicates(const boost::compute::vector<unsigned int> &inputIndices1,
                                        const boost::compute::vector<unsigned int> &inputIndices2,
                                        boost::compute::vector<unsigned int> &resultIndices1,
                                        boost::compute::vector<unsigned int> &resultIndices2,
                                        boost::compute::command_queue &queue) {
        using namespace boost;

        compute::context ctx = queue.get_context();
        compute::vector<unsigned char> dummyInputValues(ctx);
        compute::vector<unsigned char> dummyResultValues(ctx);

        return detail::ReduceDuplicatesImpl(inputIndices1, inputIndices2, dummyInputValues,
                                            resultIndices1, resultIndices2, dummyResultValues,
                                            0,
                                            "",
                                            queue);
    }

    /**
     * @brief Reduce duplicates in sorted sequence of elements
     * Reduces duplicated values in sorted sequence of key elements,
     * where one or two duplicated keys are presented.
     *
     * @note Reduces values associated with keys using provided reduceOp.
     * @note Result containers automatically resized to store result count values.
     *
     * @param inputIndices Keys elements
     * @param inputValues Input values associated with keys
     * @param resultIndices Result keys elements
     * @param resultValues Result reduced values
     * @param elementsInSequence Size in byte of single value in raw values buffer
     * @param reduceOp Binary operation used to reduce duplicated values
     * @param queue Queue to perform operation on
     *
     * @return Count of elements in the result
     */
    inline std::size_t ReduceDuplicates(const boost::compute::vector<unsigned int> &inputIndices,
                                        const boost::compute::vector<unsigned char> &inputValues,
                                        boost::compute::vector<unsigned int> &resultIndices,
                                        boost::compute::vector<unsigned char> &resultValues,
                                        std::size_t elementsInSequence,
                                        const std::string &reduceOp,
                                        boost::compute::command_queue &queue) {
        using namespace boost;

        compute::context ctx = queue.get_context();
        compute::vector<unsigned int> dummyInputIndices(ctx);
        compute::vector<unsigned int> dummyResultIndices(ctx);

        return detail::ReduceDuplicatesImpl(inputIndices, dummyInputIndices, inputValues,
                                            resultIndices, dummyResultIndices, resultValues,
                                            elementsInSequence,
                                            reduceOp,
                                            queue);
    }

    /**
     * @brief Reduce duplicates in sorted sequence of elements
     * Reduces duplicated values in sorted sequence of key elements,
     * where one or two duplicated keys are presented.
     *
     * @note Result containers automatically resized to store result count values.
     *
     * @param inputIndices Keys elements
     * @param resultIndices Result keys elements
     * @param queue Queue to perform operation on
     *
     * @return Count of elements in the result
     */
    inline std::size_t ReduceDuplicates(const boost::compute::vector<unsigned int> &inputIndices,
                                        boost::compute::vector<unsigned int> &resultIndices,
                                        boost::compute::command_queue &queue) {
        using namespace boost;

        compute::context ctx = queue.get_context();
        compute::vector<unsigned int> dummyInputIndices(ctx);
        compute::vector<unsigned int> dummyResultIndices(ctx);
        compute::vector<unsigned char> dummyInputValues(ctx);
        compute::vector<unsigned char> dummyResultValues(ctx);

        return detail::ReduceDuplicatesImpl(inputIndices, dummyInputIndices, dummyInputValues,
                                            resultIndices, dummyResultIndices, dummyResultValues,
                                            0,
                                            "",
                                            queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAREDUCEDUPLICATES_HPP
