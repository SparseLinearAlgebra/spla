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

#include <boost/compute/algorithm/reduce_by_key.hpp>

namespace spla {

    namespace detail {

        using namespace boost;
        using compute::uint_;
        using MetaKernel = compute::detail::meta_kernel;

        struct DeclareMultiKey {
            const std::string Name;
            const std::size_t KeySize;
        };

        MetaKernel &operator<<(MetaKernel &k, const DeclareMultiKey &key) {
            for (std::size_t ki = 0; ki < key.KeySize; ++ki) {
                k << k.decl<uint_>(key.Name + std::to_string(ki)) << ";";
            }
            return k;
        }

        struct DeclareAlignedValue {
            const std::string Name;
            const std::size_t vBytes;
        };

        MetaKernel &operator<<(MetaKernel &k, const DeclareAlignedValue &v) {
            k << k.decl<unsigned char>(v.Name) << "[" << std::to_string(v.vBytes) << "];";
            return k;
        }

        class VMultiKey {
        public:
            [[nodiscard]] virtual std::string GetKeyByN(std::size_t n) const = 0;
        };

        class KeyVar : public VMultiKey {
        public:
            explicit KeyVar(std::string name) : mName(std::move(name)) {}

            [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
                return mName + std::to_string(n);
            }

        private:
            const std::string mName;
        };

        class KeyArrItem : public VMultiKey {
        public:
            explicit KeyArrItem(std::string arr, std::string idx) : mArr(std::move(arr)), mIdx(std::move(idx)) {}

            [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
                return mArr + std::to_string(n) + '[' + mIdx + ']';
            }

        private:
            const std::string mArr;
            const std::string mIdx;
        };

        class KeyVec : public VMultiKey {
        public:
            explicit KeyVec(
                    std::vector<compute::vector<uint_>::iterator> vec,
                    std::string idx,
                    MetaKernel &k)
                : mVec(std::move(vec)), mIdx(std::move(idx)), mK(k) {}

            [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
                return mK.template get_buffer_identifier<uint_>(mVec[n].get_buffer()) + '[' + mIdx + ']';
            }

        private:
            const std::vector<compute::vector<uint_>::iterator> mVec;
            const std::string mIdx;
            MetaKernel &mK;
        };

        class VAlignedValue {
        public:
            [[nodiscard]] virtual std::string GetByteByN(const std::string &n) const = 0;

            [[nodiscard]] virtual std::string GetPointer() const = 0;
        };

        class ValVar : public VAlignedValue {
        public:
            explicit ValVar(std::string name) : mName(std::move(name)) {}

            [[nodiscard]] std::string GetByteByN(const std::string &n) const override {
                return mName + '[' + n + ']';
            }

            [[nodiscard]] std::string GetPointer() const override {
                return std::string("&") + mName;
            }

        private:
            std::string mName;
        };

        class ValArrItem : public VAlignedValue {
        public:
            explicit ValArrItem(std::string arr, std::string idx, std::size_t vBytes)
                : mArr(std::move(arr)), mIdx(std::move(idx)), mVBytes(vBytes) {}

            explicit ValArrItem(const compute::vector<unsigned char>::iterator &it, std::string idx, std::size_t vBytes, MetaKernel &k)
                : mArr(k.get_buffer_identifier<unsigned char>(it.get_buffer())),
                  mIdx(std::move(idx)), mVBytes(vBytes) {}

            [[nodiscard]] std::string GetByteByN(const std::string &n) const override {
                std::stringstream ss;
                ss << mArr << '[' << mIdx << " * " << mVBytes << " + " << n << ']';
                return ss.str();
            }

            [[nodiscard]] std::string GetPointer() const override {
                std::stringstream ss;
                ss << '&' << mArr << '[' << mIdx << " * " << mVBytes << ']';
                return ss.str();
            }

        private:
            std::string mArr;
            std::string mIdx;
            std::size_t mVBytes;
        };

        struct AssignKey {
            const VMultiKey &Left, &Right;
            const std::size_t KeySize;
        };

