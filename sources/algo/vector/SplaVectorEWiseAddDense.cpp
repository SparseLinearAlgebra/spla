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

#include <algo/vector/SplaVectorEWiseAddDense.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/metautil/SplaMetaUtil.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaVectorCOO.hpp>
#include <storage/block/SplaVectorDense.hpp>
#include <utils/SplaProfiling.hpp>

bool spla::VectorEWiseAddDense::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVectorEWiseAdd *>(&params);

    return p &&
           (p->mask.Is<VectorCOO>() || p->mask.Is<VectorDense>()) &&// mask is coo or dense
           (p->a.Is<VectorCOO>() || p->a.Is<VectorDense>()) &&      // a is coo or dense
           (p->b.Is<VectorCOO>() || p->b.Is<VectorDense>()) &&      // b is coo or dense
           (p->a.Is<VectorDense>() || p->b.Is<VectorDense>());      // a or b is dense
}

void spla::VectorEWiseAddDense::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    PF_SCOPE(eadd, "-veadd-dense-");

    auto p = dynamic_cast<ParamsVectorEWiseAdd *>(&params);
    auto w = p->w;
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;
    auto complement = desc->IsParamSet(Descriptor::Param::MaskComplement);

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue);

    auto op = p->op;
    auto &type = p->type;
    auto byteSize = type->GetByteSize();
    auto typeHasValues = type->HasValues();

    if (p->a.IsNull() && p->b.IsNull())
        return;
    if (p->hasMask && !complement && p->mask.IsNull())
        return;

    auto nrows = p->a.IsNull() ? p->b->GetNrows() : p->a->GetNrows();

    compute::vector<Index> resultMask(nrows, ctx);
    compute::vector<unsigned char> resultVals(ctx);

    auto argDense = p->a.Cast<VectorDense>();
    auto argOther = p->b;

    // argument A either null or not-null (but in coo)
    if (argDense.IsNull()) {
        argOther = p->a;
        argDense = p->b.Cast<VectorDense>();
    }

    assert(argDense.IsNotNull());
    CHECK_RAISE_ERROR(argDense.IsNotNull(), InvalidState, "No not-null dense argument is not allowed. "
                                                          "Check AlgorithmManager.cpp:50 for correct registration order.");

    PF_SCOPE_MARK(eadd, "setup");

    // Initialize result from one of 2 args, where 1 is required to dense
    compute::copy(argDense->GetMask().begin(), argDense->GetMask().end(), resultMask.begin(), queue);
    if (typeHasValues) {
        resultVals.resize(nrows * byteSize, queue);
        compute::copy(argDense->GetVals().begin(), argDense->GetVals().end(), resultVals.begin(), queue);
    }

    PF_SCOPE_MARK(eadd, "setup arg 1");

    // Copy second argument
    if (argOther.IsNotNull()) {
        assert(argOther->GetFormat() == VectorBlock::Format::COO || argOther->GetFormat() == VectorBlock::Format::Dense);
        CHECK_RAISE_ERROR(argOther->GetFormat() == VectorBlock::Format::COO || argOther->GetFormat() == VectorBlock::Format::Dense,
                          NotImplemented, "Only COO and Dense vectors are supported");

        if (argOther->GetFormat() == VectorBlock::Format::COO) {
            auto block = argOther.Cast<VectorCOO>();
            assert(block.IsNotNull());

            // If no values present, only mask update
            if (!typeHasValues) {
                BOOST_COMPUTE_CLOSURE(void, addOnlyMaskSparse, (unsigned int rowId), (resultMask), { resultMask[rowId] = 1u; });
                compute::for_each(block->GetRows().begin(), block->GetRows().end(), addOnlyMaskSparse, queue);
            } else {
                compute::detail::meta_kernel kernel("__spla_vec_eadd_dense_sparse");
                auto argCount = kernel.add_arg<cl_uint>("count");
                auto argInputRows = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "inputRows");
                auto argInputVals = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "inputVals");
                auto argResultMask = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "resultMask");
                auto argResultVals = kernel.add_arg<cl_uchar *>(compute::memory_object::global_memory, "resultVals");

                std::string addFunc("add_func");
                kernel.add_function(addFunc,
                                    detail::meta::MakeFunction(addFunc, op->GetSource(),
                                                               detail::meta::Visibility::Global,
                                                               detail::meta::Visibility::Global,
                                                               detail::meta::Visibility::Global));

                kernel << "#define BYTE_SIZE " << static_cast<cl_uint>(byteSize) << "\n"
                       << "const uint i = get_global_id(0);\n"
                       << "if (i < count) {\n"
                       << "    const uint src = i * BYTE_SIZE;\n"
                       << "    const uint dst = inputRows[i] * BYTE_SIZE;\n"
                       << "    resultMask[inputRows[i]] |= 1u;"
                       << "    " << addFunc << "(&resultVals[dst], &inputVals[src], &resultVals[dst]);\n"
                       << "}\n";

                std::size_t tpb = 64;
                std::size_t workSize = compute::detail::calculate_work_size(block->GetNvals(), 1, tpb);

                auto compiledKernel = kernel.compile(ctx);
                compiledKernel.set_arg(argCount, static_cast<cl_uint>(block->GetNvals()));
                compiledKernel.set_arg(argInputRows, block->GetRows().get_buffer());
                compiledKernel.set_arg(argInputVals, block->GetVals().get_buffer());
                compiledKernel.set_arg(argResultMask, resultMask.get_buffer());
                compiledKernel.set_arg(argResultVals, resultVals.get_buffer());
                queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
            }
        } else {
            auto block = argOther.Cast<VectorDense>();
            assert(block.IsNotNull());
            assert(block->GetNrows() == nrows);

            // If no values present, only mask update
            if (!typeHasValues) {
                auto &stencil = block->GetMask();
                BOOST_COMPUTE_CLOSURE(void, addOnlyMaskDesne, (unsigned int i), (resultMask, stencil), { resultMask[i] |= stencil[i]; });
                compute::for_each(compute::counting_iterator<unsigned int>(0), compute::counting_iterator<unsigned int>(nrows), addOnlyMaskDesne, queue);
            } else {
                compute::detail::meta_kernel kernel("__spla_vec_eadd_dense_dense");
                auto argCount = kernel.add_arg<cl_uint>("count");
                auto argInputMask = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "inputMask");
                auto argInputVals = kernel.add_arg<const cl_uchar *>(compute::memory_object::global_memory, "inputVals");
                auto argResultMask = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "resultMask");
                auto argResultVals = kernel.add_arg<cl_uchar *>(compute::memory_object::global_memory, "resultVals");

                std::string addFunc("add_func");
                kernel.add_function(addFunc,
                                    detail::meta::MakeFunction(addFunc, op->GetSource(),
                                                               detail::meta::Visibility::Global,
                                                               detail::meta::Visibility::Global,
                                                               detail::meta::Visibility::Global));

                kernel << "#define BYTE_SIZE " << static_cast<cl_uint>(byteSize) << "\n"
                       << "const uint i = get_global_id(0);\n"
                       << "if (i < count) {\n"
                       << "    const uint m1 = resultMask[i];\n"
                       << "    const uint m2 = inputMask[i];\n"
                       << "    if (m1 && m2) {\n"
                       << "        const uint offset = i * BYTE_SIZE;\n"
                       << "        " << addFunc << "(&resultVals[offset], &inputVals[offset], &resultVals[offset]);\n"
                       << "    }\n else if (m2) {\n"
                       << "        resultMask[i] = 1u;\n"
                       << "        const uint offset = i * BYTE_SIZE;\n"
                       << "        for (uint k = 0; k < BYTE_SIZE; k++) {\n"
                       << "            resultVals[offset + k] = inputVals[offset + k];\n"
                       << "        }\n"
                       << "    }\n"
                       << "}\n";

                std::size_t tpb = 64;
                std::size_t workSize = compute::detail::calculate_work_size(nrows, 1, tpb);

                auto compiledKernel = kernel.compile(ctx);
                compiledKernel.set_arg(argCount, static_cast<cl_uint>(nrows));
                compiledKernel.set_arg(argInputMask, block->GetMask().get_buffer());
                compiledKernel.set_arg(argInputVals, block->GetVals().get_buffer());
                compiledKernel.set_arg(argResultMask, resultMask.get_buffer());
                compiledKernel.set_arg(argResultVals, resultVals.get_buffer());
                queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
            }
        }
    }

    PF_SCOPE_MARK(eadd, "add arg 2");

    // Apply mask to define, which indices we are interested in
    if (p->hasMask && p->mask.IsNotNull()) {
        assert(p->mask->GetFormat() == VectorBlock::Format::COO || p->mask->GetFormat() == VectorBlock::Format::Dense);
        CHECK_RAISE_ERROR(p->mask->GetFormat() == VectorBlock::Format::COO || p->mask->GetFormat() == VectorBlock::Format::Dense,
                          NotImplemented, "Other mask block type not supported yet");

        if (p->mask->GetFormat() == VectorBlock::Format::COO) {
            auto mask = p->mask.Cast<VectorCOO>();
            assert(mask.IsNotNull());
            ApplyMaskDenseSparse(resultMask, mask->GetRows(), complement, queue);
        } else {
            auto mask = p->mask.Cast<VectorDense>();
            assert(mask.IsNotNull());
            auto &stencil = mask->GetMask();
            ApplyMaskDenseDense(resultMask, stencil, complement, queue);
        }
    }

    PF_SCOPE_MARK(eadd, "apply mask");

    // Commute nnz count and store result
    compute::vector<Index> deviceNnz(1, ctx);
    compute::reduce(resultMask.begin(), resultMask.end(), deviceNnz.begin(), queue);

    std::size_t resultNnz = deviceNnz.begin().read(queue);

    if (resultNnz > 0)
        p->w = VectorDense::Make(nrows, resultNnz, std::move(resultMask), std::move(resultVals)).As<VectorBlock>();

    PF_SCOPE_MARK(eadd, "eval nnz");
}

spla::Algorithm::Type spla::VectorEWiseAddDense::GetType() const {
    return spla::Algorithm::Type::VectorEWiseAdd;
}

std::string spla::VectorEWiseAddDense::GetName() const {
    return "VectorEWiseAddDense";
}
