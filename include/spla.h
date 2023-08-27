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
 *
 * @brief Spla library C++ API bindings for C language
 *
 * @see Source code: https://github.com/SparseLinearAlgebra/spla
 * @see Python Reference API: https://SparseLinearAlgebra.github.io/spla/docs-python/spla
 * @see C/C++ Reference API: https://SparseLinearAlgebra.github.io/spla/docs-cpp
 */

#if defined(__cplusplus)
    #include <cinttypes>
#else
    #include <inttypes.h>
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

#define SPLA_NULL_HND NULL

typedef uint32_t spla_uint;

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

SPLA_API void        spla_Library_finalize();
SPLA_API spla_Status spla_Library_set_accelerator(spla_AcceleratorType accelerator);
SPLA_API spla_Status spla_Library_set_platform(int index);
SPLA_API spla_Status spla_Library_set_device(int index);
SPLA_API spla_Status spla_Library_set_queues_count(int count);
SPLA_API spla_Status spla_Library_set_message_callback(spla_MessageCallback callback, void* p_user_data);
SPLA_API spla_Status spla_Library_set_default_callback();

SPLA_API spla_Type spla_Type_int();
SPLA_API spla_Type spla_Type_uint();
SPLA_API spla_Type spla_Type_float();

SPLA_API spla_Status spla_Object_ref(spla_Object object);
SPLA_API spla_Status spla_Object_unref(spla_Object object);

SPLA_API spla_Status spla_Scalar_make(spla_Scalar* scalar, spla_Type type);
SPLA_API spla_Status spla_Scalar_set_int(spla_Scalar s, int value);
SPLA_API spla_Status spla_Scalar_set_uint(spla_Scalar s, unsigned int value);
SPLA_API spla_Status spla_Scalar_set_float(spla_Scalar s, float value);
SPLA_API spla_Status spla_Scalar_get_int(spla_Scalar s, int* value);
SPLA_API spla_Status spla_Scalar_get_uint(spla_Scalar s, unsigned int* value);
SPLA_API spla_Status spla_Scalar_get_float(spla_Scalar s, float* value);

SPLA_API spla_Status spla_Array_make(spla_Array* v, spla_uint n_values, spla_Type type);
SPLA_API spla_Status spla_Array_set_int(spla_Array a, spla_uint i, int value);
SPLA_API spla_Status spla_Array_set_uint(spla_Array a, spla_uint i, unsigned int value);
SPLA_API spla_Status spla_Array_set_float(spla_Array a, spla_uint i, float value);
SPLA_API spla_Status spla_Array_get_int(spla_Array a, spla_uint i, int* value);
SPLA_API spla_Status spla_Array_get_uint(spla_Array a, spla_uint i, unsigned int* value);
SPLA_API spla_Status spla_Array_get_float(spla_Array a, spla_uint i, float* value);
SPLA_API spla_Status spla_Array_clear(spla_Array a);

SPLA_API spla_Status spla_Vector_make(spla_Vector* v, spla_uint n_rows, spla_Type type);
SPLA_API spla_Status spla_Vector_set_fill_value(spla_Vector v, spla_Scalar value);
SPLA_API spla_Status spla_Vector_set_reduce(spla_Vector v, spla_OpBinary reduce);
SPLA_API spla_Status spla_Vector_set_int(spla_Vector v, spla_uint row_id, int value);
SPLA_API spla_Status spla_Vector_set_uint(spla_Vector v, spla_uint row_id, unsigned int value);
SPLA_API spla_Status spla_Vector_set_float(spla_Vector v, spla_uint row_id, float value);
SPLA_API spla_Status spla_Vector_get_int(spla_Vector v, spla_uint row_id, int* value);
SPLA_API spla_Status spla_Vector_get_uint(spla_Vector v, spla_uint row_id, unsigned int* value);
SPLA_API spla_Status spla_Vector_get_float(spla_Vector v, spla_uint row_id, float* value);
SPLA_API spla_Status spla_Vector_clear(spla_Vector v);

SPLA_API spla_Status spla_Matrix_make(spla_Matrix* M, spla_uint n_rows, spla_Type type);
SPLA_API spla_Status spla_Matrix_set_fill_value(spla_Matrix M, spla_Scalar value);
SPLA_API spla_Status spla_Matrix_set_reduce(spla_Matrix M, spla_OpBinary reduce);
SPLA_API spla_Status spla_Matrix_set_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int value);
SPLA_API spla_Status spla_Matrix_set_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int value);
SPLA_API spla_Status spla_Matrix_set_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float value);
SPLA_API spla_Status spla_Matrix_get_int(spla_Matrix M, spla_uint row_id, spla_uint col_id, int* value);
SPLA_API spla_Status spla_Matrix_get_uint(spla_Matrix M, spla_uint row_id, spla_uint col_id, unsigned int* value);
SPLA_API spla_Status spla_Matrix_get_float(spla_Matrix M, spla_uint row_id, spla_uint col_id, float* value);
SPLA_API spla_Status spla_Matrix_clear(spla_Matrix M);

#if defined(__cplusplus)
}
#endif

#endif//SPLA_SPLA_C_H
