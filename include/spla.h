/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
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

#ifndef SPLA_SPLA_C_H
#define SPLA_SPLA_C_H

/**
 * @file spla.h
 * @author Egor Orachev
 *
 * @brief Spla library C API bindings
 *
 * @note Bindings primary intended for exporting to other programming languages,
 *       such as Python, etc. For a manual usage prefer C++ library API declared in spla.hpp file.
 *
 * @see Source code: https://github.com/SparseLinearAlgebra/spla
 * @see Python Reference API: https://SparseLinearAlgebra.github.io/spla/docs-python/spla
 * @see C/C++ Reference API: https://SparseLinearAlgebra.github.io/spla/docs-cpp
 */

#if defined(__cplusplus)
    #include <cinttypes>
    #include <cstddef>
#else
    #include <inttypes.h>
    #include <stddef.h>
#endif

#ifdef SPLA_MSVC
    #ifdef SPLA_EXPORTS
        #define SPLA_API __declspec(dllexport)
    #else
        #define SPLA_API __declspec(dllimport)
    #endif
#else
    #define SPLA_API
#endif

#if defined(__cplusplus)
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

/* General definitions */

typedef enum spla_Status {
    SPLA_STATUS_OK                 = 0,
    SPLA_STATUS_ERROR              = 1,
    SPLA_STATUS_NO_ACCELERATION    = 2,
    SPLA_STATUS_PLATFORM_NOT_FOUND = 3,
    SPLA_STATUS_DEVICE_NOT_FOUND   = 4,
    SPLA_STATUS_INVALID_STATE      = 5,
    SPLA_STATUS_INVALID_ARGUMENT   = 6,
    SPLA_STATUS_NO_VALUE           = 7,
    SPLA_STATUS_NOT_IMPLEMENTED    = 1024
} spla_Status;

typedef enum spla_AcceleratorType {
    SPLA_ACCELERATOR_TYPE_NONE   = 0,
    SPLA_ACCELERATOR_TYPE_OPENCL = 1
} spla_AcceleratorType;

typedef enum spla_FormatMatrix {
    SPLA_FORMAT_MATRIX_CPU_LIL = 0,
    SPLA_FORMAT_MATRIX_CPU_DOK = 1,
    SPLA_FORMAT_MATRIX_CPU_COO = 2,
    SPLA_FORMAT_MATRIX_CPU_CSR = 3,
    SPLA_FORMAT_MATRIX_CPU_CSC = 4,
    SPLA_FORMAT_MATRIX_ACC_COO = 5,
    SPLA_FORMAT_MATRIX_ACC_CSR = 6,
    SPLA_FORMAT_MATRIX_ACC_CSC = 7,
    SPLA_FORMAT_MATRIX_COUNT   = 8
} spla_FormatMatrix;

typedef enum spla_FormatVector {
    SPLA_FORMAT_VECTOR_CPU_DOK   = 0,
    SPLA_FORMAT_VECTOR_CPU_DENSE = 1,
    SPLA_FORMAT_VECTOR_CPU_COO   = 2,
    SPLA_FORMAT_VECTOR_ACC_DENSE = 3,
    SPLA_FORMAT_VECTOR_ACC_COO   = 4,
    SPLA_FORMAT_VECTOR_COUNT     = 5
} spla_FormatVector;

#define SPLA_NULL NULL

typedef int32_t  spla_bool;
typedef uint32_t spla_uint;
typedef size_t   spla_size_t;

typedef struct spla_RefCnt_t*       spla_RefCnt;
typedef struct spla_MemView_t*      spla_MemView;
typedef struct spla_Object_t*       spla_Object;
typedef struct spla_Type_t*         spla_Type;
typedef struct spla_Descriptor_t*   spla_Descriptor;
typedef struct spla_Matrix_t*       spla_Matrix;
typedef struct spla_Vector_t*       spla_Vector;
typedef struct spla_Array_t*        spla_Array;
typedef struct spla_Scalar_t*       spla_Scalar;
typedef struct spla_Schedule_t*     spla_Schedule;
typedef struct spla_ScheduleTask_t* spla_ScheduleTask;
typedef struct spla_OpUnary_t*      spla_OpUnary;
typedef struct spla_OpBinary_t*     spla_OpBinary;
typedef struct spla_OpSelect_t*     spla_OpSelect;

