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

#ifndef SPLA_SPLACOPYUTILS_HPP
#define SPLA_SPLACOPYUTILS_HPP

#include <boost/compute/command_queue.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/detail/meta_kernel.hpp>

#include <cassert>
#include <iostream>

namespace spla {

    inline boost::compute::event CopyMergedValues(const boost::compute::vector<unsigned int> &offsets,
                                                  const boost::compute::vector<unsigned char> &valuesA,
                                                  const boost::compute::vector<unsigned char> &valuesB,
                                                  boost::compute::vector<unsigned char> &outputValues,
                                                  std::size_t baseOffset,
                                                  std::size_t byteSize,
                                                  boost::compute::command_queue &queue) {
        using namespace boost;

        if (offsets.empty())
            return {};

        assert(outputValues.size() <= offsets.size() * byteSize);

        compute::detail::meta_kernel kernel("__spla_copy_merged_values");
        auto argBaseOffset = kernel.add_arg<cl_uint>("baseOffset");
        auto argCount = kernel.add_arg<cl_uint>("count");
        auto argOffsets = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "offsets");
        auto argValuesA = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "valuesA");
        auto argValuesB = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "valuesB");
        auto argOutputValues = kernel.add_arg<cl_uchar *>(compute::memory_object::global_memory, "outputValues");

        kernel << "#define BYTE_SIZE " << static_cast<cl_ulong>(byteSize) << "\n"
               << "const uint i = get_global_id(0);\n"
               << "if (i < count) {\n"
               << "    const uint idx = offsets[i];\n"
               << "    if (idx < baseOffset) {\n"
               << "        const uint dst = i * BYTE_SIZE;\n"
               << "        const uint src = idx * BYTE_SIZE;\n"
               << "        for (uint k = 0; k < BYTE_SIZE; k++)\n"
               << "            outputValues[dst + k] = valuesA[src + k];\n"
               << "    } else {\n"
               << "        const uint dst = i * BYTE_SIZE;\n"
               << "        const uint src = (idx - baseOffset) * BYTE_SIZE;\n"
               << "        for (uint k = 0; k < BYTE_SIZE; k++)\n"
               << "            outputValues[dst + k] = valuesB[src + k];\n"
               << "    }\n"
               << "}\n";

        auto compiledKernel = kernel.compile(queue.get_context());
        compiledKernel.set_arg(argBaseOffset, static_cast<cl_uint>(baseOffset));
        compiledKernel.set_arg(argCount, static_cast<cl_uint>(offsets.size()));
        compiledKernel.set_arg(argOffsets, offsets.get_buffer());
        compiledKernel.set_arg(argValuesA, valuesA.get_buffer());
        compiledKernel.set_arg(argValuesB, valuesB.get_buffer());
        compiledKernel.set_arg(argOutputValues, outputValues.get_buffer());

        const std::size_t blockSize = 128;
        const std::size_t workSize = compute::detail::calculate_work_size(offsets.size(), 1, blockSize);

        return queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, blockSize);
    }

    inline boost::compute::event OffsetIndices(boost::compute::vector<unsigned int> &indices,
                                               std::size_t offset,
                                               boost::compute::command_queue &queue) {
        using namespace boost;

        if (indices.empty())
            return compute::event();

        compute::detail::meta_kernel kernel("__spla_offset_indices");
        auto argOffset = kernel.add_arg<cl_uint>("offset");
        auto argIndices = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "indices");

        kernel << "const uint i = get_global_id(0);\n"
               << "indices[i] = indices[i] + offset;\n";

        auto compiledKernel = kernel.compile(queue.get_context());
        compiledKernel.set_arg(argOffset, static_cast<cl_uint>(offset));
        compiledKernel.set_arg(argIndices, indices.get_buffer());

        return queue.enqueue_1d_range_kernel(compiledKernel, 0, indices.size(), 0);
    }

}// namespace spla

#endif//SPLA_SPLACOPYUTILS_HPP
