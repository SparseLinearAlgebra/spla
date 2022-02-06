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

#include <algo/matrix/SplaMatrixTriaCOO.hpp>
#include <compute/SplaGather.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaQueueFinisher.hpp>
#include <storage/SplaMatrixStorage.hpp>
#include <storage/block/SplaMatrixCOO.hpp>
#include <storage/block/SplaMatrixCSR.hpp>
#include <utils/SplaProfiling.hpp>

bool spla::MatrixTriaCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsTria *>(&params);

    return p &&
           p->a.Is<MatrixCOO>();
}

void spla::MatrixTriaCOO::Process(spla::AlgorithmParams &params) {
    using namespace boost;

    PF_SCOPE(tria, "-tria-");

    auto p = dynamic_cast<ParamsTria *>(&params);
    auto library = p->desc->GetLibrary().GetPrivatePtr();
    auto &desc = p->desc;

    auto device = library->GetDeviceManager().GetDevice(p->deviceId);
    compute::context ctx = library->GetContext();
    compute::command_queue queue(ctx, device);
    QueueFinisher finisher(queue, library->GetLogger());

    auto &type = p->type;
    auto byteSize = type->GetByteSize();
    auto typeHasValues = type->HasValues();

    auto a = p->a.Cast<MatrixCOO>();

    // Do not process empty block
    if (a.IsNull())
        return;

    assert(a->GetNvals() > 0);

    const auto sourceNvals = a->GetNvals();
    const auto &sourceRows = a->GetRows();
    const auto &sourceCols = a->GetCols();

    PF_SCOPE_MARK(tria, "setup");

    // Define indices which below main diagonal
    compute::vector<unsigned int> indicesCount(1, ctx);
    compute::vector<unsigned int> indices(sourceNvals, ctx);
    compute::fill_n(indicesCount.begin(), 1, 0u, queue);
    {
        compute::detail::meta_kernel kernel("__spla_select_tria");
        auto argOffsetI = kernel.add_arg<cl_uint>("offsetI");
        auto argOffsetJ = kernel.add_arg<cl_uint>("offsetJ");
        auto argCount = kernel.add_arg<cl_uint>("count");
        auto argRows = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "rows");
        auto argCols = kernel.add_arg<const cl_uint *>(compute::memory_object::global_memory, "cols");
        auto argIndices = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "indices");
        auto argIndicesCount = kernel.add_arg<cl_uint *>(compute::memory_object::global_memory, "indicesCount");

        kernel << "const uint i = get_global_id(0);\n"
               << "if (i < count) {\n"
               << "    const uint rowId = offsetI + rows[i];\n"
               << "    const uint colId = offsetJ + cols[i];\n"
               << "    if (rowId > colId) {\n"
               << "        indices[atomic_add(&indicesCount[0], 1u)] = i;\n"
               << "    }\n"
               << "}\n";

        auto compiledKernel = kernel.compile(ctx);
        compiledKernel.set_arg(argOffsetI, static_cast<cl_uint>(p->firstI));
        compiledKernel.set_arg(argOffsetJ, static_cast<cl_uint>(p->firstJ));
        compiledKernel.set_arg(argCount, static_cast<cl_uint>(sourceNvals));
        compiledKernel.set_arg(argRows, sourceRows.get_buffer());
        compiledKernel.set_arg(argCols, sourceCols.get_buffer());
        compiledKernel.set_arg(argIndices, indices.get_buffer());
        compiledKernel.set_arg(argIndicesCount, indicesCount.get_buffer());

        std::size_t tpb = 64;
        std::size_t workSize = compute::detail::calculate_work_size(sourceNvals, 1, tpb);

        queue.enqueue_1d_range_kernel(compiledKernel, 0, workSize, tpb);
    }

    PF_SCOPE_MARK(tria, "select tria");

    // Get result number of values
    auto resultNvals = (indicesCount.begin()).read(queue);

    // No value, nothing to do
    if (!resultNvals)
        return;

    PF_SCOPE_MARK(tria, "read nnz");

    // Resize indices and sort before copy
    indices.resize(resultNvals, queue);
    compute::sort(indices.begin(), indices.end(), queue);

    PF_SCOPE_MARK(tria, "sort indices");

    compute::vector<unsigned int> resultRows(resultNvals, ctx);
    compute::vector<unsigned int> resultCols(resultNvals, ctx);
    compute::vector<unsigned char> resultVals(ctx);

    // Copy indices
    compute::gather(indices.begin(), indices.end(), sourceRows.begin(), resultRows.begin(), queue);
    compute::gather(indices.begin(), indices.end(), sourceCols.begin(), resultCols.begin(), queue);

    PF_SCOPE_MARK(tria, "gather indices");

    // Copy values if present
    if (typeHasValues) {
        resultVals.resize(resultNvals * byteSize, queue);
        Gather(indices.begin(), indices.end(), a->GetVals().begin(), resultVals.begin(), byteSize, queue);
    }

    PF_SCOPE_MARK(tria, "gather values");

    p->w = MatrixCSR::Make(a->GetNrows(), a->GetNcols(), resultNvals,
                           std::move(resultRows), std::move(resultCols), std::move(resultVals),
                           queue)
                   .As<MatrixBlock>();
}

spla::Algorithm::Type spla::MatrixTriaCOO::GetType() const {
    return spla::Algorithm::Type::Tria;
}

std::string spla::MatrixTriaCOO::GetName() const {
    return "MatrixTriaCOO";
}
