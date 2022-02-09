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

#include <spla-algo/SplaAlgoTc.hpp>
#include <spla-cpp/Spla.hpp>

#include <core/SplaError.hpp>
#include <core/SplaHash.hpp>

#include <cstring>
#include <unordered_set>
#include <vector>

void spla::Tc(std::int32_t &ntrins, RefPtr<Matrix> &spB, const RefPtr<Matrix> &spL, const RefPtr<Matrix> &spU, const RefPtr<Descriptor> &descriptor) {
    CHECK_RAISE_ERROR(spB.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(spL.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(spU.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(spL->GetNrows() == spL->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(spU->GetNrows() == spU->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(spL->GetDim() == spU->GetDim(), DimensionMismatch, "Must be of the same size");

    bool timing = false;

    if (descriptor.IsNotNull())
        timing = descriptor->IsParamSet(Descriptor::Param::ProfileTime);

    ntrins = 0;

    auto &library = spL->GetLibrary();

    CHECK_RAISE_ERROR(spL->GetType() == Types::Int32(library), InvalidArgument, "Must be int32 type");
    CHECK_RAISE_ERROR(spU->GetType() == Types::Int32(library), InvalidArgument, "Must be int32 type");

    auto multOp = Functions::MultInt32(library);
    auto addOp = Functions::PlusInt32(library);

    auto spNTriangles = Scalar::Make(Types::Int32(library), library);
    auto spNTrianglesData = DataScalar::Make(&ntrins, library);

    CpuTimer tightTimer;// Tight timer to measure iterations
    tightTimer.Start();

    auto spTcExpr = Expression::Make(library);
    auto spNodeMxM = spTcExpr->MakeMxM(spB, spL, multOp, addOp, spL, spU, nullptr);
    auto spNodeAccum = spTcExpr->MakeReduceScalar(spNTriangles, nullptr, nullptr, addOp, spB, nullptr);
    auto spNodeReadNTriangles = spTcExpr->MakeDataRead(spNTriangles, spNTrianglesData);
    spTcExpr->Dependency(spNodeMxM, spNodeAccum);
    spTcExpr->Dependency(spNodeAccum, spNodeReadNTriangles);
    spTcExpr->SubmitWait();
    SPLA_ALGO_CHECK(spTcExpr);

    tightTimer.Stop();

    if (timing) {
        std::cout << "Result: " << ntrins << " ntrins\n";
        std::cout << " run tight: " << tightTimer.GetElapsedMs() << "\n";
    }
}

void spla::Tc(std::int32_t &ntrins, RefPtr<Matrix> &spB, const RefPtr<Matrix> &spA) {
    CHECK_RAISE_ERROR(spB.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(spA.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(spA->GetNrows() == spA->GetNcols(), DimensionMismatch, "Matrix must be nxn");

    ntrins = 0;

    auto &library = spA->GetLibrary();
    auto type = spA->GetType();

    CHECK_RAISE_ERROR(type->IsBuiltIn(), InvalidArgument, "Must be built-in type");
    CHECK_RAISE_ERROR(type == Types::Int32(library), InvalidArgument, "Must be int32 type");

    auto multOp = Functions::MultInt32(library);
    auto addOp = Functions::PlusInt32(library);

    const auto &spATransposed = spA;
    auto spNTriangles = Scalar::Make(type, library);
    auto spNTrianglesData = spla::DataScalar::Make(library);
    spNTrianglesData->SetValue(&ntrins);

    auto spTcExpr = Expression::Make(library);
    auto spNodeMxM = spTcExpr->MakeMxM(spB, spA, multOp, addOp, spA, spATransposed, nullptr);
    auto spNodeAccum = spTcExpr->MakeReduceScalar(spNTriangles, nullptr, nullptr, addOp, spB, nullptr);
    auto spNodeReadNTriangles = spTcExpr->MakeDataRead(spNTriangles, spNTrianglesData);

    spTcExpr->Dependency(spNodeMxM, spNodeAccum);
    spTcExpr->Dependency(spNodeAccum, spNodeReadNTriangles);

    spTcExpr->SubmitWait();
    SPLA_ALGO_CHECK(spTcExpr);

    ntrins /= 6;
}

void spla::Tc(std::int32_t &ntrins, RefPtr<HostMatrix> &B, const RefPtr<HostMatrix> &A) {
    CHECK_RAISE_ERROR(A.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(A->GetNrows() == A->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(A->GetElementSize() == sizeof(std::int32_t), InvalidType, "Matrix A must have int32 values of size 4");

    ntrins = 0;
    const Size n = A->GetNcols();
    const Size m = A->GetNnvals();

    /**
     * Counting triangles
     * Triangle - set of edges
     *  - from->to
     *  - from->inter
     *  - to->inter
     */

    std::unordered_set<std::pair<Index, Index>, PairHash> edges;

    for (Size edge = 0; edge < m; ++edge) {
        Index from = A->GetRowIndices()[edge];
        Index to = A->GetColIndices()[edge];
        edges.insert({from, to});
    }

    auto hasEdge = [&](Index a, Index b) {
        return edges.find({a, b}) != edges.end();
    };

    std::vector<Index> trinsFrom;
    std::vector<Index> trinsTo;
    std::vector<std::int32_t> nTrins;

    trinsFrom.reserve(m);
    trinsTo.reserve(m);
    nTrins.reserve(m);

    for (Size edge = 0; edge < m; ++edge) {
        Index from = A->GetRowIndices()[edge];
        Index to = A->GetColIndices()[edge];
        std::int32_t thisEdgeTrins = 0;
        for (Index inter = 0; inter < n; ++inter) {
            if (hasEdge(from, inter) && hasEdge(to, inter)) {
                thisEdgeTrins += 1;
            }
        }
        ntrins += thisEdgeTrins;
        if (thisEdgeTrins != 0) {
            trinsFrom.push_back(from);
            trinsTo.push_back(to);
            nTrins.push_back(thisEdgeTrins);
        }
    }

    // Convert to raw data
    std::vector<unsigned char> data(nTrins.size() * sizeof(std::int32_t));
    std::memcpy(data.data(), nTrins.data(), nTrins.size() * sizeof(std::int32_t));

    B = RefPtr<HostMatrix>(new HostMatrix(n, n, std::move(trinsFrom), std::move(trinsTo), std::move(data)));

    ntrins /= 6;
}
