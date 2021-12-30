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
#ifndef SPLA_SPLAREDUCE_HPP
#define SPLA_SPLAREDUCE_HPP

#include <boost/compute/algorithm/reduce.hpp>

#include <compute/metautil/SplaMetaUtil.hpp>


namespace spla {

    namespace detail {

        inline void InplaceReduce(const boost::compute::vector<unsigned char> &values,
                                  std::size_t valueByteSize,
                                  const std::string &functionBody,
                                  boost::compute::command_queue &queue) {
            using namespace detail::meta;
            using namespace boost;

            std::size_t inputSize = values.size() / valueByteSize;
            if (inputSize < 2) {
                return;
            }

            const compute::context &context = queue.get_context();

            const std::size_t blockSize = 64;
            const std::size_t valuesPerThread = 8;
            std::size_t blockCount = inputSize / (blockSize * valuesPerThread);
            if (blockCount * blockSize * valuesPerThread != inputSize)
                blockCount++;

            compute::vector<unsigned char> output(blockCount * valueByteSize, context);

            compute::detail::meta_kernel k("inplace_reduce");
            size_t inputArg = k.add_arg<unsigned char *>(compute::memory_object::global_memory, "input");
            size_t inputSizeArg = k.add_arg<const uint_>("input_size");
            size_t outputArg = k.add_arg<unsigned char *>(compute::memory_object::global_memory, "output");
            size_t scratchArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "scratch");

            FunctionApplication reduceOp1(k, "inplace_reduce_reduce_1", functionBody, valueByteSize,
                                          Visibility::Unspecified,
                                          Visibility::Global,
                                          Visibility::Unspecified);

            FunctionApplication reduceOp2(k, "inplace_reduce_reduce_2", functionBody, valueByteSize,
                                          Visibility::Local,
                                          Visibility::Local,
                                          Visibility::Unspecified);

            k << "const uint gid = get_global_id(0);\n"
              << "const uint lid = get_local_id(0);\n"
              << "const uint values_per_thread = " << uint_(valuesPerThread) << ";\n"
              << "const uint index = gid * values_per_thread;\n"
              << "if(index < input_size){\n"
              << DeclareVal{"sum", valueByteSize} << ";\n"
              << AssignVal{ValVar{"sum"}, ValArrItem{"input", "index", valueByteSize}, valueByteSize} << ";\n"
              << "for(uint i = 1; i < values_per_thread && (index + i) < input_size; i++) {\n"
              << reduceOp1.Apply(ValVar{"sum"}, ValArrItem{"input", "index+i", valueByteSize}, ValVar{"sum"}) << ";\n"
              << "}\n"
              << AssignVal{ValArrItem("scratch", "lid", valueByteSize), ValVar("sum"), valueByteSize} << ";\n"
              << "}\n"
              << "for(uint i = 1; i < get_local_size(0); i <<= 1){\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    uint mask = (i << 1) - 1;\n"
              << "    uint next_index = (gid + i) * values_per_thread;\n"
                 "    if((lid & mask) == 0 && next_index < input_size){\n"
              << reduceOp2.Apply(ValArrItem("scratch", "lid", valueByteSize), ValArrItem("scratch", "lid+i", valueByteSize), ValArrItem("scratch", "lid", valueByteSize))
              << "    }\n"
              << "}\n"
              << "if(lid == 0){\n"
              << AssignVal{ValArrItem("output", "get_group_id(0)", valueByteSize), ValArrItem("scratch", "0", valueByteSize), valueByteSize} << ";\n"
              << "}\n";

            const compute::buffer *inputBuffer = &values.get_buffer();
            const compute::buffer *outputBuffer = &output.get_buffer();

            compute::kernel kernel = k.compile(context);

            while (inputSize > 1) {
                kernel.set_arg(inputArg, *inputBuffer);
                kernel.set_arg(inputSizeArg, static_cast<uint_>(inputSize));
                kernel.set_arg(outputArg, *outputBuffer);
                kernel.set_arg(scratchArg, compute::local_buffer<unsigned char>(blockSize * valueByteSize));

                queue.enqueue_1d_range_kernel(kernel,
                                              0,
                                              blockCount * blockSize,
                                              blockSize);

                inputSize = static_cast<std::size_t>(std::ceil(static_cast<float>(inputSize) / (blockSize * valuesPerThread)));

                blockCount = inputSize / (blockSize * valuesPerThread);
                if (blockCount * blockSize * valuesPerThread != inputSize)
                    blockCount++;

                std::swap(inputBuffer, outputBuffer);
            }

            if (inputBuffer != &values.get_buffer()) {
                ::boost::compute::copy(output.begin(),
                                       output.begin() + static_cast<std::ptrdiff_t>(valueByteSize),
                                       values.begin(),
                                       queue);
            }
        }

