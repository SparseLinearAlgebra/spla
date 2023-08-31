"""
Wrapped native (spla C API) type support implementation.
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
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import ctypes

from .bridge import backend
from .op import OpUnary, OpBinary, OpSelect

__all__ = [
    "Type",
    "BOOL",
    "INT",
    "UINT",
    "FLOAT",
    "BUILT_IN"
]


class Type:
    """
    Spla base Type for storage parametrization.

    List of built-in Unary operations
    ---------------------------------

    | Name      | Type     | Meaning        |
    |:----------|:---------|:---------------|
    |`IDENTITY` | unary(x) | r = x          |
    |`AINV`     | unary(x) | r = -x         |
    |`MINV`     | unary(x) | r = 1/x        |
    |`LNOT`     | unary(x) | r = !(x!=0)    |
    |`UONE`     | unary(x) | r = 1          |
    |`ABS`      | unary(x) | r = abs(x)     |
    |`BNOT`     | unary(x) | r = ~x         |
    |`SQRT`     | unary(x) | r = sqrt(x)    |
    |`LOG`      | unary(x) | r = log(x)     |
    |`EXP`      | unary(x) | r = exp(x)     |
    |`SIN`      | unary(x) | r = sin(x)     |
    |`COS`      | unary(x) | r = cos(x)     |
    |`TAN`      | unary(x) | r = tan(x)     |
    |`ASIN`     | unary(x) | r = asin(x)    |
    |`ACOS`     | unary(x) | r = acos(x)    |
    |`ATAN`     | unary(x) | r = atan(x)    |
    |`CEIL`     | unary(x) | r = ceil(x)    |
    |`FLOOR`    | unary(x) | r = floor(x)   |
    |`ROUND`    | unary(x) | r = round(x)   |
    |`TRUNC`    | unary(x) | r = trunc(x)   |

    List of built-in Binary operations
    ----------------------------------

    | Name    | Type        | Meaning        |
    |:--------|:------------|:---------------|
    |`PLUS`   | binary(x,y) | r = x + y      |
    |`MINUS`  | binary(x,y) | r = x - y      |
    |`MULT`   | binary(x,y) | r = x * y      |
    |`DIV`    | binary(x,y) | r = x / y      |
    |`FIRST`  | binary(x,y) | r = x          |
    |`SECOND` | binary(x,y) | r = y          |
    |`BONE`   | binary(x,y) | r = 1          |
    |`MIN`    | binary(x,y) | r = min(x, y)  |
    |`MAX`    | binary(x,y) | r = max(x, y)  |
    |`LOR`    | binary(x,y) | r = x lor y    |
    |`LAND`   | binary(x,y) | r = x && y     |
    |`BOR`    | binary(x,y) | r = x bor y    |
    |`BAND`   | binary(x,y) | r = x & y      |
    |`BXOR`   | binary(x,y) | r = x ^ y      |

    List of built-in Select operations
    ----------------------------------

    | Name    | Type        | Meaning        |
    |:--------|:------------|:---------------|
    |`EQZERO` | select(x)   | x == 0         |
    |`NQZERO` | select(x)   | x != 0         |
    |`GTZERO` | select(x)   | x > 0          |
    |`GEZERO` | select(x)   | x >= 0         |
    |`LTZERO` | select(x)   | x < 0          |
    |`LEZERO` | select(x)   | x <= 0         |
    |`ALWAYS` | select(x)   | true           |
    |`NEVER`  | select(x)   | false          |

    """

    _c_type = None
    _c_type_p = None
    _code = ''
    _scalar_get = None
    _scalar_set = None
    _array_get = None
    _array_set = None
    _vector_get = None
    _vector_set = None
    _matrix_get = None
    _matrix_set = None
    _hnd = None

    IDENTITY: OpUnary
    AINV: OpUnary
    MINV: OpUnary
    LNOT: OpUnary
    UONE: OpUnary
    ABS: OpUnary
    BNOT: OpUnary
    SQRT: OpUnary
    LOG: OpUnary
    EXP: OpUnary
    SIN: OpUnary
    COS: OpUnary
    TAN: OpUnary
    ASIN: OpUnary
    ACOS: OpUnary
    ATAN: OpUnary
    CEIL: OpUnary
    FLOOR: OpUnary
    ROUND: OpUnary
    TRUNC: OpUnary

    PLUS: OpBinary
    MINUS: OpBinary
    MULT: OpBinary
    DIV: OpBinary
    MINUS_POW2: OpBinary
    FIRST: OpBinary
    SECOND: OpBinary
    BONE: OpBinary
    MIN: OpBinary
    MAX: OpBinary
    LOR: OpBinary
    LAND: OpBinary
    BOR: OpBinary
    BAND: OpBinary
    BXOR: OpBinary

    EQZERO: OpSelect
    NQZERO: OpSelect
    GTZERO: OpSelect
    GEZERO: OpSelect
    LTZERO: OpSelect
    LEZERO: OpSelect
    ALWAYS: OpSelect
    NEVER: OpSelect

    @classmethod
    def get_code(cls):
        """
        Literal code of the type in numpy style.
        """
        return cls._code

    @classmethod
    def cast_value(cls, value):
        """
        Transforms native C value into python value.

        :param value: any.
            Ctypes value to unpack to python value.

        :return: Transformed value.
        """
        pass

    @classmethod
    def format_value(cls, value, width=2, precision=2):
        """
        Format value of this type for a pretty printing.

        :param value: any.
            Value to format by this type.

        :param width: optional: int. default: 2.
            Formatted string integral part width.

        :param precision:
            Formatted string fractional part width.

        :return: Formatted value.
        """
        f = "{:>%s}" % width
        if not isinstance(value, bool):
            return f.format(value)
        return f.format("t") if value is True else f.format("f")

    @classmethod
    def _setup_op_unary(cls):
        b = backend()
        type_name = cls.__name__

        def load_op(name):
            f = f"spla_OpUnary_{name}_{type_name}"
            func = getattr(b, f) if hasattr(b, f) else None
            return OpUnary(hnd=func(), name=name, dtype_res=cls, dtype_arg0=cls) if func else None

        cls.IDENTITY = load_op('IDENTITY')
        cls.AINV = load_op('AINV')
        cls.MINV = load_op('MINV')
        cls.LNOT = load_op('LNOT')
        cls.UONE = load_op('UONE')
        cls.ABS = load_op('ABS')
        cls.BNOT = load_op('BNOT')
        cls.SQRT = load_op('SQRT')
        cls.LOG = load_op('LOG')
        cls.EXP = load_op('EXP')
        cls.SIN = load_op('SIN')
        cls.COS = load_op('COS')
        cls.TAN = load_op('TAN')
        cls.ASIN = load_op('ASIN')
        cls.ACOS = load_op('ACOS')
        cls.ATAN = load_op('ATAN')
        cls.CEIL = load_op('CEIL')
        cls.FLOOR = load_op('FLOOR')
        cls.ROUND = load_op('ROUND')
        cls.TRUNC = load_op('TRUNC')

    @classmethod
    def _setup_op_binary(cls):
        b = backend()
        type_name = cls.__name__

        def load_op(name):
            f = f"spla_OpBinary_{name}_{type_name}"
            func = getattr(b, f) if hasattr(b, f) else None
            return OpBinary(hnd=func(), name=name, dtype_res=cls, dtype_arg0=cls, dtype_arg1=cls) if func else None

        cls.PLUS = load_op('PLUS')
        cls.MINUS = load_op('MINUS')
        cls.MULT = load_op('MULT')
        cls.DIV = load_op('DIV')
        cls.MINUS_POW2 = load_op('MINUS_POW2')
        cls.FIRST = load_op('FIRST')
        cls.SECOND = load_op('SECOND')
        cls.BONE = load_op('BONE')
        cls.MIN = load_op('MIN')
        cls.MAX = load_op('MAX')
        cls.LOR = load_op('LOR')
        cls.LAND = load_op('LAND')
        cls.BOR = load_op('BOR')
        cls.BAND = load_op('BAND')
        cls.BXOR = load_op('BXOR')

    @classmethod
    def _setup_op_select(cls):
        b = backend()
        type_name = cls.__name__

        def load_op(name):
            f = f"spla_OpSelect_{name}_{type_name}"
            func = getattr(b, f) if hasattr(b, f) else None
            return OpSelect(hnd=func(), name=name, dtype_arg0=cls) if func else None

        cls.EQZERO = load_op('EQZERO')
        cls.NQZERO = load_op('NQZERO')
        cls.GTZERO = load_op('GTZERO')
        cls.GEZERO = load_op('GEZERO')
        cls.LTZERO = load_op('LTZERO')
        cls.LEZERO = load_op('LEZERO')
        cls.ALWAYS = load_op('ALWAYS')
        cls.NEVER = load_op('NEVER')

    @classmethod
    def _setup(cls):
        b = backend()
        type_name = cls.__name__
        type_name_lower = type_name.lower()

        if not b:
            return

        cls._scalar_get = getattr(b, f"spla_Scalar_get_{type_name_lower}")
        cls._scalar_set = getattr(b, f"spla_Scalar_set_{type_name_lower}")
        cls._array_get = getattr(b, f"spla_Array_get_{type_name_lower}")
        cls._array_set = getattr(b, f"spla_Array_set_{type_name_lower}")
        cls._vector_get = getattr(b, f"spla_Vector_get_{type_name_lower}")
        cls._vector_set = getattr(b, f"spla_Vector_set_{type_name_lower}")
        cls._matrix_get = getattr(b, f"spla_Matrix_get_{type_name_lower}")
        cls._matrix_set = getattr(b, f"spla_Matrix_set_{type_name_lower}")
        cls._hnd = getattr(b, f"spla_Type_{type_name}")()

        cls._setup_op_unary()
        cls._setup_op_binary()
        cls._setup_op_select()


class BOOL(Type):
    """Spla logical BOOL-32 type."""

    _c_type = ctypes.c_bool
    _c_type_p = ctypes.POINTER(ctypes.c_bool)
    _code = 'B'

    @classmethod
    def cast_value(cls, value):
        return bool(value.value)


class INT(Type):
    """Spla integral INT-32 type."""

    _c_type = ctypes.c_int
    _c_type_p = ctypes.POINTER(ctypes.c_int)
    _code = 'I'

    @classmethod
    def cast_value(cls, value):
        return int(value.value)


class UINT(Type):
    """Spla integral UINT-32 type."""

    _c_type = ctypes.c_uint
    _c_type_p = ctypes.POINTER(ctypes.c_uint)
    _code = 'U'

    @classmethod
    def cast_value(cls, value):
        return int(value.value)


class FLOAT(Type):
    """Spla floating-point FLOAT-32 type."""

    _c_type = ctypes.c_float
    _c_type_p = ctypes.POINTER(ctypes.c_float)
    _code = 'F'

    @classmethod
    def cast_value(cls, value):
        return float(value.value)

    @classmethod
    def format_value(cls, value, width=2, precision=2):
        return f"{value:>{width}.{precision}}"


BUILT_IN = [INT, UINT, FLOAT]

for dtype in BUILT_IN:
    dtype._setup()
