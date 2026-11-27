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


class TestVectorAdd(unittest.TestCase):

    def test_eadd(self):
        base_path = Path(__file__).parent / "data" / "vector_add"

        a_file = base_path / "input_0.txt"
        b_file = base_path / "input_1.txt"
        result_file = base_path / "result.txt"

        a = read_vector_from_file(a_file)
        b = read_vector_from_file(b_file)
        expected = read_vector_from_file(result_file)

        result = a.eadd(INT.PLUS, b)

        for idx in range(expected.n_rows):
            self.assertEqual(result.get(idx), expected.get(idx))


class TestVectorFromList(unittest.TestCase):

    def test_from_list(self):
        base_path = Path(__file__).parent / "data" / "vector_from_list"

        input_file = base_path / "input.txt"
        result_file = base_path / "result.txt"

        indices, values = [], []
        with open(input_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    idx, val = line.split()
                    indices.append(int(idx))
                    values.append(int(val))

        size = max(indices) + 1
        v = Vector.from_lists(indices, values, size, INT)
        expected = read_vector_from_file(result_file)

        for idx in range(expected.n_rows):
            self.assertEqual(v.get(idx), expected.get(idx))