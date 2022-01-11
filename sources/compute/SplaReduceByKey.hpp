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

#ifndef SPLA_SPLAREDUCEBYKEY_HPP
#define SPLA_SPLAREDUCEBYKEY_HPP

#include <functional>

#include <boost/compute/algorithm/for_each_n.hpp>
#include <boost/compute/algorithm/inclusive_scan.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/detail/meta_kernel.hpp>
#include <boost/compute/iterator/counting_iterator.hpp>
#include <boost/compute/memory/local_buffer.hpp>

#include <compute/metautil/SplaMetaUtil.hpp>


namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    namespace detail {

        inline void GenerateUintKeys(const std::vector<std::reference_wrapper<const boost::compute::vector<unsigned int>>> &keysFirst,
                                     const boost::compute::vector<unsigned int>::iterator &newKeysFirst,
                                     std::size_t preferredWorkGroupSize,
                                     boost::compute::command_queue &queue) {
            using namespace boost;
            using namespace spla::detail::meta;
            using compute::uint_;

            compute::detail::meta_kernel k("spla_reduce_by_key_new_key_flags");
            const std::size_t count = keysFirst.at(0).get().size();
            const std::size_t nKeys = keysFirst.size();
            k.add_set_arg<const uint_>("count", static_cast<uint_>(count));

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<uint_>("value") << " = 0;\n"
              << "if(gid >= count){\n    return;\n}\n"
              << "if(gid > 0){ \n"
              << DeclareMultiKey{"key", nKeys} << ";\n"
              << AssignKey{KeyVar("key"), KeyVec(keysFirst, "gid", k), nKeys} << ";\n"
              << DeclareMultiKey{"previous_key", nKeys} << ";\n"
              << AssignKey{KeyVar("previous_key"), KeyVec(keysFirst, "gid - 1", k), nKeys}
              << "    value = " << CompareKey{KeyVar("previous_key"), KeyVar("key"), nKeys} << " ? 0 : 1;\n"
              << "}\n else {\n"
              << "    value = 0;\n"
              << "}\n"
              << newKeysFirst[k.var<const uint_>("gid")] << " = value;\n";

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);

            auto workGroupsNo = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(count) / static_cast<float>(preferredWorkGroupSize)));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * preferredWorkGroupSize,
                                          preferredWorkGroupSize);

            inclusive_scan(newKeysFirst, newKeysFirst + static_cast<std::ptrdiff_t>(count),
                           newKeysFirst, queue);
        }

        inline void CarryOuts(const boost::compute::vector<boost::compute::uint_> &keys,
                              const boost::compute::vector<unsigned char> &values,
                              const boost::compute::vector<boost::compute::uint_>::iterator &carryOutsKeysFirst,
                              const boost::compute::vector<unsigned char>::iterator &carryOutValuesFirst,
                              std::size_t workGroupSize,
                              std::size_t vBytes,
                              const std::string &reduceBody,
                              boost::compute::command_queue &queue) {
            using namespace boost;
            using namespace detail::meta;
            using compute::uint_;

            compute::vector<uint_>::iterator keysFirst = keys.begin();
            compute::vector<unsigned char>::iterator valuesFirst = values.begin();
            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_carry_outs");
            k.add_set_arg<const uint_>("count", static_cast<uint_>(keys.size()));
            size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");

            FunctionApplication reduceOp(k, "spla_reduce", reduceBody, vBytes);

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              << k.decl<uint_>("key") << ";\n"
              << DeclareVal{"value", vBytes} << ";\n"
              << "if(gid < count){\n"
              << k.var<uint_>("key") << " = " << keysFirst[k.var<const uint_>("gid")] << ";\n"
              << AssignVal{ValVar("value"), ValArrItem(valuesFirst, "gid", vBytes, k), vBytes} << ";\n"
              << "lkeys[lid] = key;\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("value"), vBytes} << ";\n"
              << "}\n"
              << DeclareVal{"result", vBytes} << ";\n"
              << AssignVal{ValVar("result"), ValVar("value"), vBytes} << ";\n"
              << k.decl<uint_>("other_key") << ";\n"
              << DeclareVal{"other_value", vBytes} << ";\n"
              << "for (" << k.decl<uint_>("offset") << " = 1; offset < wg_size; offset *= 2) {\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if (lid >= offset){\n"
              << "        other_key = lkeys[lid - offset];\n"
              << "        if(other_key == key){\n"
              << AssignVal{ValVar("other_value"), ValArrItem("lvals", "lid - offset", vBytes), vBytes} << ";\n"
              << reduceOp.Apply(ValVar("result"), ValVar("other_value"), ValVar("result")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("result"), vBytes} << ";\n"
              << "}\n"
              << "if (lid == (wg_size - 1)){\n"
              << carryOutsKeysFirst[k.var<const uint_>("group_id")] << " = key;\n"
              << AssignVal{ValArrItem(carryOutValuesFirst, "group_id", vBytes, k), ValVar("result"), vBytes} << ";\n"
              << "}\n";


            auto workGroupsNo = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(keys.size()) / static_cast<float>(workGroupSize)));

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            kernel.set_arg(localKeysArg, compute::local_buffer<uint_>(workGroupSize));
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize * vBytes));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * workGroupSize,
                                          workGroupSize);
        }

        inline void CarryIns(const boost::compute::vector<boost::compute::uint_>::iterator &carryOutsKeysFirst,
                             const boost::compute::vector<unsigned char>::iterator &carryOutValuesFirst,
                             const boost::compute::vector<unsigned char>::iterator &carryInValuesFirst,
                             std::size_t carryOutSize,
                             std::size_t workGroupSize,
                             std::size_t vBytes,
                             const std::string &reduceBody,
                             boost::compute::command_queue &queue) {
            using namespace boost;
            using namespace detail::meta;
            using compute::uint_;

            auto valuesPreWorkItem = static_cast<uint_>(
                    std::ceil(static_cast<float>(carryOutSize) / static_cast<float>(workGroupSize)));

            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_carry_ins");
            k.add_set_arg<const uint_>("carry_out_size", uint_(carryOutSize));
            k.add_set_arg<const uint_>("values_per_work_item", valuesPreWorkItem);
            std::size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            std::size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");

            FunctionApplication reduceOp(k, "spla_reduce", reduceBody, vBytes);

            k << k.decl<uint_>("id") << " = get_global_id(0) * values_per_work_item;\n"
              << k.decl<uint_>("idx") << " = id;\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              << k.decl<uint_>("key") << ";\n"
              << DeclareVal{"value", vBytes} << ";\n"
              << k.decl<uint_>("previous_key") << ";\n"
              << DeclareVal{"result", vBytes} << ";\n"
              << "if (id < carry_out_size) {\n"
              << k.var<uint_>("previous_key") << " = " << carryOutsKeysFirst[k.var<const uint_>("id")] << ";\n"
              << AssignVal{ValVar("result"), ValArrItem(carryOutValuesFirst, "id", vBytes, k), vBytes} << ";\n"
              << AssignVal{ValArrItem(carryInValuesFirst, "id", vBytes, k), ValVar("result"), vBytes} << ";\n"
              << "}\n"
              << k.decl<const uint_>("end") << " = (id + values_per_work_item) <= carry_out_size"
              << " ? (values_per_work_item + id) :  carry_out_size;\n"
              << "for (idx = idx + 1; idx < end; idx += 1) {\n"
              << "    key = " << carryOutsKeysFirst[k.var<const uint_>("idx")] << ";\n"
              << AssignVal{ValVar("value"), ValArrItem(carryOutValuesFirst, "idx", vBytes, k), vBytes} << ";\n"
              << "    if(previous_key == key){\n"
              << reduceOp.Apply(ValVar("result"), ValVar("value"), ValVar("result")) << ";\n"
              << "    }\n else { \n"
              << AssignVal{ValVar("result"), ValVar("value"), vBytes} << ";\n"
              << "    }\n"
              << AssignVal{ValArrItem(carryInValuesFirst, "idx", vBytes, k), ValVar("result"), vBytes} << ";\n"
              << "    previous_key = key;\n"
                 "}\n"
              << "lkeys[lid] = previous_key;\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("result"), vBytes} << ";\n"
              << "for (" << k.decl<uint_>("offset") << " = 1; "
              << "offset < wg_size; offset *= 2){\n"
                 "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if(lid >= offset){\n"
                 "        key = lkeys[lid - offset];\n"
              << "        if(previous_key == key){\n"
              << AssignVal{ValVar("value"), ValArrItem("lvals", "lid - offset", vBytes), vBytes} << ";\n"
              << reduceOp.Apply(ValVar("result"), ValVar("value"), ValVar("result")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("result"), vBytes} << ";\n"
              << "}\n"
              << "barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "if(lid > 0){\n"
              << "    previous_key = lkeys[lid - 1];\n"
              << AssignVal{ValVar("result"), ValArrItem("lvals", "lid - 1", vBytes), vBytes} << ";\n"
              << "}\n"
              << "for (idx = id; idx < id + values_per_work_item; idx += 1) {\n"
              << "    barrier( CLK_GLOBAL_MEM_FENCE );\n"
              << "    if (lid > 0 && idx < carry_out_size) {\n"
              << "        key = " << carryOutsKeysFirst[k.var<const uint_>("idx")] << ";\n"
              << AssignVal{ValVar("value"), ValArrItem(carryInValuesFirst, "idx", vBytes, k), vBytes} << ";\n"
              << "        if(previous_key == key){\n"
              << reduceOp.Apply(ValVar("result"), ValVar("value"), ValVar("value")) << ";\n"
              << "        }\n"
              << AssignVal{ValArrItem(carryInValuesFirst, "idx", vBytes, k), ValVar("value"), vBytes} << ";\n"
              << "    }\n"
              << "}\n";

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            kernel.set_arg(localKeysArg, compute::local_buffer<uint_>(workGroupSize));
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize * vBytes));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupSize,
                                          workGroupSize);
        }

        inline void FinalReduction(const std::vector<std::reference_wrapper<const boost::compute::vector<boost::compute::uint_>>> &keys,
                                   const boost::compute::vector<unsigned char>::iterator &valuesFirst,
                                   const std::vector<std::reference_wrapper<boost::compute::vector<boost::compute::uint_>>> &keysResult,
                                   const boost::compute::vector<unsigned char>::iterator &valuesResult,
                                   std::size_t count,
                                   const boost::compute::vector<boost::compute::uint_>::iterator &newKeysFirst,
                                   const boost::compute::vector<boost::compute::uint_>::iterator &carryInKeysFirst,
                                   const boost::compute::vector<unsigned char>::iterator &carryInValuesFirst,
                                   std::size_t workGroupSize,
                                   std::size_t vBytes,
                                   const std::string &reduceBody,
                                   boost::compute::command_queue &queue) {
            using namespace boost;
            using namespace detail::meta;
            using compute::uint_;

            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_final_reduction");
            k.add_set_arg<const uint_>("count", static_cast<uint_>(count));
            const std::size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            const std::size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");
            const std::size_t nKeys = keys.size();
            assert(nKeys == keysResult.size());
            FunctionApplication reduceOp(k, "spla_reduce", reduceBody, vBytes);

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              << k.decl<uint_>("key") << ";\n"
              << DeclareVal{"value", vBytes} << ";\n"
              << "if (gid < count) {\n"
              << k.var<uint_>("key") << " = " << newKeysFirst[k.var<const uint_>("gid")] << ";\n"
              << AssignVal{ValVar("value"), ValArrItem(valuesFirst, "gid", vBytes, k), vBytes} << ";\n"
              << "lkeys[lid] = key;\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("value"), vBytes} << ";\n"
              << "}\n"
              << DeclareVal{"result", vBytes} << ";\n"
              << AssignVal{ValVar("result"), ValVar("value"), vBytes} << ";\n"
              << k.decl<uint_>("other_key") << ";\n"
              << DeclareVal{"other_value", vBytes} << ";\n"
              << "for (" << k.decl<uint_>("offset") << " = 1; offset < wg_size ; offset *= 2) {\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if(lid >= offset) {\n"
              << "        other_key = lkeys[lid - offset];\n"
              << "        if(other_key == key){\n"
              << AssignVal{ValVar("other_value"), ValArrItem("lvals", "lid - offset", vBytes), vBytes} << ";\n"
              << reduceOp.Apply(ValVar("result"), ValVar("other_value"), ValVar("result")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("result"), vBytes} << ";\n"
              << "}\n"
              << "if (gid >= count) {\n return;\n};\n"
              << k.decl<const bool>("save") << " = (gid < (count - 1)) ?"
              << newKeysFirst[k.var<const uint_>("gid + 1")] << " != key : true;\n"
              << k.decl<uint_>("carry_in_key") << ";\n"
              << "if(group_id > 0 && save) {\n"
              << "    carry_in_key = " << carryInKeysFirst[k.var<const uint_>("group_id - 1")] << ";\n"
              << "    if(key == carry_in_key){\n"
              << AssignVal{ValVar("other_value"), ValArrItem(carryInValuesFirst, "group_id - 1", vBytes, k), vBytes} << ";\n"
              << reduceOp.Apply(ValVar("result"), ValVar("other_value"), ValVar("result")) << ";\n"
              << "    }\n"
              << "}\n"
              << "if (save) {\n"
              << AssignKey{KeyVec(keysResult, "key", k), KeyVec(keys, "gid", k), nKeys} << ";\n"
              << AssignVal{ValArrItem(valuesResult, "key", vBytes, k), ValVar("result"), vBytes} << ";\n"
              << "}\n";

            auto workGroupsNo = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(count) / static_cast<float>(workGroupSize)));

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            kernel.set_arg(localKeysArg, compute::local_buffer<uint_>(workGroupSize));
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize * vBytes));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * workGroupSize,
                                          workGroupSize);
        }

        /**
         * @details
         * 1. Keys are recalculated with @p GenerateUintKeys. It is done by
         *  comparison of each two adjustment keys and inclusive scan is performed
         *  on the resulting vector. As a bonus, we already know size of the resulting
         *  keys vector: it is the value of the last key + 1. @n
         * 2. For each work group carry-out value is calculated (it's done by key-oriented
         *  Hillis/Steele scan), where @n
         *      - Carry-out is a pair of the last key processed by work
         *       group and sum of all values under this key in work group. @n
         *      - Carry-in is a pair of the first key processed by work
         *       group and sum of all values under this key in work group. @n
         * 3. From every carry-out carry-in is calculated by performing inclusive scan
         *  by key. @n
         * 4. Final reduction by key is performed (key-oriented Hillis/Steele scan),
         *  carry-in values are added where needed.
         */
        inline std::size_t ReduceByKeyWithScan(const std::vector<std::reference_wrapper<const boost::compute::vector<boost::compute::uint_>>> &keys,
                                               const boost::compute::vector<unsigned char> &values,
                                               const std::vector<std::reference_wrapper<boost::compute::vector<boost::compute::uint_>>> &keysResult,
                                               boost::compute::vector<unsigned char> &valuesResult,
                                               std::size_t valueByteSize,
                                               const std::string &reduceBody,
                                               boost::compute::command_queue &queue) {
            using namespace boost;
            using namespace detail::meta;
            using compute::uint_;

            const compute::context &context = queue.get_context();
            const std::size_t count = keys.at(0).get().size();

            if (count == 0) {
                return 0;
            }

            const compute::device &device = queue.get_device();
            const std::size_t workGroupSize = device.get_info<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
            compute::vector<uint_> newKeys(count, context);
            GenerateUintKeys(keys, newKeys.begin(), workGroupSize, queue);

            const auto carryOutSize = static_cast<std::size_t>(
                    std::ceil(static_cast<float>(count) / static_cast<float>(workGroupSize)));
            compute::vector<uint_> carryOutKeys(carryOutSize, context);
            compute::vector<unsigned char> carryOutValues(carryOutSize * valueByteSize, context);
            CarryOuts(newKeys,
                      values,
                      carryOutKeys.begin(),
                      carryOutValues.begin(),
                      workGroupSize,
                      valueByteSize,
                      reduceBody,
                      queue);

            compute::vector<unsigned char> carryInValues(carryOutSize * valueByteSize, context);
            CarryIns(carryOutKeys.begin(),
                     carryOutValues.begin(),
                     carryInValues.begin(),
                     carryOutSize,
                     workGroupSize,
                     valueByteSize,
                     reduceBody,
                     queue);

            const std::size_t resultSize = compute::detail::read_single_value<uint_>(newKeys.get_buffer(), count - 1, queue) + 1;

            for (compute::vector<uint_> &kRes : keysResult) {
                kRes.resize(resultSize, queue);
            }
            valuesResult.resize(resultSize * valueByteSize, queue);

            FinalReduction(keys,
                           values.begin(),
                           keysResult,
                           valuesResult.begin(),
                           count,
                           newKeys.begin(),
                           carryOutKeys.begin(),
                           carryInValues.begin(),
                           workGroupSize,
                           valueByteSize,
                           reduceBody,
                           queue);

            return resultSize;
        }

    }// namespace detail

    /**
     * @brief The algorithm performs reduction for each contiguous
     * subsequence of aligned values determinate by equivalent keys. @n
     *
     * Aligned value - a sequence of bytes of size @p valueByteSize. Values must
     * be packed without offsets and paddings. @n
     *
     * Two keys are assumed to be equal if and only if two keys from
     * key sequences are match.
     *
     * @note Reduce function must be associative. Otherwise, reduction result
     * for values with the same keys is not defined.
     *
     * @note Output vectors (such as @p outputValues, @p outputIndices*)
     * are resized during @p ReduceByPairKey execution. Hence, there is no
     * need to resize them beforehand.
     *
     * @param inputIndices1 First sequence of keys
     * @param inputIndices2 Second sequence of keys
     * @param inputValues Sequence of bytes, where values are packed
     * @param outputIndices1 Output sequence of first keys
     * @param outputIndices2 Output sequence of second keys
     * @param outputValues Output sequence of value's bytes
     * @param valueByteSize Size of each value
     * @param reduceOp Body of the reduce function
     * @param queue OpenCL command queue
     * @return Size of the resulting keys sequence
     *
     * @relatesalso Look the @p ReduceByKeyWithScan for the implementation details
     */
    inline std::size_t ReduceByPairKey(const boost::compute::vector<unsigned int> &inputIndices1,
                                       const boost::compute::vector<unsigned int> &inputIndices2,
                                       const boost::compute::vector<unsigned char> &inputValues,
                                       boost::compute::vector<unsigned int> &outputIndices1,
                                       boost::compute::vector<unsigned int> &outputIndices2,
                                       boost::compute::vector<unsigned char> &outputValues,
                                       std::size_t valueByteSize,
                                       const std::string &reduceOp,
                                       boost::compute::command_queue &queue) {
        return detail::ReduceByKeyWithScan(
                std::vector{std::ref(inputIndices1), std::ref(inputIndices2)},
                inputValues,
                std::vector{std::ref(outputIndices1), std::ref(outputIndices2)},
                outputValues,
                valueByteSize,
                reduceOp,
                queue);
    }

    /**
     * @brief The algorithm performs reduction for each contiguous
     * subsequence of aligned values determinate by equivalent keys. @n
     *
     * Aligned value - a sequence of bytes of size @p valueByteSize. Values must
     * be packed without offsets and paddings.
     *
     * @note Reduce function must be associative. Otherwise, reduction result
     * for values with the same keys is not defined.
     *
     * @note Output vectors (such as @p outputValues, @p outputIndices)
     * are resized during @p ReduceByKey execution. Hence, there is no
     * need to resize them beforehand.
     *
     * @param inputIndices Vector of keys
     * @param inputValues Sequence of bytes, where values are packed
     * @param outputIndices Output sequence of keys
     * @param outputValues Output sequence of value's bytes
     * @param valueByteSize Size of each value
     * @param reduceOp Body of the reduce function
     * @param queue OpenCL command queue
     * @return Size of the resulting key sequence
     *
     * @relatesalso Look the @p ReduceByKeyWithScan for the implementation details
     */
    inline std::size_t ReduceByKey(const boost::compute::vector<unsigned int> &inputIndices,
                                   const boost::compute::vector<unsigned char> &inputValues,
                                   boost::compute::vector<unsigned int> &outputIndices,
                                   boost::compute::vector<unsigned char> &outputValues,
                                   std::size_t valueByteSize,
                                   const std::string &reduceOp,
                                   boost::compute::command_queue &queue) {
        return detail::ReduceByKeyWithScan(
                std::vector{std::ref(inputIndices)},
                inputValues,
                std::vector{std::ref(outputIndices)},
                outputValues,
                valueByteSize,
                reduceOp,
                queue);
    }

    /**
     * @brief The algorithm performs reduction of two input keys sequences
     * for each contiguous subsequence of equivalent keys.
     * Keys are considered to be equal iff the keys in corresponding
     * sequences are equal.
     *
     * @note Lengths of the sequences must be the same.
     *
     * @param inputIndices1 First sequence of keys
     * @param inputIndices2 Second sequence of keys
     * @param outputIndices1 Output sequence for the first sequence
     * @param outputIndices2 Output sequence for the second sequence
     * @param queue OpenCL command queue
     * @return Size of the resulting key sequence
     */
    inline std::size_t ReducePairKey(const boost::compute::vector<unsigned int> &inputIndices1,
                                     const boost::compute::vector<unsigned int> &inputIndices2,
                                     boost::compute::vector<unsigned int> &outputIndices1,
                                     boost::compute::vector<unsigned int> &outputIndices2,
                                     boost::compute::command_queue &queue) {
        using namespace boost;

        compute::context ctx = queue.get_context();
        assert(inputIndices1.size() == inputIndices2.size());
        const std::size_t inputSize = inputIndices1.size();

        if (inputSize <= 1) {
            outputIndices1.resize(inputSize, queue);
            outputIndices2.resize(inputSize, queue);
            compute::copy(inputIndices1.begin(), inputIndices1.end(), outputIndices1.begin(), queue);
            compute::copy(inputIndices2.begin(), inputIndices2.end(), outputIndices2.begin(), queue);
            return inputSize;
        }

        compute::vector<unsigned int> outputPos(inputSize, ctx);
        detail::GenerateUintKeys({std::ref(inputIndices1), std::ref(inputIndices2)},
                                 outputPos.begin(),
                                 queue.get_device().get_info<CL_DEVICE_MAX_WORK_GROUP_SIZE>(),
                                 queue);

        const std::size_t uniqueNum = (outputPos.end() - 1).read(queue) + 1;
        outputIndices1.resize(uniqueNum, queue);
        outputIndices2.resize(uniqueNum, queue);

        BOOST_COMPUTE_CLOSURE(void, copyToOutput, (unsigned int i), (inputIndices1, inputIndices2, outputIndices1, outputIndices2, outputPos),
                              {
                                  if (outputPos[i] != outputPos[i - 1]) {
                                      outputIndices1[outputPos[i]] = inputIndices1[i];
                                      outputIndices2[outputPos[i]] = inputIndices2[i];
                                  }
                              });

        compute::for_each_n(compute::counting_iterator(1), inputSize - 1, copyToOutput, queue);
        outputIndices1.begin().write(inputIndices1.begin().read(queue), queue);
        outputIndices2.begin().write(inputIndices2.begin().read(queue), queue);

        return uniqueNum;
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAREDUCEBYKEY_HPP