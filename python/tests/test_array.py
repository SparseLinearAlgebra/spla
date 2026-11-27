import unittest
from pyspla import INT, Array


class TestArray(unittest.TestCase):

    def test_array_creation(self):
        a = Array(INT, 5)
        self.assertEqual(a.n_vals, 5)
        self.assertEqual(a.shape, (5, 1))

    def test_array_set_get(self):
        a = Array(INT, 5)
        a.set(0, 10)
        a.set(2, 20)
        a.set(4, 30)

        self.assertEqual(a.get(0), 10)
        self.assertEqual(a.get(2), 20)
        self.assertEqual(a.get(4), 30)
        self.assertEqual(a.get(1), 0)

    def test_array_from_list(self):
        values = [1, 2, 3, 4, 5]
        a = Array.from_list(values, INT)

        self.assertEqual(a.n_vals, 5)
        self.assertEqual(a.to_list(), values)

    def test_array_resize_clear(self):
        a = Array.from_list([1, 2, 3], INT)
        self.assertEqual(a.n_vals, 3)

        a.resize(5)
        self.assertEqual(a.n_vals, 5)

        a.clear()
        self.assertEqual(a.n_vals, 0)
        self.assertTrue(a.empty)