        MetaKernel &operator<<(MetaKernel &k, const AssignKey &key) {
            for (std::size_t ki = 0; ki < key.KeySize; ++ki) {
                std::string ind = std::to_string(ki);
                k << key.Left.GetKeyByN(ki) << " = " << key.Right.GetKeyByN(ki) << ";";
            }
            return k;
        }

        struct CompareKey {
            const VMultiKey &Left, &Right;
            const std::size_t KeySize;
        };

        MetaKernel &operator<<(MetaKernel &k, const CompareKey &comp) {
            for (std::size_t ki = 0; ki < comp.KeySize; ++ki) {
                std::string ind = std::to_string(ki);
                k << '(' << comp.Left.GetKeyByN(ki) << " == " << comp.Right.GetKeyByN(ki) << ")";
                if (ki + 1 < comp.KeySize) {
                    k << " && ";
                }
            }
            return k;
        }

        struct AssignVal {
            const VAlignedValue &Left, &Right;
            const std::size_t vBytes;
        };

        MetaKernel &operator<<(MetaKernel &k, const AssignVal &v) {
            k << "for (uint byte_i = 0; byte_i < " << std::to_string(v.vBytes) << "; byte_i++) {\n"
              << v.Left.GetByteByN("byte_i") << " = " << v.Right.GetByteByN("byte_i") << ";\n"
              << "}\n";
            return k;
        }

        class ReduceApplication {
        public:
            explicit ReduceApplication(std::string opName,
                                       const VAlignedValue &a,
                                       const VAlignedValue &b,
                                       const VAlignedValue &c)
                : mOpName(std::move(opName)), mA(a), mB(b), mC(c) {}

            virtual void Print(MetaKernel &) const = 0;

        protected:
            const std::string mOpName;
            const VAlignedValue &mA, &mB, &mC;
        };

        class ReduceApplicationNonRestrict : public ReduceApplication {
        private:
            static inline const char *REDUCE_RESULT_VAR = "reduce_result";

        public:
            explicit ReduceApplicationNonRestrict(std::string opName,
                                                  const VAlignedValue &a,
                                                  const VAlignedValue &b,
                                                  const VAlignedValue &c,
                                                  std::size_t vBytes)
                : ReduceApplication(std::move(opName), a, b, c), mVBytes(vBytes) {}

            void Print(MetaKernel &k) const override {
                k << "{\n"
                  << DeclareAlignedValue{REDUCE_RESULT_VAR, mVBytes} << ";\n"
                  << mOpName << '(' << mA.GetPointer() << ", " << mB.GetPointer() << ", " << REDUCE_RESULT_VAR << ')' << ";\n"
                  << AssignVal{mC, ValVar(REDUCE_RESULT_VAR), mVBytes} << ";\n"
                  << "}\n";
            }

        private:
            const std::size_t mVBytes;
        };

        class ReduceApplicationRestrict : public ReduceApplication {
        public:
            using ReduceApplication::ReduceApplication;

            void Print(MetaKernel &k) const override {
                k << mOpName << '(' << mA.GetPointer() << ", " << mB.GetPointer() << ", " << mC.GetPointer() << ')' << ";\n";
            }
        };

        MetaKernel &operator<<(MetaKernel &k, const std::shared_ptr<ReduceApplication> &reduce) {
            reduce->Print(k);
            return k;
        }

        class ReduceOp {
        public:
            explicit ReduceOp(MetaKernel &k,
                              std::string reduceOpName,
                              const std::string &body,
                              std::size_t vBytes)
                : mReduceOpName(std::move(reduceOpName)),
                  mVBytes(vBytes) {
                std::stringstream spla_reduce_op;
                spla_reduce_op << "void " << mReduceOpName << "(__global void* vp_a, __global void* vp_b, __global void* vp_c) {\n"
                               << "#define _ACCESS_A __global\n"
                               << "#define _ACCESS_B __global\n"
                               << "#define _ACCESS_C __global\n"
                               << "   " << body << "\n"
                               << "#undef _ACCESS_A\n"
                               << "#undef _ACCESS_B\n"
                               << "#undef _ACCESS_C\n"
                               << "}";
                k.add_function(mReduceOpName, spla_reduce_op.str());
            }

