import unittest
from pathlib import Path

from pyspla import INT, FLOAT, Scalar


def read_scalar_from_file(filepath):
    with open(filepath, 'r') as f:
        line = f.readline().strip()
        if '.' in line:
            return float(line)
        else:
            return int(line)


class TestScalar(unittest.TestCase):

    def test_scalar_creation(self):
        s = Scalar(INT, 10)
        self.assertEqual(s.get(), 10)

        s = Scalar(FLOAT, 3.14)
        self.assertAlmostEqual(s.get(), 3.14, places=5)

    def test_scalar_set_get(self):
        s = Scalar(INT)
        s.set(5)
        self.assertEqual(s.get(), 5)

        s.set(42)
        self.assertEqual(s.get(), 42)

    def test_scalar_operations(self):
        base_path = Path(__file__).parent / "data" / "scalar_operations"

        a_file = base_path / "input_0.txt"
        b_file = base_path / "input_1.txt"
        plus_file = base_path / "result_plus.txt"
        sub_file = base_path / "result_sub.txt"
        mul_file = base_path / "result_mult.txt"
        div_file = base_path / "result_div.txt"

        a_val = read_scalar_from_file(a_file)
        b_val = read_scalar_from_file(b_file)
        a = Scalar(INT, a_val)
        b = Scalar(INT, b_val)

        self.assertEqual((a + b).get(), read_scalar_from_file(plus_file))
        self.assertEqual((a - b).get(), read_scalar_from_file(sub_file))
        self.assertEqual((a * b).get(), read_scalar_from_file(mul_file))
        self.assertEqual((a // b).get(), read_scalar_from_file(div_file))