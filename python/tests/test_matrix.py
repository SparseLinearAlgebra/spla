import unittest
from pathlib import Path

from pyspla import INT, Matrix, Vector


def read_matrix_from_file(filepath):
    I = []
    J = []
    V = []

    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                i, j, v = line.split()
                I.append(int(i))
                J.append(int(j))
                V.append(int(v))

    n_rows = max(I) + 1 if I else 3
    n_cols = max(J) + 1 if J else 3

    return Matrix.from_lists(I, J, V, (n_rows, n_cols), INT)


class TestMatrix(unittest.TestCase):

    def test_matrix_creation(self):
        M = Matrix((3, 4), INT)
        self.assertEqual(M.n_rows, 3)
        self.assertEqual(M.n_cols, 4)
        self.assertEqual(M.shape, (3, 4))

    def test_matrix_set_get(self):
        M = Matrix((3, 3), INT)
        M.set(0, 1, 5)
        M.set(1, 2, 10)
        M.set(2, 0, 15)

        self.assertEqual(M.get(0, 1), 5)
        self.assertEqual(M.get(1, 2), 10)
        self.assertEqual(M.get(2, 0), 15)
        self.assertEqual(M.get(1, 1), 0)

    def test_matrix_eadd(self):
        base_path = Path(__file__).parent / "data" / "matrix_eadd"

        A = read_matrix_from_file(base_path / "input_0.txt")
        B = read_matrix_from_file(base_path / "input_1.txt")
        expected = read_matrix_from_file(base_path / "result.txt")

        result = A.eadd(INT.PLUS, B)
        self.assertEqual(result.to_lists(), expected.to_lists())

    def test_matrix_transpose(self):
        base_path = Path(__file__).parent / "data" / "matrix_transpose"

        M = read_matrix_from_file(base_path / "input.txt")
        expected = read_matrix_from_file(base_path / "result.txt")

        result = M.transpose()

        for i in range(result.n_rows):
            for j in range(result.n_cols):
                self.assertEqual(result.get(i, j), expected.get(i, j))

    def test_matrix_reduce_by_row(self):
        base_path = Path(__file__).parent / "data" / "matrix_reduce_by_row"

        M = read_matrix_from_file(base_path / "input_0.txt")

        I_vec, V_vec = [], []
        with open(base_path / "result.txt", 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    idx, val = line.split()
                    I_vec.append(int(idx))
                    V_vec.append(int(val))

        size = max(I_vec) + 1 if I_vec else 3
        expected = Vector.from_lists(I_vec, V_vec, size, INT)

        result = M.reduce_by_row(INT.PLUS)
        self.assertEqual(result.to_lists(), expected.to_lists())