            std::shared_ptr<ReduceApplication> Apply(const VAlignedValue &left,
                                                     const VAlignedValue &right,
                                                     const VAlignedValue &result) {
                std::string lPtr = left.GetPointer();
                std::string rPtr = right.GetPointer();
                std::string resPtr = result.GetPointer();
                if (lPtr != resPtr && rPtr != resPtr) {
                    return std::make_shared<ReduceApplicationRestrict>(mReduceOpName, left, right, result);
                }
                return std::make_shared<ReduceApplicationNonRestrict>(mReduceOpName, left, right, result, mVBytes);
            }

        private:
            const std::string mReduceOpName;
            const std::size_t mVBytes;
        };

        inline void CarryOuts(const std::vector<compute::vector<uint_>::iterator> &keysFirst,
                              const compute::vector<unsigned char>::iterator &values,
                              std::size_t count,
                              const std::vector<compute::vector<uint_>::iterator> &carryOutsKeysFirst,
                              const compute::vector<unsigned char>::iterator &carryOutValuesFirst,
                              std::size_t vBytes,
                              std::size_t workGroupSize,
                              const std::string &reduceBody,
                              compute::command_queue &queue) {
            const std::size_t nKeys = keysFirst.size();

            compute::detail::meta_kernel k("reduce_by_key_with_scan_carry_outs");
            k.add_set_arg<const uint_>("count", uint_(count));
            std::vector<size_t> localKeysArg(nKeys);
            const std::string vBytesString = std::to_string(vBytes);
            for (std::size_t ki = 0; ki < nKeys; ++ki) {
                localKeysArg[ki] = k.add_arg<uint_ *>(compute::memory_object::local_memory, std::string("lkeys") + std::to_string(ki));
            }
            size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");
            const std::string valuesBufferName = k.get_buffer_identifier<unsigned char>(values.get_buffer());

            ReduceOp reduce(k, "_spla_reduce_op", reduceBody, vBytes);

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              << DeclareMultiKey{"key", nKeys} << ";\n"
              << DeclareAlignedValue{"value", vBytes} << ";\n"
              << "if (gid < count) {\n"
              << AssignKey{KeyVar("key"), KeyVec(keysFirst, "gid", k), nKeys} << ";\n"
              << AssignVal{ValVar("value"), ValArrItem(valuesBufferName, "gid", vBytes), vBytes} << ";\n"
              << AssignKey{KeyArrItem("lkeys", "lid"), KeyVar("key"), nKeys} << ";\n"
              << AssignVal{ValArrItem("lvals", "lid", vBytes), ValVar("value"), vBytes} << ";\n"
              << "}\n"
              << DeclareAlignedValue{"result", vBytes} << AssignVal{ValVar("result"), ValVar("value"), vBytes} << ";\n"
              << DeclareMultiKey{"other_key", nKeys} << ";\n"
              << DeclareAlignedValue{"other_value", vBytes} << ";\n"
              << "for(" << k.decl<uint_>("offset") << " = 1; offset < wg_size; offset *= 2){\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if(lid >= offset){\n"
              << AssignKey{KeyVar("other_key"), KeyArrItem("lkeys", "lid - offset"), nKeys} << ";\n"
              << "        if(" << CompareKey{KeyVar("other_key"), KeyVar("key"), nKeys} << "){\n"
              << AssignVal{ValVar("other_value"), ValArrItem("lvals", "lid - offset", vBytes), vBytes} << ";\n"
              << reduce.Apply(ValVar("result"), ValVar("other_value"), ValVar("result")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << AssignKey{KeyArrItem("lvals", "lid"), KeyVar("result"), nKeys}
              << "}\n"
              << "if(lid == (wg_size - 1)){\n"
              << AssignKey{KeyVec(carryOutsKeysFirst, "group_id", k), KeyVar("key"), nKeys} << ";\n"
              << AssignVal{ValArrItem(carryOutValuesFirst, "group_id", vBytes, k), ValVar("result"), vBytes} << ";\n"
              << "}\n";

            auto work_groups_no = static_cast<size_t>(
                    std::ceil(static_cast<float>(count) / static_cast<float>(workGroupSize)));

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            for (std::size_t ki = 0; ki < nKeys; ++ki) {
                kernel.set_arg(localKeysArg[ki], compute::local_buffer<uint_>(workGroupSize));
            }
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          work_groups_no * workGroupSize,
                                          workGroupSize);
        }

