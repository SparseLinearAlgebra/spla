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

#ifndef SPLA_SPLATRANSFORMVALUES_HPP
#define SPLA_SPLATRANSFORMVALUES_HPP

#include <boost/compute/command_queue.hpp>
#include <boost/compute/container/vector.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace detail {

        class TransformValuesKernel : public boost::compute::detail::meta_kernel {
        public:
            TransformValuesKernel() : boost::compute::detail::meta_kernel("__spla_transform_values_kernel") {
            }

            template<typename InputMap,
                     typename InputValues,
                     typename OutputValues>
            void SetRange(InputMap aMap,
                          InputMap bMap,
                          InputValues aValues,
                          InputValues bValues,
                          OutputValues outputValues,
                          std::size_t aByteSize,
                          std::size_t bByteSize,
                          std::size_t resultByteSize,
                          std::size_t count,
                          const std::string &transformOp) {
                using namespace boost;
                mCount = count;

                std::stringstream _spla_transform_op;
                _spla_transform_op << "void _spla_transform_op(__global void* vp_a, __global void* vp_b, __global void* vp_c) {\n"
                                   << "#define _ACCESS_A __global\n"
                                   << "#define _ACCESS_B __global\n"
                                   << "#define _ACCESS_C __global\n"
                                   << "   " << transformOp << "\n"
                                   << "#undef _ACCESS_A\n"
                                   << "#undef _ACCESS_B\n"
                                   << "#undef _ACCESS_C\n"
                                   << "}";

                add_function("_spla_transform_op", _spla_transform_op.str());

                *this << "const uint i = get_global_id(0);\n"
                      << "const uint a_idx = " << aMap[expr<compute::uint_>("i")] << ";\n"
                      << "const uint b_idx = " << bMap[expr<compute::uint_>("i")] << ";\n"
                      << "const uint a_offset = a_idx * " << aByteSize << ";\n"
                      << "const uint b_offset = b_idx * " << bByteSize << ";\n"
                      << "const uint result_offset = i * " << resultByteSize << ";\n"
                      << "_spla_transform_op(&" << aValues[expr<compute::uint_>("a_offset")] << ", &" << bValues[expr<compute::uint_>("b_offset")] << ", &" << outputValues[expr<compute::uint_>("result_offset")] << ");\n";
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

    }// namespace detail

    /**
     * @brief Transforms a and b values using map and transfrom op
     * Transforms sequence of a and b values using map to fetch actual
     * a and b values. For each index i of computes result values as v[i] = transform(a[a_map[i]], b[b_map[i]]).
     *
     * @param aMap Map for a values
     * @param bMap Map for b values
     * @param aValues A values
     * @param bValues B values
     * @param values Where to store result of transformation; must be allocated by the user
     * @param aByteSize Size of a values
     * @param bByteSize Size of b values
     * @param resultByteSize Size of result values
     * @param transformOp Source code for transform function
     * @param queue Command queue to perform operation
     *
     * @return Event to syn this operation
     */
    inline boost::compute::event TransformValues(const boost::compute::vector<unsigned int> &aMap,
                                                 const boost::compute::vector<unsigned int> &bMap,
                                                 const boost::compute::vector<unsigned char> &aValues,
                                                 const boost::compute::vector<unsigned char> &bValues,
                                                 boost::compute::vector<unsigned char> &values,
                                                 std::size_t aByteSize,
                                                 std::size_t bByteSize,
                                                 std::size_t resultByteSize,
                                                 const std::string &transformOp,
                                                 boost::compute::command_queue &queue) {
        using namespace boost;
        auto count = aMap.size();

        assert(aMap.size() == bMap.size());
        assert(values.size() == count * resultByteSize);

        detail::TransformValuesKernel kernel;
        kernel.SetRange(aMap.begin(), bMap.begin(),
                        aValues.begin(), bValues.begin(),
                        values.begin(),
                        aByteSize,
                        bByteSize,
                        resultByteSize,
                        count,
                        transformOp);

        return kernel.Exec(queue);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLATRANSFORMVALUES_HPP