typedef void(spla_MessageCallback)(spla_Status, const char* message, const char* file, const char* function, int line, void* p_user_data);

//////////////////////////////////////////////////////////////////////////////////////

/* Library configuration and accessors */

SPLA_API void        spla_Library_finalize();
SPLA_API spla_Status spla_Library_initialize();
SPLA_API spla_Status spla_Library_set_accelerator(spla_AcceleratorType accelerator);
SPLA_API spla_Status spla_Library_set_platform(int index);
SPLA_API spla_Status spla_Library_set_device(int index);
SPLA_API spla_Status spla_Library_set_queues_count(int count);
SPLA_API spla_Status spla_Library_set_message_callback(spla_MessageCallback callback, void* p_user_data);
SPLA_API spla_Status spla_Library_set_default_callback();
SPLA_API spla_Status spla_Library_get_accelerator_info(char* buffer, int length);

//////////////////////////////////////////////////////////////////////////////////////

/* Built-in predefined scalar values types for storage parametrization  */

SPLA_API spla_Type spla_Type_BOOL();
SPLA_API spla_Type spla_Type_INT();
SPLA_API spla_Type spla_Type_UINT();
SPLA_API spla_Type spla_Type_FLOAT();

//////////////////////////////////////////////////////////////////////////////////////

/* Built-in unary element-wise operations */

SPLA_API spla_OpUnary spla_OpUnary_IDENTITY_INT();
SPLA_API spla_OpUnary spla_OpUnary_IDENTITY_UINT();
SPLA_API spla_OpUnary spla_OpUnary_IDENTITY_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_AINV_INT();
SPLA_API spla_OpUnary spla_OpUnary_AINV_UINT();
SPLA_API spla_OpUnary spla_OpUnary_AINV_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_MINV_INT();
SPLA_API spla_OpUnary spla_OpUnary_MINV_UINT();
SPLA_API spla_OpUnary spla_OpUnary_MINV_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_LNOT_INT();
SPLA_API spla_OpUnary spla_OpUnary_LNOT_UINT();
SPLA_API spla_OpUnary spla_OpUnary_LNOT_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_UONE_INT();
SPLA_API spla_OpUnary spla_OpUnary_UONE_UINT();
SPLA_API spla_OpUnary spla_OpUnary_UONE_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_ABS_INT();
SPLA_API spla_OpUnary spla_OpUnary_ABS_UINT();
SPLA_API spla_OpUnary spla_OpUnary_ABS_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_BNOT_INT();
SPLA_API spla_OpUnary spla_OpUnary_BNOT_UINT();
SPLA_API spla_OpUnary spla_OpUnary_SQRT_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_LOG_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_EXP_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_SIN_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_COS_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_TAN_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_ASIN_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_ACOS_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_ATAN_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_CEIL_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_FLOOR_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_ROUND_FLOAT();
SPLA_API spla_OpUnary spla_OpUnary_TRUNC_FLOAT();

//////////////////////////////////////////////////////////////////////////////////////

/* Built-in binary element-wise operations  */