        template<class OutputValueIterator, class BinaryFunction>
        inline void carry_ins(compute::vector<uint_>::iterator carryOutsKeysFirst,
                              OutputValueIterator carryOutValuesFirst,
                              OutputValueIterator carry_in_values_first,
                              size_t carry_out_size,
                              BinaryFunction function,
                              size_t workGroupSize,
                              compute::command_queue &queue) {
            typedef typename std::iterator_traits<OutputValueIterator>::value_type value_out_type;

            auto values_pre_work_item = static_cast<uint_>(
                    std::ceil(float(carry_out_size) / workGroupSize));

            compute::detail::meta_kernel k("reduce_by_key_with_scan_carry_ins");
            k.add_set_arg<const uint_>("carry_out_size", uint_(carry_out_size));
            k.add_set_arg<const uint_>("values_per_work_item", values_pre_work_item);
            size_t local_keys_arg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            size_t local_vals_arg = k.add_arg<value_out_type *>(compute::memory_object::local_memory, "lvals");

            k << k.decl<uint_>("id") << " = get_global_id(0) * values_per_work_item;\n"
              << k.decl<uint_>("idx") << " = id;\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              <<

                    k.decl<uint_>("key") << ";\n"
              << k.decl<value_out_type>("value") << ";\n"
              << k.decl<uint_>("previous_key") << ";\n"
              << k.decl<value_out_type>("result") << ";\n"
              <<

                    "if(id < carry_out_size){\n"
              << k.var<uint_>("previous_key") << " = " << carryOutsKeysFirst[k.var<const uint_>("id")] << ";\n"
              << k.var<value_out_type>("result") << " = " << carryOutValuesFirst[k.var<const uint_>("id")] << ";\n"
              << carry_in_values_first[k.var<const uint_>("id")] << " = result;\n"
              << "}\n"
              <<

                    k.decl<const uint_>("end") << " = (id + values_per_work_item) <= carry_out_size"
              << " ? (values_per_work_item + id) :  carry_out_size;\n"
              <<

                    "for(idx = idx + 1; idx < end; idx += 1){\n"
              << "    key = " << carryOutsKeysFirst[k.var<const uint_>("idx")] << ";\n"
              << "    value = " << carryOutValuesFirst[k.var<const uint_>("idx")] << ";\n"
              << "    if(previous_key == key){\n"
              << "        result = " << function(k.var<value_out_type>("result"), k.var<value_out_type>("value")) << ";\n"
              << "    }\n else { \n"
              << "        result = value;\n"
                 "    }\n"
              << "    " << carry_in_values_first[k.var<const uint_>("idx")] << " = result;\n"
              << "    previous_key = key;\n"
                 "}\n"
              <<

                    // save the last key and result to local memory
                    "lkeys[lid] = previous_key;\n"
              << "lvals[lid] = result;\n"
              <<

                    // Hillis/Steele scan
                    "for(" << k.decl<uint_>("offset") << " = 1; "
              << "offset < wg_size; offset *= 2){\n"
                 "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if(lid >= offset){\n"
                 "        key = lkeys[lid - offset];\n"
              << "        if(previous_key == key){\n"
              << "            value = lvals[lid - offset];\n"
              << "            result = " << function(k.var<value_out_type>("result"), k.var<value_out_type>("value")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    lvals[lid] = result;\n"
              << "}\n"
              << "barrier(CLK_LOCAL_MEM_FENCE);\n"
              <<

                    "if(lid > 0){\n"
              <<
                    // load key-value reduced by previous work item
                    "    previous_key = lkeys[lid - 1];\n"
              << "    result       = lvals[lid - 1];\n"
              << "}\n"
              <<

                    // add key-value reduced by previous work item
                    "for(idx = id; idx < id + values_per_work_item; idx += 1){\n"
              <<
                    // make sure all carry-ins are saved in global memory
                    "    barrier( CLK_GLOBAL_MEM_FENCE );\n"
              << "    if(lid > 0 && idx < carry_out_size) {\n"
                 "        key = "
              << carryOutsKeysFirst[k.var<const uint_>("idx")] << ";\n"
              << "        value = " << carry_in_values_first[k.var<const uint_>("idx")] << ";\n"
              << "        if(previous_key == key){\n"
              << "            value = " << function(k.var<value_out_type>("result"), k.var<value_out_type>("value")) << ";\n"
              << "        }\n"
              << "        " << carry_in_values_first[k.var<const uint_>("idx")] << " = value;\n"
              << "    }\n"
              << "}\n";


            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            kernel.set_arg(local_keys_arg, compute::local_buffer<uint_>(workGroupSize));
            kernel.set_arg(local_vals_arg, compute::local_buffer<value_out_type>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupSize,
                                          workGroupSize);
        }

        template<class InputKeyIterator, class InputValueIterator,
                 class OutputKeyIterator, class OutputValueIterator,
                 class BinaryFunction>
        inline void final_reduction(InputKeyIterator keys_first,
                                    InputValueIterator values_first,
                                    OutputKeyIterator keys_result,
                                    OutputValueIterator values_result,
                                    size_t count,
                                    BinaryFunction function,
                                    compute::vector<uint_>::iterator new_keys_first,
                                    compute::vector<uint_>::iterator carry_in_keys_first,
                                    OutputValueIterator carry_in_values_first,
                                    size_t carry_in_size,
                                    size_t workGroupSize,
                                    compute::command_queue &queue) {
            typedef typename std::iterator_traits<OutputValueIterator>::value_type value_out_type;

            compute::detail::meta_kernel k("reduce_by_key_with_scan_final_reduction");
            k.add_set_arg<const uint_>("count", uint_(count));
            size_t local_keys_arg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            size_t local_vals_arg = k.add_arg<value_out_type *>(compute::memory_object::local_memory, "lvals");

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              <<

                    k.decl<uint_>("key") << ";\n"
              << k.decl<value_out_type>("value") << ";\n"

                                                    "if(gid < count){\n"
              << k.var<uint_>("key") << " = " << new_keys_first[k.var<const uint_>("gid")] << ";\n"
              << k.var<value_out_type>("value") << " = " << values_first[k.var<const uint_>("gid")] << ";\n"
              << "lkeys[lid] = key;\n"
              << "lvals[lid] = value;\n"
              << "}\n"
              <<

                    // Hillis/Steele scan
                    k.decl<value_out_type>("result") << " = value;\n"
              << k.decl<uint_>("other_key") << ";\n"
              << k.decl<value_out_type>("other_value") << ";\n"
              <<

                    "for(" << k.decl<uint_>("offset") << " = 1; "
              << "offset < wg_size ; offset *= 2){\n"
                 "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    if(lid >= offset) {\n"
              << "        other_key = lkeys[lid - offset];\n"
              << "        if(other_key == key){\n"
              << "            other_value = lvals[lid - offset];\n"
              << "            result = " << function(k.var<value_out_type>("result"), k.var<value_out_type>("other_value")) << ";\n"
              << "        }\n"
              << "    }\n"
              << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
              << "    lvals[lid] = result;\n"
              << "}\n"
              <<

                    "if(gid >= count) {\n return;\n};\n"
              <<

                    k.decl<const bool>("save") << " = (gid < (count - 1)) ?"
              << new_keys_first[k.var<const uint_>("gid + 1")] << " != key"
              << ": true;\n"
              <<

                    // Add carry in
                    k.decl<uint_>("carry_in_key") << ";\n"
              << "if(group_id > 0 && save) {\n"
              << "    carry_in_key = " << carry_in_keys_first[k.var<const uint_>("group_id - 1")] << ";\n"
              << "    if(key == carry_in_key){\n"
              << "        other_value = " << carry_in_values_first[k.var<const uint_>("group_id - 1")] << ";\n"
              << "        result = " << function(k.var<value_out_type>("result"), k.var<value_out_type>("other_value")) << ";\n"
              << "    }\n"
              << "}\n"
              <<

                    // Save result only if the next key is different or it's the last element.
                    "if(save){\n"
              << keys_result[k.var<uint_>("key")] << " = " << keys_first[k.var<const uint_>("gid")] << ";\n"
              << values_result[k.var<uint_>("key")] << " = result;\n"
              << "}\n";

            auto work_groups_no = static_cast<size_t>(
                    std::ceil(float(count) / workGroupSize));

            const compute::context &context = queue.get_context();
            compute::kernel kernel = k.compile(context);
            kernel.set_arg(local_keys_arg, compute::local_buffer<uint_>(workGroupSize));
            kernel.set_arg(local_vals_arg, compute::local_buffer<value_out_type>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          work_groups_no * workGroupSize,
                                          workGroupSize);
        }

        template<class InputKeyIterator, class InputValueIterator,
                 class OutputKeyIterator, class OutputValueIterator,
                 class BinaryFunction, class BinaryPredicate>
        inline size_t reduce_by_key_with_scan(InputKeyIterator keys_first,
                                              InputKeyIterator keys_last,
                                              InputValueIterator values_first,
                                              OutputKeyIterator keys_result,
                                              OutputValueIterator values_result,
                                              BinaryFunction function,
                                              BinaryPredicate predicate,
                                              compute::command_queue &queue) {
            typedef typename std::iterator_traits<InputValueIterator>::value_type value_type;
            typedef typename std::iterator_traits<InputKeyIterator>::value_type key_type;
            typedef typename std::iterator_traits<OutputValueIterator>::value_type value_out_type;

            const compute::context &context = queue.get_context();
            size_t count = compute::detail::iterator_range_size(keys_first, keys_last);

            if (count == 0) {
                return size_t(0);
            }

            const compute::device &device = queue.get_device();
            size_t workGroupSize = get_work_group_size<value_type, key_type>(device);

            // Replace original key with unsigned integer keys generated based on given
            // predicate. New key is also an index for keys_result and values_result vectors,
            // which points to place where reduced value should be saved.
            compute::vector<uint_> new_keys(count, context);
            compute::vector<uint_>::iterator new_keys_first = new_keys.begin();
            generate_uint_keys(keys_first, count, predicate, new_keys_first,
                               workGroupSize, queue);

            // Calculate carry-out and carry-in vectors size
            const auto carry_out_size = static_cast<size_t>(
                    std::ceil(float(count) / workGroupSize));
            compute::vector<uint_> carry_out_keys(carry_out_size, context);
            compute::vector<value_out_type> carry_out_values(carry_out_size, context);
            carry_outs(new_keys_first, values_first, count, carry_out_keys.begin(),
                       carry_out_values.begin(), function, workGroupSize, queue);

            compute::vector<value_out_type> carry_in_values(carry_out_size, context);
            carry_ins(carry_out_keys.begin(), carry_out_values.begin(),
                      carry_in_values.begin(), carry_out_size, function, workGroupSize,
                      queue);

            final_reduction(keys_first, values_first, keys_result, values_result,
                            count, function, new_keys_first, carry_out_keys.begin(),
                            carry_in_values.begin(), carry_out_size, workGroupSize,
                            queue);

            const size_t result = compute::detail::read_single_value<uint_>(new_keys.get_buffer(),
                                                                            count - 1, queue);
            return result + 1;
        }

    }// namespace detail

    std::ptrdiff_t ReduceByKey(const boost::compute::vector<unsigned int> &inputIndices1,
                               const boost::compute::vector<unsigned int> &inputIndices2,
                               const boost::compute::vector<unsigned char> &inputValues,
                               boost::compute::vector<unsigned int> &outputIndices1,
                               boost::compute::vector<unsigned int> &outputIndices2,
                               boost::compute::vector<unsigned char> &outputValues,
                               std::size_t elementsInSequence,
                               const std::string &reduceOp,
                               boost::compute::command_queue &queue) {
        return 0;
    }

    std::ptrdiff_t ReduceByKey(const boost::compute::vector<unsigned int> &inputIndices,
                               const boost::compute::vector<unsigned char> &inputValues,
                               boost::compute::vector<unsigned int> &outputIndices,
                               boost::compute::vector<unsigned char> &outputValues,
                               std::size_t elementsInSequence,
                               const std::string &reduceOp,
                               boost::compute::command_queue &queue) {
        return 0;
    }

}// namespace spla

#endif//SPLA_SPLAREDUCEBYKEY_HPP
