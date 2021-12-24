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

#ifndef SPLA_SPLAREDUCE2_HPP
#define SPLA_SPLAREDUCE2_HPP

#include <compute/metautil/SplaMetaUtil.hpp>

#include <boost/compute/container/vector.hpp>
#include <boost/compute/kernel.hpp>

#include <cassert>
#include <cmath>
#include <sstream>

namespace spla {

    inline void Reduce2(const boost::compute::vector<unsigned char> &values,
                        boost::compute::vector<unsigned char> &result,
                        std::size_t count,
                        unsigned int byteSize,
                        const std::string &reduceOp,
                        boost::compute::command_queue &queue) {
        using namespace boost;

        assert(count > 0);

        result.resize(byteSize, queue);

        // Edge case - nothing to do
        if (count == 1) {
            compute::copy_n(values.begin(), byteSize, result.begin(), queue);
            return;
        }

        auto ctx = queue.get_context();
        auto device = queue.get_device();

        compute::detail::meta_kernel kernel("__spla_reduce_2");
        auto argCount = kernel.add_arg<cl_uint>("count");
        auto argInput = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "input");
        auto argOutput = kernel.add_arg<cl_uchar *>(compute::memory_object::global_memory, "output");

        const cl_ulong MAX_LOCAL_BUFFER_SIZE = device.get_info<cl_ulong>(CL_DEVICE_LOCAL_MEM_SIZE);
        const cl_ulong MAX_ITEMS_PER_WORK_GROUP = MAX_LOCAL_BUFFER_SIZE / static_cast<cl_ulong>(byteSize);
        const cl_ulong DESIRED_BLOCK_SIZE = 128;
        const cl_ulong AVAILABLE_BLOCK_SIZE = std::min(MAX_ITEMS_PER_WORK_GROUP, DESIRED_BLOCK_SIZE);
        const cl_ulong BLOCK_SIZE = 1ul << static_cast<cl_uint>(std::floor(std::log2(AVAILABLE_BLOCK_SIZE)));

        assert(!((BLOCK_SIZE - 1) & BLOCK_SIZE) && "Must be power of 2");

        std::string access_mode("__local");
        std::string reduce_function("reduce_function");
        kernel.add_function(reduce_function, detail::MakeFunction(reduce_function, reduceOp, access_mode, access_mode, access_mode));

        // Why do we have to use `__local ulong lvalues_buffer[]` ?
        // It throws INVALID_COMMAND_QUEUE on nvidia because it requires 8-byte alignment for __local pointers
        // See this issue info:
        // https://forums.developer.nvidia.com/t/cl-invalid-command-queue-error-due-to-local-memory-byte-alignment/78379
        // https://github.com/fangq/mcxcl/commit/136f3bf1f94882d388cc72283c031f4f80c73f6e

        kernel << "#define BLOCK_SIZE " << BLOCK_SIZE << "\n"
               << "#define BYTE_SIZE " << byteSize << "\n"
               << "const uint gid = get_global_id(0);\n"
               << "const uint lid = get_local_id(0);\n"
               << "const uint wid = get_group_id(0);\n"
               << "const uint gid_base = wid * BLOCK_SIZE;\n"
               << "__local ulong lvalues_buffer[(BLOCK_SIZE * BYTE_SIZE) /" << static_cast<cl_uint>(sizeof(cl_ulong)) << "];\n"
               << "__local uchar* lvalues = (__local uchar*)lvalues_buffer;\n"
               << "if (gid < count) {\n"
               << "    for (uint i = 0; i < BYTE_SIZE; i++) {\n"
               << "        lvalues[lid * BYTE_SIZE + i] = input[gid * BYTE_SIZE + i];\n"
               << "    }\n"
               << "}\n"
               << "for (uint stride = BLOCK_SIZE/2; stride > 0; stride = stride / 2) {\n"
               << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "    if (lid < stride && (gid_base + lid + stride) < count) {\n"
               << "        " << reduce_function << "(&lvalues[lid * BYTE_SIZE], &lvalues[(lid + stride) * BYTE_SIZE], &lvalues[lid * BYTE_SIZE]);\n;"
               << "    }\n"
               << "}\n"
               << "if (lid == 0) {\n"
               << "    for (uint i = 0; i < BYTE_SIZE; i++) {\n"
               << "        output[wid * BYTE_SIZE + i] = lvalues[0 * BYTE_SIZE + i];\n"
               << "    }\n"
               << "}\n";

        compute::kernel compiledKerned = kernel.compile(ctx);

        auto getNBlocks = [=](cl_ulong count) { return count / BLOCK_SIZE + (count % BLOCK_SIZE ? 1 : 0); };

        // Resize buffers ahead
        compute::vector<unsigned char> tmpValuesBuffer1((count > BLOCK_SIZE ? getNBlocks(count) : 0) * byteSize, ctx);
        compute::vector<unsigned char> tmpValuesBuffer2((getNBlocks(count) > BLOCK_SIZE ? getNBlocks(getNBlocks(count)) : 0) * byteSize, ctx);

        // Pointers to ping-pong inside while loop
        const compute::buffer *input = &values.get_buffer();
        const compute::buffer *output = &tmpValuesBuffer1.get_buffer();

        // Reduce while have something to reduce
        while (count > BLOCK_SIZE) {
            // Number of block for iteration (will become new count on the next iteration)
            auto blocksCount = getNBlocks(count);
            compiledKerned.set_arg(argCount, static_cast<cl_uint>(count));
            compiledKerned.set_arg(argInput, *input);
            compiledKerned.set_arg(argOutput, *output);
            queue.enqueue_1d_range_kernel(compiledKerned, 0, blocksCount * BLOCK_SIZE, BLOCK_SIZE);

            // If first iteration, update input to point to tmp buffer
            if (input == &values.get_buffer())
                input = &tmpValuesBuffer2.get_buffer();

            std::swap(input, output);
            count = blocksCount;
        }

        // Reduce to final result buffer (guaranty to be in input)
        compiledKerned.set_arg(argCount, static_cast<cl_uint>(count));
        compiledKerned.set_arg(argInput, *input);
        compiledKerned.set_arg(argOutput, result.get_buffer());
        queue.enqueue_1d_range_kernel(compiledKerned, 0, BLOCK_SIZE, BLOCK_SIZE);
    }

}// namespace spla

#endif//SPLA_SPLAREDUCE2_HPP
