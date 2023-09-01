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

#define SPLA_WRAP_EXEC(what, ...)                                                                                                     \
    spla::ref_ptr<spla::ScheduleTask> ref_task;                                                                                       \
    const spla::Status                status = spla::what(__VA_ARGS__, as_ref<spla::Descriptor>(desc), (task ? &ref_task : nullptr)); \
    if (status == spla::Status::Ok && task) {                                                                                         \
        *task = as_ptr<spla_ScheduleTask_t>(ref_task.release());                                                                      \
        return SPLA_STATUS_OK;                                                                                                        \
    }                                                                                                                                 \
    return to_c_status(status);

#define AS_S(x)  as_ref<spla::Scalar>(x)
#define AS_M(x)  as_ref<spla::Matrix>(x)
#define AS_V(x)  as_ref<spla::Vector>(x)
#define AS_OB(x) as_ref<spla::OpBinary>(x)
#define AS_OU(x) as_ref<spla::OpUnary>(x)
#define AS_OS(x) as_ref<spla::OpSelect>(x)

spla_Status spla_Exec_mxm(spla_Matrix R, spla_Matrix A, spla_Matrix B, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_mxm, AS_M(R), AS_M(A), AS_M(B), AS_OB(op_multiply), AS_OB(op_add), AS_S(init));
}
spla_Status spla_Exec_mxmT_masked(spla_Matrix R, spla_Matrix mask, spla_Matrix A, spla_Matrix B, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_mxmT_masked, AS_M(R), AS_M(mask), AS_M(A), AS_M(B), AS_OB(op_multiply), AS_OB(op_add), AS_OS(op_select), AS_S(init));
}
spla_Status spla_Exec_mxv_masked(spla_Vector r, spla_Vector mask, spla_Matrix M, spla_Vector v, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_mxv_masked, AS_V(r), AS_V(mask), AS_M(M), AS_V(v), AS_OB(op_multiply), AS_OB(op_add), AS_OS(op_select), AS_S(init));
}
spla_Status spla_Exec_vxm_masked(spla_Vector r, spla_Vector mask, spla_Vector v, spla_Matrix M, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_vxm_masked, AS_V(r), AS_V(mask), AS_V(v), AS_M(M), AS_OB(op_multiply), AS_OB(op_add), AS_OS(op_select), AS_S(init));
}
spla_Status spla_Exec_m_reduce_by_row(spla_Vector r, spla_Matrix M, spla_OpBinary op_reduce, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_m_reduce_by_row, AS_V(r), AS_M(M), AS_OB(op_reduce), AS_S(init));
}
spla_Status spla_Exec_m_reduce_by_column(spla_Vector r, spla_Matrix M, spla_OpBinary op_reduce, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_m_reduce_by_column, AS_V(r), AS_M(M), AS_OB(op_reduce), AS_S(init));
}
spla_Status spla_Exec_m_reduce(spla_Scalar r, spla_Scalar s, spla_Matrix M, spla_OpBinary op_reduce, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_m_reduce, AS_S(r), AS_S(s), AS_M(M), AS_OB(op_reduce));
}
spla_Status spla_Exec_v_eadd(spla_Vector r, spla_Vector u, spla_Vector v, spla_OpBinary op, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_eadd, AS_V(r), AS_V(u), AS_V(v), AS_OB(op));
}
spla_Status spla_Exec_v_emult(spla_Vector r, spla_Vector u, spla_Vector v, spla_OpBinary op, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_emult, AS_V(r), AS_V(u), AS_V(v), AS_OB(op));
}
spla_Status spla_Exec_v_eadd_fdb(spla_Vector r, spla_Vector v, spla_Vector fdb, spla_OpBinary op, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_eadd_fdb, AS_V(r), AS_V(v), AS_V(fdb), AS_OB(op));
}
spla_Status spla_Exec_v_assign_masked(spla_Vector r, spla_Vector mask, spla_Scalar value, spla_OpBinary op_assign, spla_OpSelect op_select, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_assign_masked, AS_V(r), AS_V(mask), AS_S(value), AS_OB(op_assign), AS_OS(op_select));
}
spla_Status spla_Exec_v_map(spla_Vector r, spla_Vector v, spla_OpUnary op, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_map, AS_V(r), AS_V(v), AS_OU(op));
}
spla_Status spla_Exec_v_reduce(spla_Scalar r, spla_Scalar s, spla_Vector v, spla_OpBinary op_reduce, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_reduce, AS_S(r), AS_S(s), AS_V(v), AS_OB(op_reduce));
}
spla_Status spla_Exec_v_count_mf(spla_Scalar r, spla_Vector v, spla_Descriptor desc, spla_ScheduleTask* task) {
    SPLA_WRAP_EXEC(exec_v_count_mf, AS_S(r), AS_V(v));
}