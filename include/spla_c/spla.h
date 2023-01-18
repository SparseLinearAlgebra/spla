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
 * @brief Spla library C++ API bindings for C language
 *
 * This file contains:
 *
 *  - Status and error codes
 *  - Optional functions hint flags
 *  - Library initialization/finalization API
 *  - Accelerator API
 *  - Matrix manipulation API
 *  - Vector manipulation API
 *  - Schedule operations API
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

/**
 * @brief Status of library operation execution
 */
typedef enum spla_Status {
    /** No error */
    SPLA_STATUS_OK = 0,
    /** Some error occurred */
    SPLA_STATUS_ERROR = 1,
    /** Library has no configured accelerator for computations */
    SPLA_STATUS_NO_ACCELERATION = 2,
    /** Accelerator platform not found */
    SPLA_STATUS_PLATFORM_NOT_FOUND = 3,
    /** Accelerator device not found */
    SPLA_STATUS_DEVICE_NOT_FOUND = 4,
    /** Call of the function is not possible for some context */
    SPLA_STATUS_INVALID_STATE = 5,
    /** Passed invalid argument for some function */
    SPLA_STATUS_INVALID_ARGUMENT = 6,
    /** No such requested value in matrix, vector or scalar storage */
    SPLA_STATUS_NO_VALUE = 7,
    /** Some library feature is not implemented */
    SPLA_STATUS_NOT_IMPLEMENTED = 1024
} spla_Status;

/**
 * @brief Types of supported accelerators for computations
 */
typedef enum spla_AcceleratorType {
    /** No acceleration to be used */
    SPLA_ACCELERATOR_TYPE_NONE = 0,
    /** OpenCL-based single device acceleration */
    SPLA_ACCELERATOR_TYPE_OPENCL = 1
} spla_AcceleratorType;

/**
 * @brief Null handle, used to mark empty spla object
 */
#define SPLA_NULL_HND NULL

/**
 * @brief Handle to any spla object
 */
typedef struct spla_Object_t* spla_Object;

/**
 * @brief Handle to spla descriptor object
 */
typedef struct spla_Descriptor_t* spla_Descriptor;

/**
 * @brief Handle to spla matrix primitive
 */
typedef struct spla_Matrix_t* spla_Matrix;

/**
 * @brief Handle to spla vector primitive
 */
typedef struct spla_Vector_t* spla_Vector;

/**
 * @brief Handle to spla schedule object
 */
typedef struct spla_Schedule_t* spla_Schedule;

/**
 * @brief Handle to spla schedule node object
 */
typedef struct spla_ScheduleTask_t* spla_ScheduleTask;

/**
 * @brief Callback function called on library message event
 */
typedef void(spla_MessageCallback)(spla_Status, const char* message, const char* file, const char* function, int line, void* p_user_data);

/**
 * @brief Finalize library execution
 *
 * Finalize method must be called at the end after application
 * to correctly shutdown global library state, release any
 * enabled acceleration device and release any pending device resources.
 *
 * @warning Must be called after application execution
 * @warning After this call no other library function call is allowed
 */
SPLA_API void spla_Library_finalize();

/**
 * @brief Set accelerator to be used in library computations
 *
 * Sets type of the accelerator to be used in library computations.
 * By default library attempts automatically init OpenCL accelerator
 * if OpenCL runtime present in the system. Set `None` to disable acceleration.
 *
 * @param accelerator Accelerate type
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_accelerator(spla_AcceleratorType accelerator);

/**
 * @brief Selects platform for computations for current accelerator
 *
 * @param index Platform index to select in current PC supported list.
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_platform(int index);

/**
 * @brief Selects device for computations for current accelerator
 *
 * @param index Device index in current platform devices
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_device(int index);

/**
 * @brief Set number of GPU queues for parallel ops execution
 *
 * @param count Number of queues to set
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_queues_count(int count);

/**
 * @brief Set callback function called on library message event
 *
 * @param callback Function to be called
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_message_callback(spla_MessageCallback callback, void* p_user_data);

/**
 * @brief Sets default library callback to log messages to console
 *
 * @return Function call status
 */
SPLA_API spla_Status spla_Library_set_default_callback();

#if defined(__cplusplus)
}
#endif

#endif//SPLA_SPLA_C_H
