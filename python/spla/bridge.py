"""
Wrapped native (spla C API) raw functions access.
"""

__copyright__ = "Copyright (c) 2021-2022 JetBrains-Research"

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
    "_spla_lib",
    "_callback_t"
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

_spla_lib_path = None
_spla_lib = None
_int = None
_enum_t = None
_status_t = None
_object_t = None
_p_object_t = None
_callback_t = None
_default_callback = None

_status_mapping = {
    0: "OK",
    1: "ERROR",
    2: "NO_ACCELERATION",
    3: "PLATFORM_NOT_FOUND",
    4: "DEVICE_NOT_FOUND",
    5: "INVALID_STATE",
    6: "INVALID_ARGUMENT",
    1024: "NOT_IMPLEMENTED",
}


def load_library(lib_path):
    global _spla_lib
    global _int
    global _enum_t
    global _status_t
    global _object_t
    global _p_object_t
    global _callback_t

    _spla_lib = ctypes.cdll.LoadLibrary(str(lib_path))
    _int = ctypes.c_int
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

    _spla_lib.spla_Library_finalize.restype = _status_t
    _spla_lib.spla_Library_finalize.argtypes = []

    _spla_lib.spla_Library_set_accelerator.restype = _status_t
    _spla_lib.spla_Library_set_accelerator.argtypes = [_enum_t]

    _spla_lib.spla_Library_set_platform.restype = _status_t
    _spla_lib.spla_Library_set_platform.argtypes = [_int]

    _spla_lib.spla_Library_set_device.restype = _status_t
    _spla_lib.spla_Library_set_device.argtypes = [_int]

    _spla_lib.spla_Library_set_queues_count.restype = _status_t
    _spla_lib.spla_Library_set_queues_count.argtypes = [_int]

    _spla_lib.spla_Library_set_message_callback.restype = _status_t
    _spla_lib.spla_Library_set_message_callback.argtypes = [_callback_t, ctypes.c_void_p]

    _spla_lib.spla_Library_set_default_callback.restype = _status_t
    _spla_lib.spla_Library_set_default_callback.argtypes = []


def default_callback(status, msg, file, function, line, user_data):
    decoded_msg = msg.decode("utf-8")
    decoded_file = file.decode("utf-8")
    print(f"PySpla: [{decoded_file}:{line}] {_status_mapping[status]}: {decoded_msg}")


def finalize():
    if _spla_lib:
        _spla_lib.spla_Library_finalize()


def initialize():
    global _spla_lib
    global _spla_lib_path
    global _callback_t
    global _default_callback

    try:
        # If generating docs, no lib init required
        if os.environ["SPLA_DOCS"]:
            return
    except KeyError:
        pass

    _spla_lib_path = pathlib.Path(__file__).resolve().parent / TARGET

    try:
        # Override library path from ENV variable (for debug & custom build)
        if os.environ["SPLA_PATH"]:
            _spla_lib_path = pathlib.Path(os.environ["SPLA_PATH"])
    except KeyError:
        pass

    if not _spla_lib_path.is_file():
        # Validate file before loading
        raise Exception(f"no compiled spla file {TARGET} to load")

    load_library(_spla_lib_path)
    _default_callback = _callback_t(default_callback)

    try:
        # If debug enable in ENV, setup default callback for messages on init
        if int(os.environ["SPLA_DEBUG"]):
            _spla_lib.spla_Library_set_message_callback(
                _default_callback, ctypes.c_void_p(0))
    except KeyError:
        pass

    atexit.register(finalize)


def check(status):
    if status != 0:
        raise Exception(f"{_status_mapping[status]}")


# Initialize bridge on import
initialize()
