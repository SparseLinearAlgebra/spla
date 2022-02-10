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

#include <algo/mxm/SplaMxMMaskedCSRCSC.hpp>
#include <compute/SplaApplyMask.hpp>
#include <compute/SplaIndicesToRowOffsets.hpp>
#include <compute/SplaReduceByKey.hpp>
#include <compute/SplaReduceDuplicates.hpp>
#include <compute/SplaStableSortByColumn.hpp>
#include <compute/SplaTransformValues.hpp>
#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <utils/SplaProfiling.hpp>

bool spla::MxMMaskedCSRCSC::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsMxM *>(&params);

    // Requires a and bT be in csr format with mask in csr
    return p &&
           !p->desc->IsParamSet(Descriptor::Param::MaskComplement) &&
           p->mask.Is<MatrixCSR>() &&
           p->a.Is<MatrixCSR>() &&
           p->bT.Is<MatrixCSR>() &&
           p->mask.IsNotNull() &&
           p->a.IsNotNull() &&
           p->bT.IsNotNull();
}

void spla::MxMMaskedCSRCSC::Process(spla::AlgorithmParams &algoParams) {
    using namespace boost;

    PF_SCOPE(mxm, "-mxm-m-csr-csc-");

    auto params = dynamic_cast<ParamsMxM *>(&algoParams);
    auto library = params->desc->GetLibrary().GetPrivatePtr();
    auto device = library->GetDeviceManager().GetDevice(params->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    SPDLOG_LOGGER_TRACE(library->GetLogger(), "Select masked csrxcsc");

    // Algorithm strategy:
    // 1. For each mask value (i,j) define product of A[i,:] x B^T[j,:]
    // 2. Allocate required memory for rows cols and indices
    // 3. For each mask value (i,j) evaluate product of A[i,:] x B^T[j,:]
    // 4. Use a warp (32/64 threads) per row

    MatrixCSR &m = *params->mask.Cast<MatrixCSR>();
    MatrixCSR &a = *params->a.Cast<MatrixCSR>();
    MatrixCSR &bT = *params->bT.Cast<MatrixCSR>();

    std::size_t M = a.GetNrows(),
                K = a.GetNcols(),
                N = bT.GetNrows();

    std::size_t tpb = 32;
    std::size_t workSize = M * tpb;

    assert(m.GetNrows() == M);
    assert(m.GetNcols() == N);
    assert(bT.GetNcols() == K);

    // Auxiliary binary search to find bCol in the bT row by aCol
    std::stringstream binSearch;
    binSearch << "uint binary_search(const uint x, __global const uint* buffer, const uint range) {\n"
              << "    int l = 0, r = range - 1;\n"
              << "    while (l <= r) {\n"
              << "        int k = (l + r) / 2;\n"
              << "        if (buffer[k] < x) l = k + 1;\n"
              << "        else if (x < buffer[k]) r = k - 1;\n"
              << "        else return k;\n"
              << "    }\n"
              << "    return 0xffffffff;\n"
              << "}\n";

    // Evaluate exact size of the result to allocate buffers
    compute::vector<unsigned int> rowsLenC(M + 1, ctx);
    compute::fill(rowsLenC.begin(), rowsLenC.end(), 0u, queue);
    {
        compute::detail::meta_kernel kernel("__spla_masked_csr_csc_structure");
        kernel.add_function("binary_search", binSearch.str());

        auto argRowsPtrM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrM");
        auto argColsM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsM");
        auto argRowsPtrA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrA");
        auto argColsA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsA");
        auto argRowsPtrB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrB");
        auto argColsB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsB");
        auto argRowsLenC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "rowsLenC");

        kernel << "#define tpb " << static_cast<cl_uint>(tpb) << "\n"
               << "const uint rid = get_group_id(0);\n"
               << "const uint tid = get_local_id(0);\n"
               << "__local uint hasValue;\n"
               << "const uint mRowStart = rowsPtrM[rid];\n"
               << "const uint mRowEnd = rowsPtrM[rid + 1];\n"
               << "for (uint mIdx = mRowStart; mIdx < mRowEnd; mIdx++) {\n"
               << "    if (tid == 0) hasValue = 0;\n"
               << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "    const uint cid = colsM[mIdx];\n"
               << "    const uint aRowStart = rowsPtrA[rid];\n"
               << "    const uint aRowEnd = rowsPtrA[rid + 1];\n"
               << "    const uint bRowStart = rowsPtrB[cid];\n"
               << "    const uint bRowEnd = rowsPtrB[cid + 1];\n"
               << "    const uint bRowLen = bRowEnd - bRowStart;\n"
               << "    if (bRowLen > 0) {\n"
               << "        for (uint aIdx = aRowStart + tid; aIdx < aRowEnd; aIdx += tpb) {\n"
               << "            const uint aCol = colsA[aIdx];\n"
               << "            const uint bIdx = binary_search(aCol, &colsB[bRowStart], bRowLen);\n"
               << "            if (bIdx != 0xffffffff) hasValue = 1;\n"
               << "        }\n"
               << "        barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "        if (hasValue && tid == 0) atomic_inc(&rowsLenC[rid]);\n"
               << "    }\n"
               << "}\n";

        auto compiledKernel = kernel.compile(ctx);
        compiledKernel.set_arg(argRowsPtrM, m.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsM, m.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrA, a.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsA, a.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrB, bT.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsB, bT.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsLenC, rowsLenC.get_buffer());
        queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
    }

    // Evaluate offsets for each row of the result
    compute::vector<unsigned int> rowsPtrC(M + 1, ctx);
    compute::exclusive_scan(rowsLenC.begin(), rowsLenC.end(), rowsPtrC.begin(), 0u, queue);

    // Get nnz of the result
    std::size_t nvalsC = (rowsPtrC.end() - 1).read(queue);

    SPDLOG_LOGGER_TRACE(library->GetLogger(), "Result size {} nnz", nvalsC);

    // Early exit, nothing to do
    if (nvalsC <= 0)
        return;

    // Evaluate actual result
    compute::vector<unsigned int> rowsC(nvalsC, ctx);
    compute::vector<unsigned int> colsC(nvalsC, ctx);
    compute::vector<unsigned char> valsC(params->tw->HasValues() ? nvalsC * params->tw->GetByteSize() : 0, ctx);
    {
        compute::detail::meta_kernel kernel("__spla_masked_csr_csc_prod");
        kernel.add_function("binary_search", binSearch.str());

        auto argRowsPtrM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrM");
        auto argColsM = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsM");
        auto argRowsPtrA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrA");
        auto argColsA = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsA");
        auto argRowsPtrB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrB");
        auto argColsB = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "colsB");
        auto argRowsPtrC = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rowsPtrC");
        auto argRowsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "rowsC");
        auto argColsC = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "colsC");

        kernel << "#define tpb " << static_cast<cl_uint>(tpb) << "\n"
               << "const uint rid = get_group_id(0);\n"
               << "const uint tid = get_local_id(0);\n"
               << "__local uint hasValue;\n"
               << "uint writePos = 0;\n"
               << "const uint mRowStart = rowsPtrM[rid];\n"
               << "const uint mRowEnd = rowsPtrM[rid + 1];\n"
               << "const uint cRowStart = rowsPtrC[rid];\n"
               << "for (uint mIdx = mRowStart; mIdx < mRowEnd; mIdx++) {\n"
               << "    if (tid == 0) hasValue = 0;\n"
               << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "    const uint cid = colsM[mIdx];\n"
               << "    const uint aRowStart = rowsPtrA[rid];\n"
               << "    const uint aRowEnd = rowsPtrA[rid + 1];\n"
               << "    const uint bRowStart = rowsPtrB[cid];\n"
               << "    const uint bRowEnd = rowsPtrB[cid + 1];\n"
               << "    const uint bRowLen = bRowEnd - bRowStart;\n"
               << "    if (bRowLen > 0) {\n"
               << "        for (uint aIdx = aRowStart + tid; aIdx < aRowEnd; aIdx += tpb) {\n"
               << "            const uint aCol = colsA[aIdx];\n"
               << "            const uint bIdx = binary_search(aCol, &colsB[bRowStart], bRowLen);\n"
               << "            if (bIdx != 0xffffffff) hasValue = 1;\n"
               << "        }\n"
               << "        barrier(CLK_LOCAL_MEM_FENCE);\n"
               << "        if (hasValue && tid == 0) {\n"
               << "            rowsC[cRowStart + writePos] = rid;\n"
               << "            colsC[cRowStart + writePos] = cid;\n"
               << "            writePos += 1;\n"
               << "        }\n"
               << "    }\n"
               << "}\n";

        auto compiledKernel = kernel.compile(ctx);
        compiledKernel.set_arg(argRowsPtrM, m.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsM, m.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrA, a.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsA, a.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrB, bT.GetRowsOffsets().get_buffer());
        compiledKernel.set_arg(argColsB, bT.GetCols().get_buffer());
        compiledKernel.set_arg(argRowsPtrC, rowsPtrC.get_buffer());
        compiledKernel.set_arg(argRowsC, rowsC.get_buffer());
        compiledKernel.set_arg(argColsC, colsC.get_buffer());
        queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
    }

    params->w = MatrixCSR::Make(M, N, nvalsC,
                                std::move(rowsC), std::move(colsC), std::move(valsC),
                                std::move(rowsPtrC), std::move(rowsLenC))
                        .As<MatrixBlock>();
}

spla::Algorithm::Type spla::MxMMaskedCSRCSC::GetType() const {
    return spla::Algorithm::Type::MxM;
}

std::string spla::MxMMaskedCSRCSC::GetName() const {
    return "MxMMaskedCSRCSC";
}
