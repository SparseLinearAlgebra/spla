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

    namespace {

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

                std::stringstream _spla_reduce_op;
                _spla_reduce_op << "void _spla_reduce_op(__global void* vp_a, __global void* vp_b, __global void* vp_c) {\n"
                                << "   " << reduceOp << "\n"
                                << "}";

                add_function("_spla_reduce_op", _spla_reduce_op.str());

                *this << "const uint i = get_global_id(0);\n"
                      << "const uint m = " << mask[expr<compute::uint_>("i")] << ";\n"
                      << "if (m) {\n"
                      << "  const uint offset = " << offset[expr<compute::uint_>("i")] << ";\n"
                      << "  if (i + 1 < " << count << " && " << mask[expr<compute::uint_>("i+1")] << " == 0) {\n"
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

    }// namespace

    inline std::size_t ReduceDuplicates(const boost::compute::vector<unsigned int> &inputIndices,
                                        const boost::compute::vector<unsigned char> &inputValues,
                                        boost::compute::vector<unsigned int> &resultIndices,
                                        boost::compute::vector<unsigned char> &resultValues,
                                        std::size_t elementsInSequence,
                                        const std::string &reduceOp,
                                        boost::compute::command_queue &queue) {
        using namespace boost;
        auto ctx = queue.get_context();
        auto count = inputIndices.size();

        compute::vector<unsigned int> unique(count + 1, ctx);
        unique.begin().write(1u, queue);

        BOOST_COMPUTE_CLOSURE(
                unsigned int, findUnique, (unsigned int i), (inputIndices), {
                    const uint row = inputIndices[i];
                    const uint rowPrev = inputIndices[i - 1];
                    return rowPrev == row ? 0 : 1;
                });

        // For each entry starting from 1 check if is unique, first is always unique
        compute::transform(compute::counting_iterator<unsigned int>(1),
                           compute::counting_iterator<unsigned int>(count),
                           unique.begin() + 1,
                           findUnique,
                           queue);

        // Define write offsets (where to write value in result buffer) for each unique value
        compute::vector<unsigned int> offsets(unique.size(), ctx);
        compute::exclusive_scan(unique.begin(), unique.end(), offsets.begin(), 0, queue);

        // Count number of unique values to allocate storage
        std::size_t resultNvals = (offsets.end() - 1).read(queue);

        resultIndices.resize(resultNvals, queue);

        // Copy indices
        BOOST_COMPUTE_CLOSURE(
                void, copyIndices, (unsigned int i), (unique, offsets, resultIndices, inputIndices), {
                    if (unique[i]) {
                        const uint offset = offsets[i];
                        resultIndices[offset] = inputIndices[i];
                    }
                });
        compute::for_each_n(compute::counting_iterator<unsigned int>(0),
                            count,
                            copyIndices,
                            queue);

        resultValues.resize(resultNvals * elementsInSequence, queue);

        // Copy values
        CopyReduceDuplicates copyReduceDuplicates;
        copyReduceDuplicates.SetRange(unique.begin(),
                                      offsets.begin(),
                                      inputValues.begin(),
                                      resultValues.begin(),
                                      elementsInSequence,
                                      count,
                                      reduceOp);
        copyReduceDuplicates.Exec(queue);

        queue.finish();
        return resultNvals;
    }

}// namespace spla

#endif//SPLA_SPLAREDUCEDUPLICATES_HPP
