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

        struct DeclareVal {
            const std::string Name;
            const std::size_t vBytes;
        };

        MetaKernel &operator<<(MetaKernel &k, const DeclareVal &v) {
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
                    const std::vector<compute::vector<uint_>> &vectors,
                    std::string idx,
                    MetaKernel &k)
                : mIdx(std::move(idx)) {
                mBuffersNames.reserve(vectors.size());
                for (const compute::vector<uint_> &v : vectors) {
                    mBuffersNames.push_back(k.template get_buffer_identifier<uint_>(v.get_buffer()));
                }
            }

            explicit KeyVec(
                    const std::vector<compute::vector<uint_>::iterator> &iterators,
                    std::string idx,
                    MetaKernel &k)
                : mIdx(std::move(idx)) {
                mBuffersNames.reserve(iterators.size());
                for (const compute::vector<uint_>::iterator &it : iterators) {
                    mBuffersNames.push_back(k.template get_buffer_identifier<uint_>(it.get_buffer()));
                }
            }

            [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
                return mBuffersNames[n] + '[' + mIdx + ']';
            }

        private:
            std::vector<std::string> mBuffersNames;
            const std::string mIdx;
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
                ss << mArr << '[' << mIdx << " * " << mVBytes << " + (" << n << ")]";
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
                  << DeclareVal{REDUCE_RESULT_VAR, mVBytes} << ";\n"
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

        inline void GenerateUintKeys(const std::vector<compute::vector<unsigned int>> &keysFirst,
                                     const compute::vector<unsigned int>::iterator &newKeysFirst,
                                     std::size_t preferredWorkGroupSize,
                                     compute::command_queue &queue) {
            compute::detail::meta_kernel k("spla_reduce_by_key_new_key_flags");
            const std::size_t nKeys = keysFirst.at(0).size();
            k.add_set_arg<const uint_>("count", uint_(nKeys));

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
                    std::ceil(static_cast<float>(nKeys) / static_cast<float>(preferredWorkGroupSize)));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * preferredWorkGroupSize,
                                          preferredWorkGroupSize);

            inclusive_scan(newKeysFirst, newKeysFirst + static_cast<std::ptrdiff_t>(nKeys),
                           newKeysFirst, queue);
        }

        inline void CarryOuts(const compute::vector<uint_> &keys,
                              const compute::vector<unsigned char> &values,
                              const compute::vector<uint_>::iterator &carryOutsKeysFirst,
                              const compute::vector<unsigned char>::iterator &carryOutValuesFirst,
                              std::size_t workGroupSize,
                              std::size_t vBytes,
                              const std::string &reduceBody,
                              compute::command_queue &queue) {
            compute::vector<uint_>::iterator keysFirst = keys.begin();
            compute::vector<unsigned char>::iterator valuesFirst = values.begin();
            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_carry_outs");
            k.add_set_arg<const uint_>("count", static_cast<uint_>(keys.size()));
            size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            size_t localVarsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");

            ReduceOp reduceOp(k, "spla_reduce", reduceBody, vBytes);

            k << k.decl<const uint_>("gid") << " = get_global_id(0);\n"
              << k.decl<const uint_>("wg_size") << " = get_local_size(0);\n"
              << k.decl<const uint_>("lid") << " = get_local_id(0);\n"
              << k.decl<const uint_>("group_id") << " = get_group_id(0);\n"
              << k.decl<uint_>("key") << ";\n"
              << DeclareVal{"value", vBytes} << ";\n"
              << "if(gid < count){\n"
              << k.var<uint_>("key") << " = " << keysFirst[k.var<const uint_>("gid")] << ";\n"
              << AssignVal{ValVar("Value"), ValArrItem(valuesFirst, "gid", vBytes, k), vBytes} << ";\n"
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
              << "            other_value = lvals[lid - offset];\n"
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
            kernel.set_arg(localVarsArg, compute::local_buffer<unsigned char>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * workGroupSize,
                                          workGroupSize);
        }

        inline void CarryIns(const compute::vector<uint_>::iterator &carryOutsKeysFirst,
                             const compute::vector<unsigned char>::iterator &carryOutValuesFirst,
                             const compute::vector<unsigned char>::iterator &carryInValuesFirst,
                             std::size_t carryOutSize,
                             std::size_t workGroupSize,
                             std::size_t vBytes,
                             const std::string &reduceBody,
                             compute::command_queue &queue) {
            auto valuesPreWorkItem = static_cast<uint_>(
                    std::ceil(static_cast<float>(carryOutSize) / static_cast<float>(workGroupSize)));

            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_carry_ins");
            k.add_set_arg<const uint_>("carry_out_size", uint_(carryOutSize));
            k.add_set_arg<const uint_>("values_per_work_item", valuesPreWorkItem);
            std::size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            std::size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");

            ReduceOp reduceOp(k, "spla_reduce", reduceBody, vBytes);

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
              << "lvals[lid] = result;\n"
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
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupSize,
                                          workGroupSize);
        }

        inline void FinalReduction(const std::vector<compute::vector<uint_>> &keys,
                                   const compute::vector<unsigned char>::iterator &valuesFirst,
                                   const std::vector<compute::vector<uint_>::iterator> &keysResult,
                                   const compute::vector<unsigned char>::iterator &valuesResult,
                                   std::size_t count,
                                   const compute::vector<uint_>::iterator &newKeysFirst,
                                   const compute::vector<uint_>::iterator &carryInKeysFirst,
                                   const compute::vector<unsigned char>::iterator &carryInValuesFirst,
                                   std::size_t workGroupSize,
                                   std::size_t vBytes,
                                   const std::string &reduceBody,
                                   compute::command_queue &queue) {
            compute::detail::meta_kernel k("spla_reduce_by_key_with_scan_final_reduction");
            k.add_set_arg<const uint_>("count", uint_(count));
            const std::size_t localKeysArg = k.add_arg<uint_ *>(compute::memory_object::local_memory, "lkeys");
            const std::size_t localValsArg = k.add_arg<unsigned char *>(compute::memory_object::local_memory, "lvals");
            const std::size_t nKeys = keys.size();
            assert(nKeys == keysResult.size());
            ReduceOp reduceOp(k, "spla_reduce", reduceBody, vBytes);

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
              << "lvals[lid] = value;\n"
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
              << newKeysFirst[k.var<const uint_>("gid + 1")] << " != key"
              << ": true;\n"
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
            kernel.set_arg(localValsArg, compute::local_buffer<unsigned char>(workGroupSize));

            queue.enqueue_1d_range_kernel(kernel,
                                          0,
                                          workGroupsNo * workGroupSize,
                                          workGroupSize);
        }

        inline std::size_t ReduceByKeyWithScanImpl(const std::vector<compute::vector<uint_>> &keys,
                                                   const compute::vector<unsigned char> &values,
                                                   const std::vector<compute::vector<uint_>::iterator> &keysResult,
                                                   const compute::vector<unsigned char> &valuesResult,
                                                   std::size_t valueByteSize,
                                                   const std::string &reduceBody,
                                                   compute::command_queue &queue) {
            const compute::context &context = queue.get_context();
            const std::size_t count = keys.at(0).size();
            const std::size_t nKeys = keys.size();

            if (count == 0) {
                return 0;
            }

            const compute::device &device = queue.get_device();
            const std::size_t workGroupSize = device.get_info<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
            compute::vector<uint_> newKeys(count, context);
            GenerateUintKeys(keys, newKeys.begin(), workGroupSize, queue);

            // Calculate carry-out and carry-in vectors size
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

            compute::vector<unsigned char> carryInValues(carryOutSize, context);
            CarryIns(carryOutKeys.begin(),
                     carryOutValues.begin(),
                     carryInValues.begin(),
                     carryOutSize,
                     workGroupSize,
                     valueByteSize,
                     reduceBody,
                     queue);

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

            const size_t result = compute::detail::read_single_value<uint_>(newKeys.get_buffer(),
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
