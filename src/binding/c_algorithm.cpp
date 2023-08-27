/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#include "c_config.hpp"

spla_Status spla_Algorithm_bfs(spla_Vector v, spla_Matrix A, spla_uint s, spla_Descriptor descriptor) {
    return to_c_status(spla::bfs(as_ref<spla::Vector>(v), as_ref<spla::Matrix>(A), s, as_ref<spla::Descriptor>(descriptor)));
}
spla_Status spla_Algorithm_sssp(spla_Vector v, spla_Matrix A, spla_uint s, spla_Descriptor descriptor) {
    return to_c_status(spla::sssp(as_ref<spla::Vector>(v), as_ref<spla::Matrix>(A), s, as_ref<spla::Descriptor>(descriptor)));
}
spla_Status spla_Algorithm_pr(spla_Vector* p, spla_Matrix A, float alpha, float eps, spla_Descriptor descriptor) {
    spla::ref_ptr<spla::Vector> p_inout;
    p_inout.reset(as_ptr<spla::Vector>(*p));

    spla::Status status = spla::pr(p_inout, as_ref<spla::Matrix>(A), alpha, eps, as_ref<spla::Descriptor>(descriptor));

    if (status == spla::Status::Ok) {
        *p = as_ptr<spla_Vector_t>(p_inout.release());
        return SPLA_STATUS_OK;
    }

    return to_c_status(status);
}
spla_Status spla_Algorithm_tc(int* ntrins, spla_Matrix A, spla_Matrix B, spla_Descriptor descriptor) {
    return to_c_status(spla::tc(*ntrins, as_ref<spla::Matrix>(A), as_ref<spla::Matrix>(B), as_ref<spla::Descriptor>(descriptor)));
}