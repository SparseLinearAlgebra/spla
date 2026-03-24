import unittest
from pathlib import Path

from pyspla import INT, Vector


def read_vector_from_file(filepath):
    indices = []
    values = []

    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                idx, val = line.split()
                indices.append(int(idx))
                values.append(int(val))

    size = max(indices) + 1 if indices else 5
    return Vector.from_lists(indices, values, size, INT)


class TestOperations(unittest.TestCase):

    def test_unary_ops(self):
        base_path = Path(__file__).parent / "data" / "op_unary"

        input_file = base_path / "input.txt"
        abs_file = base_path / "result_abs.txt"
        ainv_file = base_path / "result_ainv.txt"

        v = read_vector_from_file(input_file)

        result_abs = v.map(INT.ABS)
        expected_abs = read_vector_from_file(abs_file)
        self.assertEqual(result_abs.to_lists(), expected_abs.to_lists())

        result_ainv = v.map(INT.AINV)
        expected_ainv = read_vector_from_file(ainv_file)
        self.assertEqual(result_ainv.to_lists(), expected_ainv.to_lists())

    def test_binary_ops(self):
        base_path = Path(__file__).parent / "data" / "op_binary"

        a_file = base_path / "input_0.txt"
        b_file = base_path / "input_1.txt"
        plus_file = base_path / "result_plus.txt"
        mult_file = base_path / "result_mult.txt"
        max_file = base_path / "result_max.txt"

        a = read_vector_from_file(a_file)
        b = read_vector_from_file(b_file)

        result_plus = a.eadd(INT.PLUS, b)
        expected_plus = read_vector_from_file(plus_file)
        for idx in range(expected_plus.n_rows):
            self.assertEqual(result_plus.get(idx), expected_plus.get(idx))

        result_mult = a.emult(INT.MULT, b)
        expected_mult = read_vector_from_file(mult_file)
        for idx in range(expected_mult.n_rows):
            self.assertEqual(result_mult.get(idx), expected_mult.get(idx))

        result_max = a.eadd(INT.MAX, b)
        expected_max = read_vector_from_file(max_file)
        for idx in range(expected_max.n_rows):
            self.assertEqual(result_max.get(idx), expected_max.get(idx))