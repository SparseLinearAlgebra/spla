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

#include <compute/SplaIndicesToRowOffsets.hpp>
#include <core/SplaError.hpp>
#include <spla-algo/SplaAlgoBfs.hpp>
#include <spla-cpp/Spla.hpp>

#include <iostream>
#include <limits>
#include <queue>
#include <vector>

void spla::Bfs(RefPtr<Vector> &sp_v, const RefPtr<Matrix> &sp_A, Index s, const RefPtr<Descriptor> &descriptor) {
    CHECK_RAISE_ERROR(sp_A.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(sp_A->GetNrows() == sp_A->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(s < sp_A->GetNrows(), InvalidArgument, "Start index must be withing A bounds");

    bool timing = false;
    float denseFactor = 1.0f;

    if (descriptor.IsNotNull()) {
        timing = descriptor->IsParamSet(Descriptor::Param::ProfileTime);
        descriptor->GetParamT(Descriptor::Param::DenseFactor, denseFactor);
    }

    auto &library = sp_A->GetLibrary();
    auto n = sp_A->GetNrows();

    sp_v = Vector::Make(n, Types::Int32(library), library);      // Vector with reached levels
    auto sp_q = Vector::Make(n, Types::Void(library), library);  // Vector-front of the bfs
    auto sp_depth = Scalar::Make(Types::Int32(library), library);// Scalar to update depth of the v

    auto sp_desc_accum = Descriptor::Make(library);// Used to assign to v new reached level and preserve v values
    sp_desc_accum->SetParam(Descriptor::Param::AccumResult);

    auto sp_desc_comp = Descriptor::Make(library);// Used to apply !v mask when try to discover new values and ignore previously found
    sp_desc_comp->SetParam(Descriptor::Param::MaskComplement);

    CpuTimer overallTimer;// Overall time of execution
    overallTimer.Start();

    auto sp_setup = Expression::Make(library);// Set q[s]: start vertex
    sp_setup->MakeDataWrite(sp_q, DataVector::Make(&s, nullptr, 1, library));
    sp_setup->SubmitWait();

    std::int32_t depth = 1;// Start for depth 1: v[s]=1

    CpuTimer tightTimer;// Tight timer to measure iterations
    tightTimer.Start();
    double tight = 0.0;
    bool sparseToDense = false;

    while (sp_q->GetNvals() != 0) {
        auto sp_iter = Expression::Make(library);

        auto t1 = sp_iter->MakeDataWrite(sp_depth, DataScalar::Make(&depth, library));     // Update depth scalar
        auto t2 = sp_iter->MakeAssign(sp_v, sp_q, nullptr, sp_depth, sp_desc_accum);       // New reached vertices v[q] = depth
        auto t3 = sp_iter->MakeVxM(sp_q, sp_v, nullptr, nullptr, sp_q, sp_A, sp_desc_comp);// Discover new front q[!v] = q x A

        // Check how dense is result vector v
        // Explicitly transition to dense storage if is filled more than threshold
        // NOTE: in future we need automatically transition to dense inside library
        if (!sparseToDense && sp_v->GetFillFactor() >= denseFactor) {
            auto tt = sp_iter->MakeToDense(sp_v, sp_v);
            sp_iter->Dependency(tt, t2);
            sparseToDense = true;
        }

        sp_iter->Dependency(t1, t2);
        sp_iter->Dependency(t2, t3);
        sp_iter->SubmitWait();

        if (timing) {
            tightTimer.Stop();
            std::cout << " - iter: " << depth << ", src: " << s << ", visit: "
                      << sp_q->GetNvals() << "/" << n << ", " << tightTimer.GetElapsedMs() - tight << "\n";
            tight = tightTimer.GetElapsedMs();
            tightTimer.Start();
        }

        depth += 1;
    }

    tightTimer.Stop();
    overallTimer.Stop();

    if (timing) {
        std::cout << "Result: " << sp_v->GetNvals() << " reached\n";
        std::cout << " run tight: " << tightTimer.GetElapsedMs() << "\n";
        std::cout << " run overall: " << overallTimer.GetElapsedMs() << "\n";
    }
}

void spla::Bfs(RefPtr<HostVector> &v, const RefPtr<HostMatrix> &A, Index s) {
    CHECK_RAISE_ERROR(A.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(A->GetNrows() == A->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(s < A->GetNrows(), InvalidArgument, "Start index must be withing A bounds");

    auto n = A->GetNrows();

    // Offset to fetch rows by index
    std::vector<Index> offsets;
    // Depths of reached vertices, initially - +inf
    std::vector<std::int32_t> depths(n, std::numeric_limits<std::int32_t>::max());
    // Track already reached vertices
    std::vector<bool> visited(n, false);
    // Queue of the bfs front
    std::queue<Index> front;

    // Compute row offsets
    IndicesToRowOffsets(A->GetRowIndices(), offsets, n);

    // Start from s
    front.push(s);
    depths[s] = 1;
    visited[s] = true;

    while (!front.empty()) {
        // Get next vertex and remove it from the queue
        auto i = front.front();
        front.pop();

        // Traverse all adjacent neighbors
        for (Index k = offsets[i]; k < offsets[i + 1]; k++) {
            auto j = A->GetColIndices()[k];

            // Reached new vertex
            if (!visited[j]) {
                // Update depth
                depths[j] = depths[i] + 1;
                // Mark as visited
                visited[j] = true;
                // Add to the front
                front.push(j);
            }
        }
    }

    // Define reached vertices
    std::vector<Index> rows;
    std::vector<std::int32_t> values;

    // Add in order only reached vertices
    for (Index k = 0; k < n; k++) {
        if (visited[k]) {
            rows.push_back(k);
            values.push_back(depths[k]);
        }
    }

    // Convert to raw data
    std::vector<unsigned char> data(values.size() * sizeof(std::int32_t));
    std::memcpy(data.data(), values.data(), values.size() * sizeof(std::int32_t));

    // Build result vector
    v = RefPtr<HostVector>(new HostVector(n, std::move(rows), std::move(data)));
}