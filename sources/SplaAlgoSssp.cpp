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

#include <spla-algo/SplaAlgoSssp.hpp>
#include <spla-cpp/Spla.hpp>

#include <compute/SplaIndicesToRowOffsets.hpp>
#include <core/SplaError.hpp>

#include <limits>
#include <queue>
#include <vector>

void spla::Sssp(RefPtr<Vector> &sp_v, const RefPtr<Matrix> &sp_A, Index s) {
    CHECK_RAISE_ERROR(sp_A.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(sp_A->GetNrows() == sp_A->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(s < sp_A->GetNrows(), InvalidArgument, "Start index must be withing A bounds");

    auto &library = sp_A->GetLibrary();
    auto type = sp_A->GetType();
    auto n = sp_A->GetNrows();

    CHECK_RAISE_ERROR(type->IsBuiltIn(), InvalidArgument, "Must be built-in type");
    CHECK_RAISE_ERROR(type == Types::Float32(library), InvalidArgument, "Must be float32 type");

    float zero = 0.0f;

    // Used to check if something changed
    auto reduceOp = Functions::PlusFloat32(library);
    // Used to concatenate paths lengths
    auto multOp = Functions::PlusFloat32(library);
    // Used find shortest path almond others
    auto addOp = Functions::MinFloat32(library);

    // Vector with reached paths
    sp_v = Vector::Make(n, type, library);
    // Temporary to store new reached paths
    auto sp_w = Vector::Make(n, type, library);
    // Scalar to store reduce succ
    auto sp_succ = Scalar::Make(type, library);

    // Set v[s] = w[s] = 0.0f - so start is reached in 0.0
    auto sp_setup = Expression::Make(library);
    sp_setup->MakeDataWrite(sp_v, DataVector::Make(&s, &zero, 1, library));
    sp_setup->MakeDataWrite(sp_w, DataVector::Make(&s, &zero, 1, library));
    sp_setup->SubmitWait();

    std::size_t iterations = 0;
    float succLast = 0.0f;
    float succ = 1.0f;

    while (iterations < n && succ != succLast) {
        succLast = succ;

        auto sp_iter = Expression::Make(library);

        auto t1 = sp_iter->MakeVxM(sp_w, nullptr, multOp, addOp, sp_w, sp_A);
        auto t2 = sp_iter->MakeEWiseAdd(sp_v, nullptr, addOp, sp_v, sp_w);
        auto t3 = sp_iter->MakeReduce(sp_succ, reduceOp, sp_v);
        auto t4 = sp_iter->MakeDataRead(sp_succ, DataScalar::Make(&succ, library));

        sp_iter->Dependency(t1, t2);
        sp_iter->Dependency(t2, t3);
        sp_iter->Dependency(t3, t4);
        sp_iter->SubmitWait();

        iterations += 1;
    }

#if defined(SPLA_DEBUG) || defined(SPLA_DEBUG_RELEASE)
    std::cout << "Exec iterations: " << iterations << "\n";
#endif
}

void spla::Sssp(RefPtr<HostVector> &v, const RefPtr<HostMatrix> &A, Index s) {
    CHECK_RAISE_ERROR(A.IsNotNull(), NullPointer, "Passed null argument");
    CHECK_RAISE_ERROR(A->GetNrows() == A->GetNcols(), DimensionMismatch, "Matrix must be nxn");
    CHECK_RAISE_ERROR(s < A->GetNrows(), InvalidArgument, "Start index must be withing A bounds");
    CHECK_RAISE_ERROR(A->GetElementSize() == sizeof(float), InvalidType, "Matrix A must have float values of size 4");

    auto n = A->GetNrows();
    auto inf = std::numeric_limits<float>::max();

    // Note: implementation of shortest path fastest algorithms
    // see for details https://en.wikipedia.org/wiki/Shortest_Path_Faster_Algorithm

    // Offset to fetch rows by index
    std::vector<Index> offsets;
    // Initial distances to vertices
    std::vector<float> distances(n, inf);
    // Track already reached vertices
    std::vector<bool> isInQueue(n, false);
    // Queue of the search
    std::queue<Index> front;

    // Compute row offsets
    IndicesToRowOffsets(A->GetRowIndices(), offsets, n);

    // Start from s
    front.push(s);
    distances[s] = 0.0f;
    isInQueue[s] = true;

    while (!front.empty()) {
        auto i = front.front();
        front.pop();
        isInQueue[i] = false;

        // Traverse all adjacent neighbors
        for (Index k = offsets[i]; k < offsets[i + 1]; k++) {
            auto j = A->GetColIndices()[k];
            auto w = *(reinterpret_cast<const float *>(A->GetValues().data()) + k);

            if (distances[j] == inf || distances[i] + w < distances[j]) {
                distances[j] = distances[i] + w;
                if (!isInQueue[j]) {
                    isInQueue[j] = true;
                    front.push(j);
                }
            }
        }
    }

    // Define reached vertices
    std::vector<Index> rows;
    std::vector<float> values;

    // Add in order only reached vertices
    for (Index k = 0; k < n; k++) {
        if (distances[k] != inf) {
            rows.push_back(k);
            values.push_back(distances[k]);
        }
    }

    // Convert to raw data
    std::vector<unsigned char> data(values.size() * sizeof(float));
    std::memcpy(data.data(), values.data(), values.size() * sizeof(float));

    // Build result vector
    v = RefPtr<HostVector>(new HostVector(n, std::move(rows), std::move(data)));
}