SPLA_API spla_OpBinary spla_OpBinary_PLUS_INT();
SPLA_API spla_OpBinary spla_OpBinary_PLUS_UINT();
SPLA_API spla_OpBinary spla_OpBinary_PLUS_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_INT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_UINT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_MULT_INT();
SPLA_API spla_OpBinary spla_OpBinary_MULT_UINT();
SPLA_API spla_OpBinary spla_OpBinary_MULT_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_DIV_INT();
SPLA_API spla_OpBinary spla_OpBinary_DIV_UINT();
SPLA_API spla_OpBinary spla_OpBinary_DIV_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_POW2_INT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_POW2_UINT();
SPLA_API spla_OpBinary spla_OpBinary_MINUS_POW2_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_FIRST_INT();
SPLA_API spla_OpBinary spla_OpBinary_FIRST_UINT();
SPLA_API spla_OpBinary spla_OpBinary_FIRST_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_SECOND_INT();
SPLA_API spla_OpBinary spla_OpBinary_SECOND_UINT();
SPLA_API spla_OpBinary spla_OpBinary_SECOND_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_BONE_INT();
SPLA_API spla_OpBinary spla_OpBinary_BONE_UINT();
SPLA_API spla_OpBinary spla_OpBinary_BONE_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_MIN_INT();
SPLA_API spla_OpBinary spla_OpBinary_MIN_UINT();
SPLA_API spla_OpBinary spla_OpBinary_MIN_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_MAX_INT();
SPLA_API spla_OpBinary spla_OpBinary_MAX_UINT();
SPLA_API spla_OpBinary spla_OpBinary_MAX_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_LOR_INT();
SPLA_API spla_OpBinary spla_OpBinary_LOR_UINT();
SPLA_API spla_OpBinary spla_OpBinary_LOR_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_LAND_INT();
SPLA_API spla_OpBinary spla_OpBinary_LAND_UINT();
SPLA_API spla_OpBinary spla_OpBinary_LAND_FLOAT();
SPLA_API spla_OpBinary spla_OpBinary_BOR_INT();
SPLA_API spla_OpBinary spla_OpBinary_BOR_UINT();
SPLA_API spla_OpBinary spla_OpBinary_BAND_INT();
SPLA_API spla_OpBinary spla_OpBinary_BAND_UINT();
SPLA_API spla_OpBinary spla_OpBinary_BXOR_INT();
SPLA_API spla_OpBinary spla_OpBinary_BXOR_UINT();

//////////////////////////////////////////////////////////////////////////////////////

/* Built-in selection operations  */

SPLA_API spla_OpSelect spla_OpSelect_EQZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_EQZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_EQZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_NQZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_NQZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_NQZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_GTZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_GTZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_GTZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_GEZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_GEZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_GEZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_LTZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_LTZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_LTZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_LEZERO_INT();
SPLA_API spla_OpSelect spla_OpSelect_LEZERO_UINT();
SPLA_API spla_OpSelect spla_OpSelect_LEZERO_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_ALWAYS_INT();
SPLA_API spla_OpSelect spla_OpSelect_ALWAYS_UINT();
SPLA_API spla_OpSelect spla_OpSelect_ALWAYS_FLOAT();
SPLA_API spla_OpSelect spla_OpSelect_NEVER_INT();
SPLA_API spla_OpSelect spla_OpSelect_NEVER_UINT();
SPLA_API spla_OpSelect spla_OpSelect_NEVER_FLOAT();

//////////////////////////////////////////////////////////////////////////////////////

/* General base RefCnt type methods */

SPLA_API spla_Status spla_RefCnt_ref(spla_RefCnt object);
SPLA_API spla_Status spla_RefCnt_unref(spla_RefCnt object);

//////////////////////////////////////////////////////////////////////////////////////

/* General base Object type methods */

//////////////////////////////////////////////////////////////////////////////////////

/* Memory view resource methods */

SPLA_API spla_Status spla_MemView_make(spla_MemView* view, void* buffer, spla_size_t size, spla_bool is_mutable);
SPLA_API spla_Status spla_MemView_read(spla_MemView view, spla_size_t offset, spla_size_t size, void* dst);
SPLA_API spla_Status spla_MemView_write(spla_MemView view, spla_size_t offset, spla_size_t size, const void* src);
SPLA_API spla_Status spla_MemView_get_buffer(spla_MemView view, void** buffer);
SPLA_API spla_Status spla_MemView_get_size(spla_MemView view, spla_size_t* size);
SPLA_API spla_Status spla_MemView_is_mutable(spla_MemView view, spla_bool* is_mutable);

