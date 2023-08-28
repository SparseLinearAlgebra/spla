"""
Wrapped native (spla C API) raw functions access.
"""

__copyright__ = "Copyright (c) 2021-2023 SparseLinearAlgebra"

__license__ = """
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER INs AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

__all__ = [
    "backend",
    "check",
    "is_docs"
]

import os
import ctypes
import pathlib
import platform
import atexit

ARCH = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64'}[platform.machine()]
SYSTEM = {'Darwin': 'macos', 'Linux': 'linux', 'Windows': 'windows'}[platform.system()]
TARGET_SUFFIX = {'macos': '.dylib', 'linux': '.so', 'windows': '.dll'}[SYSTEM]
TARGET = {'macos': 'libspla', 'linux': 'libspla', 'windows': 'spla'}[SYSTEM] + "_" + ARCH + TARGET_SUFFIX

_spla_path = None
_spla = None
_int = None
_enum_t = None
_status_t = None
_object_t = None
_p_object_t = None
_callback_t = None
_default_callback = None
_is_docs = False


class SplaError(Exception):
    pass


class SplaNoAcceleration(SplaError):
    pass


class SplaPlatformNotFound(SplaError):
    pass


class SplaDeviceNotFound(SplaError):
    pass


class SplaInvalidState(SplaError):
    pass


class SplaInvalidArgument(SplaError):
    pass


class SplaNoValue(SplaError):
    pass


class SplaCompilationError(SplaError):
    pass


class SplaNotImplemented(SplaError):
    pass


_status_mapping = {
    1: SplaError,
    2: SplaNoAcceleration,
    3: SplaPlatformNotFound,
    4: SplaDeviceNotFound,
    5: SplaInvalidState,
    6: SplaInvalidArgument,
    7: SplaNoValue,
    1024: SplaNotImplemented,
}


def load_library(lib_path):
    global _spla
    global _int
    global _enum_t
    global _status_t
    global _object_t
    global _p_object_t
    global _callback_t

    _spla = ctypes.cdll.LoadLibrary(str(lib_path))
    _int = ctypes.c_int
    _uint = ctypes.c_uint
    _float = ctypes.c_float
    _p_int = ctypes.POINTER(ctypes.c_int)
    _p_uint = ctypes.POINTER(ctypes.c_uint)
    _p_float = ctypes.POINTER(ctypes.c_float)
    _enum_t = ctypes.c_uint
    _status_t = ctypes.c_uint
    _object_t = ctypes.c_void_p
    _p_object_t = ctypes.POINTER(_object_t)
    _callback_t = ctypes.CFUNCTYPE(None,
                                   ctypes.c_int,
                                   ctypes.c_char_p,
                                   ctypes.c_char_p,
                                   ctypes.c_char_p,
                                   ctypes.c_int,
                                   ctypes.c_void_p)

    _spla.spla_Library_finalize.restype = None
    _spla.spla_Library_finalize.argtypes = []
    _spla.spla_Library_set_accelerator.restype = _status_t
    _spla.spla_Library_set_accelerator.argtypes = [_enum_t]
    _spla.spla_Library_set_platform.restype = _status_t
    _spla.spla_Library_set_platform.argtypes = [_int]
    _spla.spla_Library_set_device.restype = _status_t
    _spla.spla_Library_set_device.argtypes = [_int]
    _spla.spla_Library_set_queues_count.restype = _status_t
    _spla.spla_Library_set_queues_count.argtypes = [_int]
    _spla.spla_Library_set_message_callback.restype = _status_t
    _spla.spla_Library_set_message_callback.argtypes = [_callback_t, ctypes.c_void_p]
    _spla.spla_Library_set_default_callback.restype = _status_t
    _spla.spla_Library_set_default_callback.argtypes = []
    _spla.spla_Library_get_accelerator_info.restype = _status_t
    _spla.spla_Library_get_accelerator_info.argtypes = [ctypes.c_char_p, _int]

    _spla.spla_Type_int.restype = _object_t
    _spla.spla_Type_int.argtypes = []
    _spla.spla_Type_uint.restype = _object_t
    _spla.spla_Type_uint.argtypes = []
    _spla.spla_Type_float.restype = _object_t
    _spla.spla_Type_float.argtypes = []

    _spla.spla_OpBinary_PLUS_INT.restype = _object_t
    _spla.spla_OpBinary_PLUS_UINT.restype = _object_t
    _spla.spla_OpBinary_PLUS_FLOAT.restype = _object_t
    _spla.spla_OpBinary_MINUS_INT.restype = _object_t
    _spla.spla_OpBinary_MINUS_UINT.restype = _object_t
    _spla.spla_OpBinary_MINUS_FLOAT.restype = _object_t
    _spla.spla_OpBinary_MULT_INT.restype = _object_t
    _spla.spla_OpBinary_MULT_UINT.restype = _object_t
    _spla.spla_OpBinary_MULT_FLOAT.restype = _object_t
    _spla.spla_OpBinary_DIV_INT.restype = _object_t
    _spla.spla_OpBinary_DIV_UINT.restype = _object_t
    _spla.spla_OpBinary_DIV_FLOAT.restype = _object_t
    _spla.spla_OpBinary_MINUS_POW2_INT.restype = _object_t
    _spla.spla_OpBinary_MINUS_POW2_UINT.restype = _object_t
    _spla.spla_OpBinary_MINUS_POW2_FLOAT.restype = _object_t
    _spla.spla_OpBinary_FIRST_INT.restype = _object_t
    _spla.spla_OpBinary_FIRST_UINT.restype = _object_t
    _spla.spla_OpBinary_FIRST_FLOAT.restype = _object_t
    _spla.spla_OpBinary_SECOND_INT.restype = _object_t
    _spla.spla_OpBinary_SECOND_UINT.restype = _object_t
    _spla.spla_OpBinary_SECOND_FLOAT.restype = _object_t
    _spla.spla_OpBinary_ONE_INT.restype = _object_t
    _spla.spla_OpBinary_ONE_UINT.restype = _object_t
    _spla.spla_OpBinary_ONE_FLOAT.restype = _object_t
    _spla.spla_OpBinary_MIN_INT.restype = _object_t
    _spla.spla_OpBinary_MIN_UINT.restype = _object_t
    _spla.spla_OpBinary_MIN_FLOAT.restype = _object_t
    _spla.spla_OpBinary_MAX_INT.restype = _object_t
    _spla.spla_OpBinary_MAX_UINT.restype = _object_t
    _spla.spla_OpBinary_MAX_FLOAT.restype = _object_t
    _spla.spla_OpBinary_BOR_INT.restype = _object_t
    _spla.spla_OpBinary_BOR_UINT.restype = _object_t
    _spla.spla_OpBinary_BAND_INT.restype = _object_t
    _spla.spla_OpBinary_BAND_UINT.restype = _object_t
    _spla.spla_OpBinary_BXOR_INT.restype = _object_t
    _spla.spla_OpBinary_BXOR_UINT.restype = _object_t

    _spla.spla_OpBinary_PLUS_INT.argtypes = []
    _spla.spla_OpBinary_PLUS_UINT.argtypes = []
    _spla.spla_OpBinary_PLUS_FLOAT.argtypes = []
    _spla.spla_OpBinary_MINUS_INT.argtypes = []
    _spla.spla_OpBinary_MINUS_UINT.argtypes = []
    _spla.spla_OpBinary_MINUS_FLOAT.argtypes = []
    _spla.spla_OpBinary_MULT_INT.argtypes = []
    _spla.spla_OpBinary_MULT_UINT.argtypes = []
    _spla.spla_OpBinary_MULT_FLOAT.argtypes = []
    _spla.spla_OpBinary_DIV_INT.argtypes = []
    _spla.spla_OpBinary_DIV_UINT.argtypes = []
    _spla.spla_OpBinary_DIV_FLOAT.argtypes = []
    _spla.spla_OpBinary_MINUS_POW2_INT.argtypes = []
    _spla.spla_OpBinary_MINUS_POW2_UINT.argtypes = []
    _spla.spla_OpBinary_MINUS_POW2_FLOAT.argtypes = []
    _spla.spla_OpBinary_FIRST_INT.argtypes = []
    _spla.spla_OpBinary_FIRST_UINT.argtypes = []
    _spla.spla_OpBinary_FIRST_FLOAT.argtypes = []
    _spla.spla_OpBinary_SECOND_INT.argtypes = []
    _spla.spla_OpBinary_SECOND_UINT.argtypes = []
    _spla.spla_OpBinary_SECOND_FLOAT.argtypes = []
    _spla.spla_OpBinary_ONE_INT.argtypes = []
    _spla.spla_OpBinary_ONE_UINT.argtypes = []
    _spla.spla_OpBinary_ONE_FLOAT.argtypes = []
    _spla.spla_OpBinary_MIN_INT.argtypes = []
    _spla.spla_OpBinary_MIN_UINT.argtypes = []
    _spla.spla_OpBinary_MIN_FLOAT.argtypes = []
    _spla.spla_OpBinary_MAX_INT.argtypes = []
    _spla.spla_OpBinary_MAX_UINT.argtypes = []
    _spla.spla_OpBinary_MAX_FLOAT.argtypes = []
    _spla.spla_OpBinary_BOR_INT.argtypes = []
    _spla.spla_OpBinary_BOR_UINT.argtypes = []
    _spla.spla_OpBinary_BAND_INT.argtypes = []
    _spla.spla_OpBinary_BAND_UINT.argtypes = []
    _spla.spla_OpBinary_BXOR_INT.argtypes = []
    _spla.spla_OpBinary_BXOR_UINT.argtypes = []

    _spla.spla_OpSelect_EQZERO_INT.restype = _object_t
    _spla.spla_OpSelect_EQZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_EQZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_NQZERO_INT.restype = _object_t
    _spla.spla_OpSelect_NQZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_NQZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_GTZERO_INT.restype = _object_t
    _spla.spla_OpSelect_GTZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_GTZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_GEZERO_INT.restype = _object_t
    _spla.spla_OpSelect_GEZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_GEZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_LTZERO_INT.restype = _object_t
    _spla.spla_OpSelect_LTZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_LTZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_LEZERO_INT.restype = _object_t
    _spla.spla_OpSelect_LEZERO_UINT.restype = _object_t
    _spla.spla_OpSelect_LEZERO_FLOAT.restype = _object_t
    _spla.spla_OpSelect_ALWAYS_INT.restype = _object_t
    _spla.spla_OpSelect_ALWAYS_UINT.restype = _object_t
    _spla.spla_OpSelect_ALWAYS_FLOAT.restype = _object_t
    _spla.spla_OpSelect_NEVER_INT.restype = _object_t
    _spla.spla_OpSelect_NEVER_UINT.restype = _object_t
    _spla.spla_OpSelect_NEVER_FLOAT.restype = _object_t

    _spla.spla_OpSelect_EQZERO_INT.argtypes = []
    _spla.spla_OpSelect_EQZERO_UINT.argtypes = []
    _spla.spla_OpSelect_EQZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_NQZERO_INT.argtypes = []
    _spla.spla_OpSelect_NQZERO_UINT.argtypes = []
    _spla.spla_OpSelect_NQZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_GTZERO_INT.argtypes = []
    _spla.spla_OpSelect_GTZERO_UINT.argtypes = []
    _spla.spla_OpSelect_GTZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_GEZERO_INT.argtypes = []
    _spla.spla_OpSelect_GEZERO_UINT.argtypes = []
    _spla.spla_OpSelect_GEZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_LTZERO_INT.argtypes = []
    _spla.spla_OpSelect_LTZERO_UINT.argtypes = []
    _spla.spla_OpSelect_LTZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_LEZERO_INT.argtypes = []
    _spla.spla_OpSelect_LEZERO_UINT.argtypes = []
    _spla.spla_OpSelect_LEZERO_FLOAT.argtypes = []
    _spla.spla_OpSelect_ALWAYS_INT.argtypes = []
    _spla.spla_OpSelect_ALWAYS_UINT.argtypes = []
    _spla.spla_OpSelect_ALWAYS_FLOAT.argtypes = []
    _spla.spla_OpSelect_NEVER_INT.argtypes = []
    _spla.spla_OpSelect_NEVER_UINT.argtypes = []
    _spla.spla_OpSelect_NEVER_FLOAT.argtypes = []

    _spla.spla_RefCnt_ref.restype = _status_t
    _spla.spla_RefCnt_unref.restype = _status_t

    _spla.spla_RefCnt_ref.argtypes = [_object_t]
    _spla.spla_RefCnt_unref.argtypes = [_object_t]

    _spla.spla_MemView_make.restype = _status_t
    _spla.spla_MemView_read.restype = _status_t
    _spla.spla_MemView_write.restype = _status_t
    _spla.spla_MemView_get_buffer = _status_t
    _spla.spla_MemView_get_size = _status_t
    _spla.spla_MemView_is_mutable = _status_t

    _spla.spla_MemView_make.argtypes = [_p_object_t, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int]
    _spla.spla_MemView_read.argtypes = [_object_t, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_void_p]
    _spla.spla_MemView_write.argtypes = [_object_t, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_void_p]
    _spla.spla_MemView_get_buffer.argtypes = [_object_t, ctypes.POINTER(ctypes.c_void_p)]
    _spla.spla_MemView_get_size.argtypes = [_object_t, ctypes.POINTER(ctypes.c_size_t)]
    _spla.spla_MemView_is_mutable.argtypes = [_object_t, ctypes.POINTER(ctypes.c_int)]

    _spla.spla_Scalar_make.restype = _status_t
    _spla.spla_Scalar_set_int.restype = _status_t
    _spla.spla_Scalar_set_uint.restype = _status_t
    _spla.spla_Scalar_set_float.restype = _status_t
    _spla.spla_Scalar_get_int.restype = _status_t
    _spla.spla_Scalar_get_uint.restype = _status_t
    _spla.spla_Scalar_get_float.restype = _status_t

    _spla.spla_Scalar_make.argtypes = [_p_object_t, _object_t]
    _spla.spla_Scalar_set_int.argtypes = [_object_t, _int]
    _spla.spla_Scalar_set_uint.argtypes = [_object_t, _uint]
    _spla.spla_Scalar_set_float.argtypes = [_object_t, _float]
    _spla.spla_Scalar_get_int.argtypes = [_object_t, _p_int]
    _spla.spla_Scalar_get_uint.argtypes = [_object_t, _p_uint]
    _spla.spla_Scalar_get_float.argtypes = [_object_t, _p_float]

    _spla.spla_Array_make.restype = _status_t
    _spla.spla_Array_get_n_values.restype = _status_t
    _spla.spla_Array_set_int.restype = _status_t
    _spla.spla_Array_set_uint.restype = _status_t
    _spla.spla_Array_set_float.restype = _status_t
    _spla.spla_Array_get_int.restype = _status_t
    _spla.spla_Array_get_uint.restype = _status_t
    _spla.spla_Array_get_float.restype = _status_t
    _spla.spla_Array_resize.restype = _status_t
    _spla.spla_Array_build.restype = _status_t
    _spla.spla_Array_read.restype = _status_t
    _spla.spla_Array_clear.restype = _status_t

    _spla.spla_Array_make.argtypes = [_p_object_t, _uint, _object_t]
    _spla.spla_Array_get_n_values.argtypes = [_object_t, _p_uint]
    _spla.spla_Array_set_int.argtypes = [_object_t, _uint, _int]
    _spla.spla_Array_set_uint.argtypes = [_object_t, _uint, _uint]
    _spla.spla_Array_set_float.argtypes = [_object_t, _uint, _float]
    _spla.spla_Array_get_int.argtypes = [_object_t, _uint, _p_int]
    _spla.spla_Array_get_uint.argtypes = [_object_t, _uint, _p_uint]
    _spla.spla_Array_get_float.argtypes = [_object_t, _uint, _p_float]
    _spla.spla_Array_resize.argtypes = [_object_t, _uint]
    _spla.spla_Array_build.argtypes = [_object_t, _object_t]
    _spla.spla_Array_read.argtypes = [_object_t, _p_object_t]
    _spla.spla_Array_clear.argtypes = [_object_t]

    _spla.spla_Vector_make.restype = _status_t
    _spla.spla_Vector_set_fill_value.restype = _status_t
    _spla.spla_Vector_set_reduce.restype = _status_t
    _spla.spla_Vector_set_int.restype = _status_t
    _spla.spla_Vector_set_uint.restype = _status_t
    _spla.spla_Vector_set_float.restype = _status_t
    _spla.spla_Vector_get_int.restype = _status_t
    _spla.spla_Vector_get_uint.restype = _status_t
    _spla.spla_Vector_get_float.restype = _status_t
    _spla.spla_Vector_build.restype = _status_t
    _spla.spla_Vector_read.restype = _status_t
    _spla.spla_Vector_clear.restype = _status_t

    _spla.spla_Vector_make.argtypes = [_p_object_t, _uint, _object_t]
    _spla.spla_Vector_set_fill_value.argtypes = [_object_t, _object_t]
    _spla.spla_Vector_set_reduce.argtypes = [_object_t, _object_t]
    _spla.spla_Vector_set_int.argtypes = [_object_t, _uint, _int]
    _spla.spla_Vector_set_uint.argtypes = [_object_t, _uint, _uint]
    _spla.spla_Vector_set_float.argtypes = [_object_t, _uint, _float]
    _spla.spla_Vector_get_int.argtypes = [_object_t, _uint, _p_int]
    _spla.spla_Vector_get_uint.argtypes = [_object_t, _uint, _p_uint]
    _spla.spla_Vector_get_float.argtypes = [_object_t, _uint, _p_float]
    _spla.spla_Vector_build.argtypes = [_object_t, _object_t, _object_t]
    _spla.spla_Vector_read.argtypes = [_object_t, _p_object_t, _p_object_t]
    _spla.spla_Vector_clear.argtypes = [_object_t]

    _spla.spla_Matrix_make.restype = _status_t
    _spla.spla_Matrix_set_fill_value.restype = _status_t
    _spla.spla_Matrix_set_reduce.restype = _status_t
    _spla.spla_Matrix_set_int.restype = _status_t
    _spla.spla_Matrix_set_uint.restype = _status_t
    _spla.spla_Matrix_set_float.restype = _status_t
    _spla.spla_Matrix_get_int.restype = _status_t
    _spla.spla_Matrix_get_uint.restype = _status_t
    _spla.spla_Matrix_get_float.restype = _status_t
    _spla.spla_Matrix_build.restype = _status_t
    _spla.spla_Matrix_read.restype = _status_t
    _spla.spla_Matrix_clear.restype = _status_t

    _spla.spla_Matrix_make.argtypes = [_p_object_t, _uint, _uint, _object_t]
    _spla.spla_Matrix_set_fill_value.argtypes = [_object_t, _object_t]
    _spla.spla_Matrix_set_reduce.argtypes = [_object_t, _object_t]
    _spla.spla_Matrix_set_int.argtypes = [_object_t, _uint, _uint, _int]
    _spla.spla_Matrix_set_uint.argtypes = [_object_t, _uint, _uint, _uint]
    _spla.spla_Matrix_set_float.argtypes = [_object_t, _uint, _uint, _float]
    _spla.spla_Matrix_get_int.argtypes = [_object_t, _uint, _uint, _p_int]
    _spla.spla_Matrix_get_uint.argtypes = [_object_t, _uint, _uint, _p_uint]
    _spla.spla_Matrix_get_float.argtypes = [_object_t, _uint, _uint, _p_float]
    _spla.spla_Matrix_build.argtypes = [_object_t, _object_t, _object_t, _object_t]
    _spla.spla_Matrix_read.argtypes = [_object_t, _p_object_t, _p_object_t, _p_object_t]
    _spla.spla_Matrix_clear.argtypes = [_object_t]

    _spla.spla_Algorithm_bfs.restype = _status_t
    _spla.spla_Algorithm_sssp.restype = _status_t
    _spla.spla_Algorithm_pr.restype = _status_t
    _spla.spla_Algorithm_tc.restype = _status_t

    _spla.spla_Algorithm_bfs.argtypes = [_object_t, _object_t, _uint, _object_t]
    _spla.spla_Algorithm_sssp.argtypes = [_object_t, _object_t, _uint, _object_t]
    _spla.spla_Algorithm_pr.argtypes = [_p_object_t, _object_t, _float, _float, _object_t]
    _spla.spla_Algorithm_tc.argtypes = [_p_int, _object_t, _object_t, _object_t]


def default_callback(status, msg, file, function, line, user_data):
    decoded_msg = msg.decode("utf-8")
    decoded_file = file.decode("utf-8")
    print(f"pyspla: [{decoded_file}:{line}] {_status_mapping[status]}: {decoded_msg}")


def finalize():
    if _spla:
        _spla.spla_Library_finalize()


def initialize():
    global _is_docs
    global _spla
    global _spla_path
    global _callback_t
    global _default_callback

    try:
        # If generating docs, no lib init required
        if os.environ["SPLA_DOCS"]:
            _is_docs = True
            return
    except KeyError:
        pass

    _spla_path = pathlib.Path(__file__).resolve().parent / TARGET

    try:
        # Override library path from ENV variable (for debug & custom build)
        if os.environ["SPLA_PATH"]:
            _spla_path = pathlib.Path(os.environ["SPLA_PATH"])
    except KeyError:
        pass

    if not _spla_path.is_file():
        # Validate file before loading
        raise Exception(f"no compiled spla file {TARGET} to load")

    load_library(_spla_path)
    _default_callback = _callback_t(default_callback)

    try:
        # If debug enable in ENV, setup default callback for messages on init
        if int(os.environ["SPLA_DEBUG"]):
            _spla.spla_Library_set_message_callback(_default_callback, ctypes.c_void_p(0))
    except KeyError:
        pass

    atexit.register(finalize)


def check(status):
    if status != 0:
        raise _status_mapping[status]


def is_docs():
    global _is_docs
    return _is_docs


def backend():
    global _spla
    return _spla
