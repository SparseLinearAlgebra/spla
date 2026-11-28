"""Tests for the pyspla package: containers, ops, and error handling."""

import pytest

from pyspla import (
    FLOAT,
    INT,
    UINT,
    Descriptor,
    FormatVector,
    Matrix,
    Scalar,
    Vector,
)

# 4-vertex directed graph from the package docs:
#   0 -> 1, 1 -> 2, 2 -> 0, 2 -> 3, 3 -> 2
GRAPH_I = [0, 1, 2, 2, 3]
GRAPH_J = [1, 2, 0, 3, 2]
GRAPH_V = [1, 1, 1, 1, 1]


@pytest.fixture(scope="module")
def graph() -> Matrix:
    return Matrix.from_lists(GRAPH_I, GRAPH_J, GRAPH_V, shape=(4, 4), dtype=INT)


class TestScalar:
    def test_set_get_each_dtype(self):
        # BOOL scalars are not supported by the C bindings (no set/get function)
        for dtype, value in ((INT, -42), (UINT, 7), (FLOAT, 2.5)):
            s = Scalar(dtype, value)
            assert s.get() == value

    def test_set_after_construction(self):
        s = Scalar(INT, 1)
        s.set(9)
        assert s.get() == 9

    def test_arithmetic(self):
        a, b = Scalar(INT, 6), Scalar(INT, 4)
        assert (a + b).get() == 10
        assert (a - b).get() == 2
        assert (a * b).get() == 24
        assert (a // b).get() == 1
        # true division yields a float, so it is only representable for FLOAT scalars
        fa, fb = Scalar(FLOAT, 6.0), Scalar(FLOAT, 4.0)
        assert (fa / fb).get() == 1.5

    def test_inplace_arithmetic(self):
        a = Scalar(INT, 6)
        a += 4
        assert a.get() == 10
        a -= 4
        assert a.get() == 6
        a *= 2
        assert a.get() == 12

    def test_mixed_scalar_and_python_value(self):
        s = Scalar(FLOAT, 2.0)
        assert (s + 3.0).get() == 5.0
        assert (s * 3).get() == 6.0


class TestVector:
    def test_empty_construction(self):
        v = Vector(5, INT)
        assert v.n_rows == 5
        assert v.shape == (5, 1)
        assert v.dtype == INT
        assert v.to_lists() == ([], [])

    def test_set_get(self):
        v = Vector(4, INT)
        v.set(1, 10)
        v.set(3, 20)
        assert v.get(1) == 10
        assert v.get(3) == 20
        assert v.get(0) == 0  # unset entries read as zero

    def test_from_lists_roundtrip(self):
        v = Vector.from_lists([0, 2], [10, 20], 4, INT)
        assert v.to_lists() == ([0, 2], [10, 20])
        assert v.to_list() == [(0, 10), (2, 20)]

    def test_float_values(self):
        v = Vector.from_lists([0, 1], [1.5, -2.25], 2, FLOAT)
        assert v.to_lists() == ([0, 1], [1.5, -2.25])

    def test_fill_with(self):
        v = Vector(4, INT)
        v.fill_with(Scalar(INT, 7))
        assert v.to_list() == [(0, 7), (1, 7), (2, 7), (3, 7)]

    def test_set_fill_value(self):
        # explicit entries must be preserved when the fill value changes
        v = Vector.from_lists([0], [10], 4, INT)
        v.set_fill_value(Scalar(INT, -1))
        assert v.to_list() == [(0, 10)]
        assert v.get(1) == -1

    def test_reduce(self):
        v = Vector.from_lists([0, 1, 2], [1, 2, 3], 3, INT)
        assert v.reduce(op_reduce=INT.PLUS).get() == 6
        assert v.reduce(op_reduce=INT.MAX).get() == 3
        # reduction starts from `init` (default 0), so MIN needs a proper neutral value
        assert v.reduce(op_reduce=INT.MIN, init=Scalar(INT, 100)).get() == 1

    def test_eadd_emult(self):
        # eadd/emult are element-wise: both operands must have the same length
        v = Vector.from_lists([0, 1], [1, 2], 3, INT)
        w = Vector.from_lists([1, 2], [10, 20], 3, INT)
        s = v.eadd(INT.PLUS, w)
        assert s.to_list() == [(0, 1), (1, 12), (2, 20)]
        p = v.emult(INT.MULT, w)
        assert p.to_list() == [(1, 20)]

    def test_assign_masked(self):
        v = Vector(4, INT)
        mask = Vector.from_lists([1, 3], [1, 1], 4, INT)
        v.assign(mask, Scalar(INT, 5), op_assign=INT.SECOND, op_select=INT.NQZERO)
        assert v.to_list() == [(1, 5), (3, 5)]

    def test_map_unary(self):
        v = Vector.from_lists([0, 1], [3, -4], 2, INT)
        neg = v.map(INT.AINV)
        assert neg.to_list() == [(0, -3), (1, 4)]
        abs_ = v.map(INT.ABS)
        assert abs_.to_list() == [(0, 3), (1, 4)]

    def test_count_mf(self):
        v = Vector.from_lists([0, 2], [1, 1], 4, INT)
        assert v.count_mf().get() == 2

    def test_vxm_masked(self):
        # BFS-style traversal: front x A, keep only unvisited targets
        A = Matrix.from_lists(GRAPH_I, GRAPH_J, GRAPH_V, shape=(4, 4), dtype=INT)
        front = Vector.from_lists([0], [1], 4, INT)
        visited = Vector(4, INT)
        nxt = front.vxm(visited, A, op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO)
        assert nxt.to_list() == [(1, 1)]

    def test_format_conversion(self):
        v = Vector.from_lists([0, 2], [10, 20], 4, INT)
        v.set_format(FormatVector.CPU_DENSE)
        # reading back goes through the sparse representation, which drops fill values
        assert v.to_list() == [(0, 10), (2, 20)]
        assert v.get(1) == 0


class TestMatrix:
    def test_from_lists_roundtrip(self, graph):
        i, j, v = graph.to_lists()
        assert sorted(zip(i, j, v)) == sorted(zip(GRAPH_I, GRAPH_J, GRAPH_V))

    def test_shape_properties(self, graph):
        assert graph.n_rows == 4
        assert graph.n_cols == 4
        assert graph.shape == (4, 4)

    def test_set_get(self):
        m = Matrix((3, 3), INT)
        m.set(0, 1, 5)
        assert m.get(0, 1) == 5
        assert m.get(1, 0) == 0

    def test_mxm(self, graph):
        # A * A: two-hop reachability
        r = graph.mxm(graph, op_mult=INT.MULT, op_add=INT.PLUS)
        i, j, v = r.to_lists()
        assert sorted(zip(i, j)) == [(0, 2), (1, 0), (1, 3), (2, 1), (2, 2), (3, 0), (3, 3)]
        assert all(x == 1 for x in v)

    def test_mxmT_masked(self, graph):
        # R = mask * (A * A^T): common out-neighbors per masked position.
        # The mask must not alias the operands: the kernel writes the result
        # into the same LIL rows it reads from.
        mask = graph.eadd(INT.PLUS, Matrix.diag((4, 4), INT, diag_value=1))
        r = graph.mxmT(mask, graph, op_mult=INT.MULT, op_add=INT.PLUS, op_select=INT.GTZERO)
        i, j, v = r.to_lists()
        # diagonal of A * A^T: out-degree of each vertex; some backends keep
        # explicit zeros at the other masked positions
        got = dict(zip(zip(i, j), v))
        expected = {(0, 0): 1, (1, 1): 1, (2, 2): 2, (3, 3): 1}
        for key, value in expected.items():
            assert got.get(key) == value
        for key, value in got.items():
            if key not in expected:
                assert value == 0

    def test_mxv(self, graph):
        v = Vector.from_lists([0, 1, 2, 3], [1, 2, 3, 4], 4, INT)
        r = graph.mxv(v, v, op_mult=INT.MULT, op_add=INT.PLUS, op_select=INT.ALWAYS)
        assert r.to_list() == [(0, 2), (1, 3), (2, 5), (3, 3)]

    def test_vxm(self, graph):
        v = Vector.from_lists([0, 1, 2, 3], [1, 2, 3, 4], 4, INT)
        r = v.vxm(v, graph, op_mult=INT.MULT, op_add=INT.PLUS, op_select=INT.ALWAYS)
        assert r.to_list() == [(0, 3), (1, 1), (2, 6), (3, 3)]

    def test_transpose(self, graph):
        t = graph.transpose()
        i, j, _ = t.to_lists()
        assert sorted(zip(i, j)) == sorted(zip(GRAPH_J, GRAPH_I))

    def test_eadd_emult(self, graph):
        other = Matrix.from_lists([0, 0], [1, 3], [10, 20], shape=(4, 4), dtype=INT)
        s = graph.eadd(INT.PLUS, other)
        assert s.get(0, 1) == 11
        assert s.get(0, 3) == 20
        p = graph.emult(INT.MULT, other)
        assert p.get(0, 1) == 10
        assert p.get(1, 2) == 0  # absent in `other`

    def test_reduce_by_row(self, graph):
        r = graph.reduce_by_row(op_reduce=INT.PLUS)
        assert r.to_list() == [(0, 1), (1, 1), (2, 2), (3, 1)]

    def test_reduce_by_column(self, graph):
        r = graph.reduce_by_column(op_reduce=INT.PLUS)
        assert r.to_list() == [(0, 1), (1, 1), (2, 2), (3, 1)]

    def test_reduce(self, graph):
        assert graph.reduce(op_reduce=INT.PLUS).get() == 5

    def test_extract_row(self, graph):
        r = graph.extract_row(2)
        i, v = r.to_lists()
        assert list(zip(i, v)) == [(0, 1), (3, 1)]

    def test_extract_column(self, graph):
        c = graph.extract_column(2)
        i, v = c.to_lists()
        assert list(zip(i, v)) == [(1, 1), (3, 1)]

    def test_kron(self):
        a = Matrix.from_lists([0, 1], [1, 0], [1, 2], shape=(2, 2), dtype=INT)
        r = a.kron(a, op_mult=INT.MULT)
        i, j, v = r.to_lists()
        assert dict(zip(zip(i, j), v)) == {(0, 3): 1, (1, 2): 2, (2, 1): 2, (3, 0): 4}

    def test_diag(self):
        d = Matrix.diag((3, 3), INT, diag_value=2)
        i, j, v = d.to_lists()
        assert sorted(zip(i, j, v)) == [(0, 0, 2), (1, 1, 2), (2, 2, 2)]

    def test_dense(self):
        d = Matrix.dense((3, 3), INT, fill_value=5)
        assert d.to_list() == [(i, j, 5) for i in range(3) for j in range(3)]


class TestDescriptor:
    def test_make_and_setters(self):
        d = Descriptor()
        d.set_struct_only(True)
        d.set_early_exit(False)
        # a descriptor must be usable in an operation
        v = Vector.from_lists([0], [1], 2, INT)
        assert v.reduce(op_reduce=INT.PLUS, desc=d).get() == 1


class TestErrors:
    # Note: out-of-range set() indices are not validated by the C library in
    # release builds (bounds checks are asserts only), so they are not tested.

    def test_shape_mismatch_mxm(self, graph):
        other = Matrix.from_lists([0], [0], [1], shape=(3, 3), dtype=INT)
        with pytest.raises(AssertionError):
            graph.mxm(other, op_mult=INT.MULT, op_add=INT.PLUS)

    def test_dtype_mismatch_eadd(self):
        v = Vector.from_lists([0], [1], 2, INT)
        w = Vector.from_lists([0], [1.5], 2, FLOAT)
        with pytest.raises(AssertionError):
            v.eadd(INT.PLUS, w)