//////////////////////////////////////////////////////////////////////////////////////

/* Scala container creation and manipulation */

SPLA_API spla_Status spla_Scalar_make(spla_Scalar* scalar, spla_Type type);
SPLA_API spla_Status spla_Scalar_set_int(spla_Scalar s, int value);
SPLA_API spla_Status spla_Scalar_set_uint(spla_Scalar s, unsigned int value);
SPLA_API spla_Status spla_Scalar_set_float(spla_Scalar s, float value);
SPLA_API spla_Status spla_Scalar_get_int(spla_Scalar s, int* value);
SPLA_API spla_Status spla_Scalar_get_uint(spla_Scalar s, unsigned int* value);
SPLA_API spla_Status spla_Scalar_get_float(spla_Scalar s, float* value);

//////////////////////////////////////////////////////////////////////////////////////

/* Array container creation and manipulation */

SPLA_API spla_Status spla_Array_make(spla_Array* v, spla_uint n_values, spla_Type type);
SPLA_API spla_Status spla_Array_get_n_values(spla_Array a, spla_uint* values);
SPLA_API spla_Status spla_Array_set_int(spla_Array a, spla_uint i, int value);
SPLA_API spla_Status spla_Array_set_uint(spla_Array a, spla_uint i, unsigned int value);
SPLA_API spla_Status spla_Array_set_float(spla_Array a, spla_uint i, float value);
SPLA_API spla_Status spla_Array_get_int(spla_Array a, spla_uint i, int* value);
SPLA_API spla_Status spla_Array_get_uint(spla_Array a, spla_uint i, unsigned int* value);
SPLA_API spla_Status spla_Array_get_float(spla_Array a, spla_uint i, float* value);
SPLA_API spla_Status spla_Array_resize(spla_Array a, spla_uint n);
SPLA_API spla_Status spla_Array_build(spla_Array a, spla_MemView view);
SPLA_API spla_Status spla_Array_read(spla_Array a, spla_MemView* view);
SPLA_API spla_Status spla_Array_clear(spla_Array a);

//////////////////////////////////////////////////////////////////////////////////////

/* Vector container creation and manipulation */

SPLA_API spla_Status spla_Vector_make(spla_Vector* v, spla_uint n_rows, spla_Type type);
SPLA_API spla_Status spla_Vector_set_format(spla_Vector v, int format);
SPLA_API spla_Status spla_Vector_set_fill_value(spla_Vector v, spla_Scalar value);
SPLA_API spla_Status spla_Vector_set_reduce(spla_Vector v, spla_OpBinary reduce);
SPLA_API spla_Status spla_Vector_set_int(spla_Vector v, spla_uint row_id, int value);
SPLA_API spla_Status spla_Vector_set_uint(spla_Vector v, spla_uint row_id, unsigned int value);
SPLA_API spla_Status spla_Vector_set_float(spla_Vector v, spla_uint row_id, float value);
SPLA_API spla_Status spla_Vector_get_int(spla_Vector v, spla_uint row_id, int* value);
SPLA_API spla_Status spla_Vector_get_uint(spla_Vector v, spla_uint row_id, unsigned int* value);
SPLA_API spla_Status spla_Vector_get_float(spla_Vector v, spla_uint row_id, float* value);
SPLA_API spla_Status spla_Vector_build(spla_Vector v, spla_MemView keys, spla_MemView values);
SPLA_API spla_Status spla_Vector_read(spla_Vector v, spla_MemView* keys, spla_MemView* values);
SPLA_API spla_Status spla_Vector_clear(spla_Vector v);