        inline std::size_t Reduce(const boost::compute::vector<unsigned char> &values,
                                  std::size_t valueByteSize,
                                  boost::compute::vector<unsigned char> &result,
                                  std::size_t blockSize,
                                  const std::string &functionBody,
                                  boost::compute::command_queue &queue) {
            using namespace detail::meta;
            using namespace boost;

            const compute::context &context = queue.get_context();
            const std::size_t nValues = values.size() / valueByteSize;
            const std::size_t blockCount = nValues / 2 / blockSize;
            const auto totalBlockCount = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(nValues) / 2.f / static_cast<float>(blockSize)));

            if (blockCount != 0) {
                compute::detail::meta_kernel k("block_reduce");
                std::size_t outputArg = k.add_arg<unsigned char *>(compute::memory_object::global_memory, "output");
                std::size_t blockArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "block");

                FunctionApplication reduceOpGlobal(k, "block_reduce_reduce_global", functionBody, valueByteSize,
                                                   Visibility::Global,
                                                   Visibility::Global,
                                                   Visibility::Local);
                FunctionApplication reduceOpLocal(k, "block_reduce_reduce_local", functionBody, valueByteSize,
                                                  Visibility::Local,
                                                  Visibility::Local,
                                                  Visibility::Unspecified);

                k << "const uint gid = get_global_id(0);\n"
                  << "const uint lid = get_local_id(0);\n"
                  << reduceOpGlobal.Apply(ValArrItem(values, "gid*2+0", valueByteSize, k),
                                          ValArrItem(values, "gid*2+1", valueByteSize, k),
                                          ValArrItem("block", "lid", valueByteSize))
                  << "for(uint i = 1; i < " << uint_(blockSize) << "; i <<= 1){\n"
                  << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
                  << "    uint mask = (i << 1) - 1;\n"
                  << "    if((lid & mask) == 0){\n"
                  << reduceOpLocal.Apply(ValArrItem("block", "lid", valueByteSize),
                                         ValArrItem("block", "lid+i", valueByteSize),
                                         ValArrItem("block", "lid", valueByteSize))
                  << "    }\n"
                  << "}\n"
                  << "if(lid == 0)\n"
                  << AssignVal{ValArrItem{"output", "get_group_id(0)", valueByteSize},
                               ValArrItem{"block", "0", valueByteSize},
                               valueByteSize}
                  << ";\n";

                compute::kernel kernel = k.compile(context);
                kernel.set_arg(outputArg, result.get_buffer());
                kernel.set_arg(blockArg, compute::local_buffer<unsigned char>(blockSize * valueByteSize));

                queue.enqueue_1d_range_kernel(kernel,
                                              0,
                                              blockCount * blockSize,
                                              blockSize);
            }

            // serially reduce any leftovers
            if (blockCount * blockSize * 2 < nValues) {
                std::size_t lastBlockStart = blockCount * blockSize * 2;

                compute::detail::meta_kernel k("extra_serial_reduce");
                const std::size_t countArg = k.add_arg<uint_>("count");
                const std::size_t offsetArg = k.add_arg<uint_>("offset");
                const std::size_t outputArg = k.add_arg<unsigned char *>(compute::memory_object::global_memory, "output");
                const std::size_t outputOffsetArg = k.add_arg<uint_>("output_offset");

                FunctionApplication reduceOp(k, "leftover_reduce_reduce", functionBody, valueByteSize,
                                             Visibility::Unspecified,
                                             Visibility::Global,
                                             Visibility::Unspecified);

                k << DeclareVal{"result", valueByteSize} << ";\n"
                  << AssignVal{ValVar{"result"}, ValArrItem{values, "offset", valueByteSize, k}, valueByteSize}
                  << "for(uint i = offset + 1; i < count; i++)\n"
                  << reduceOp.Apply(ValVar{"result"},
                                    ValArrItem(values, "i", valueByteSize, k),
                                    ValVar{"result"})
                  << AssignVal{ValArrItem{"output", "output_offset", valueByteSize},
                               ValVar{"result"},
                               valueByteSize}
                  << ";\n";

                compute::kernel kernel = k.compile(context);
                kernel.set_arg(countArg, static_cast<uint_>(nValues));
                kernel.set_arg(offsetArg, static_cast<uint_>(lastBlockStart));
                kernel.set_arg(outputArg, result.get_buffer());
                kernel.set_arg(outputOffsetArg, static_cast<uint_>(blockCount));

                queue.enqueue_task(kernel);
            }

            return totalBlockCount;
        }

        inline boost::compute::vector<unsigned char> BlockReduce(const boost::compute::vector<unsigned char> &values,
                                                                 std::size_t valueByteSize,
                                                                 std::size_t blockSize,
                                                                 const std::string &functionBody,
                                                                 boost::compute::command_queue &queue) {
            using namespace boost;

            const compute::context &ctx = queue.get_context();
            const std::size_t nValues = values.size() / valueByteSize;

            auto totalBlockCount = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(nValues) / 2.f / static_cast<float>(blockSize)));
            compute::vector<unsigned char> resultVector(totalBlockCount * valueByteSize, ctx);

            Reduce(values, valueByteSize, resultVector, blockSize, functionBody, queue);

            return resultVector;
        }

        inline void GenericReduce(const boost::compute::vector<unsigned char> &values,
                                  std::size_t valueByteSize,
                                  boost::compute::vector<unsigned char> &result,
                                  const std::string &functionBody,
                                  boost::compute::command_queue &queue) {
            using namespace boost;

            const std::size_t blockSize = 256;

            compute::vector<unsigned char> results = BlockReduce(values, valueByteSize, blockSize, functionBody, queue);

            if (results.size() / valueByteSize > 1) {
                InplaceReduce(results,
                              valueByteSize,
                              functionBody,
                              queue);
            }

            compute::copy_n(results.begin(), valueByteSize, result.begin(), queue);
        }

    }// namespace detail

    inline boost::compute::vector<unsigned char> Reduce(const boost::compute::vector<unsigned char> &values,
                                                        std::size_t valueByteSize,
                                                        const std::string &reduceOp,
                                                        boost::compute::command_queue &queue) {
        using namespace boost;

        const compute::context &ctx = queue.get_context();
        boost::compute::vector<unsigned char> result(valueByteSize, ctx);
        compute::fill(result.begin(), result.end(), 0, queue);

        detail::GenericReduce(values, valueByteSize, result, reduceOp, queue);

        return result;
    }

}// namespace spla

#endif//SPLA_SPLAREDUCE_HPP