//////////////////////////////////////////////////////////////////////////////////////

/* Matrix container creation and manipulation */

SPLA_API spla_Status spla_Matrix_make(spla_Matrix* M, spla_uint n_rows, spla_uint n_cols, spla_Type type);
SPLA_API spla_Status spla_Matrix_set_format(spla_Matrix M, int format);
SPLA_API spla_Status spla_Matrix_set_fill_value(spla_Matrix M, spla_Scalar value);
SPLA_API spla_Status spla_Matrix_set_reduce(spla_Matrix M, spla_OpBinary reduce);
SPLA_API spla_Status spla_Matrix_set_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int value);
SPLA_API spla_Status spla_Matrix_set_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int value);
SPLA_API spla_Status spla_Matrix_set_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float value);
SPLA_API spla_Status spla_Matrix_get_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int* value);
SPLA_API spla_Status spla_Matrix_get_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int* value);
SPLA_API spla_Status spla_Matrix_get_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float* value);
SPLA_API spla_Status spla_Matrix_build(spla_Matrix M, spla_MemView keys1, spla_MemView keys2, spla_MemView values);
SPLA_API spla_Status spla_Matrix_read(spla_Matrix M, spla_MemView* keys1, spla_MemView* keys2, spla_MemView* values);
SPLA_API spla_Status spla_Matrix_clear(spla_Matrix M);

//////////////////////////////////////////////////////////////////////////////////////

/* Implemented some common graph algorithms using spla library */

SPLA_API spla_Status spla_Algorithm_bfs(spla_Vector v, spla_Matrix A, spla_uint s, spla_Descriptor descriptor);
SPLA_API spla_Status spla_Algorithm_sssp(spla_Vector v, spla_Matrix A, spla_uint s, spla_Descriptor descriptor);
SPLA_API spla_Status spla_Algorithm_pr(spla_Vector* p, spla_Matrix A, float alpha, float eps, spla_Descriptor descriptor);
SPLA_API spla_Status spla_Algorithm_tc(int* ntrins, spla_Matrix A, spla_Matrix B, spla_Descriptor descriptor);

//////////////////////////////////////////////////////////////////////////////////////

/* Scheduling and operations execution */

SPLA_API spla_Status spla_Exec_mxmT_masked(spla_Matrix R, spla_Matrix mask, spla_Matrix A, spla_Matrix B, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_mxv_masked(spla_Vector r, spla_Vector mask, spla_Matrix M, spla_Vector v, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_vxm_masked(spla_Vector r, spla_Vector mask, spla_Vector v, spla_Matrix M, spla_OpBinary op_multiply, spla_OpBinary op_add, spla_OpSelect op_select, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_m_reduce_by_row(spla_Vector r, spla_Matrix M, spla_OpBinary op_reduce, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_m_reduce_by_column(spla_Vector r, spla_Matrix M, spla_OpBinary op_reduce, spla_Scalar init, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_m_reduce(spla_Scalar r, spla_Scalar s, spla_Matrix M, spla_OpBinary op_reduce, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_eadd(spla_Vector r, spla_Vector u, spla_Vector v, spla_OpBinary op, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_eadd_fdb(spla_Vector r, spla_Vector v, spla_Vector fdb, spla_OpBinary op, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_assign_masked(spla_Vector r, spla_Vector mask, spla_Scalar value, spla_OpBinary op_assign, spla_OpSelect op_select, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_map(spla_Vector r, spla_Vector v, spla_OpUnary op, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_reduce(spla_Scalar r, spla_Scalar s, spla_Vector v, spla_OpBinary op_reduce, spla_Descriptor desc, spla_ScheduleTask* task);
SPLA_API spla_Status spla_Exec_v_count_mf(spla_Scalar r, spla_Vector v, spla_Descriptor desc, spla_ScheduleTask* task);

//////////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
}
#endif

#endif//SPLA_SPLA_C_H
