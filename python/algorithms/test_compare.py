import unittest
import math
from collections import defaultdict
from pyspla import Matrix, INT, FLOAT

from bfs_classic import bfs as bfs_c, INF as BFS_INF
from sssp_classic import sssp as sssp_c, INF as SSSP_INF
from pr_classic import pagerank_classic as pr_c
from tc_classic import tc_simple as tc_c

from bfs_spla import bfs as bfs_s
from sssp_spla import sssp as sssp_s
from pr_spla import pagerank as pr_s
from tc_spla import cohen as tc_s

class TestCompareAlgorithms(unittest.TestCase):

    # ==========================================
    # 1. ПРОСТЫЕ ФУНКЦИИ СОЗДАНИЯ ГРАФОВ
    # ==========================================

    def build_bfs_graph(self, edges, n):
        graph_c = defaultdict(list)
        I, J, V = [], [], []
        for u, v in edges:
            graph_c[u].append(v)
            graph_c[v].append(u)
            I.extend([u, v])
            J.extend([v, u])
            V.extend([1, 1])
        return graph_c, Matrix.from_lists(I, J, V, shape=(n, n), dtype=INT)

    def build_sssp_graph(self, edges, n):
        graph_c = defaultdict(list)
        I, J, V = [], [], []
        for u, v, w in edges:
            graph_c[u].append((v, w))
            graph_c[v].append((u, w))
            I.extend([u, v])
            J.extend([v, u])
            V.extend([w, w])
        return graph_c, Matrix.from_lists(I, J, V, shape=(n, n), dtype=INT)

    def build_tc_graph(self, edges, n):
        graph_c = defaultdict(list)
        I, J, V = [], [], []
        for u, v in edges:
            graph_c[u].append(v)
            graph_c[v].append(u)
            I.extend([u, v])
            J.extend([v, u])
            V.extend([1, 1])
        return graph_c, Matrix.from_lists(I, J, V, shape=(n, n), dtype=INT)

    def build_pr_graph(self, edges, n, alpha=0.85):
        adj_in = defaultdict(list)
        out_degree = [0] * n
        I, J, V = [], [], []
        
        # Сначала считаем исходящие ребра
        for u, v in edges:
            out_degree[u] += 1
            if u != v:
                out_degree[v] += 1
                
        # Затем строим графы
        for u, v in edges:
            adj_in[v].append(u)
            I.append(u); J.append(v); V.append(alpha / out_degree[u])
            if u != v:
                adj_in[u].append(v)
                I.append(v); J.append(u); V.append(alpha / out_degree[v])
                
        return adj_in, out_degree, Matrix.from_lists(I, J, V, shape=(n, n), dtype=FLOAT)

    # ==========================================
    # 2. ФУНКЦИИ СРАВНЕНИЯ РЕЗУЛЬТАТОВ
    # ==========================================

    def check_lists_equal(self, list1, list2):
        self.assertEqual(len(list1), len(list2))
        for k in range(len(list1)):
            self.assertEqual(list1[k], list2[k], f"Mismatch at index {k}: {list1[k]} != {list2[k]}")

    def check_lists_close(self, list1, list2, tol=1e-3):
        self.assertEqual(len(list1), len(list2))
        for k in range(len(list1)):
            self.assertTrue(math.isclose(list1[k], list2[k], rel_tol=tol), f"Mismatch at index {k}: {list1[k]} != {list2[k]}")

    # ==========================================
    # 3. ТЕСТЫ: BFS (Обход в ширину)
    # ==========================================

    def run_bfs_test(self, edges, n, start=0):
        graph_c, A_s = self.build_bfs_graph(edges, n)
        dist_c = bfs_c(start, graph_c, n)
        v_s, _, _ = bfs_s(start, A_s)
        idx, vals = v_s.to_lists()
        dist_s = [BFS_INF] * n
        for k in range(len(idx)):
            dist_s[idx[k]] = vals[k] - 1
        self.check_lists_equal(dist_c, dist_s)

    def test_bfs_linear(self):
        self.run_bfs_test([(0, 1), (1, 2), (2, 3), (3, 4)], n=5)

    def test_bfs_star(self):
        self.run_bfs_test([(0, 1), (0, 2), (0, 3), (0, 4)], n=5)

    def test_bfs_disconnected(self):
        self.run_bfs_test([(0, 1), (2, 3)], n=4)

    def test_bfs_cycle(self):
        self.run_bfs_test([(0, 1), (1, 2), (2, 3), (3, 0)], n=4)

    def test_bfs_fully_connected(self):
        self.run_bfs_test([(0,1), (0,2), (0,3), (1,2), (1,3), (2,3)], n=4)

    # ==========================================
    # 4. ТЕСТЫ: SSSP (Кратчайшие пути)
    # ==========================================

    def run_sssp_test(self, edges, n, start=0):
        graph_c, A_s = self.build_sssp_graph(edges, n)
        dist_c = sssp_c(start, graph_c, n)
        v_s = sssp_s(start, A_s)
        idx, vals = v_s.to_lists()
        dist_s = [int(1e9)] * n
        for k in range(len(idx)):
            dist_s[idx[k]] = vals[k]
        self.check_lists_equal(dist_c, dist_s)

    def test_sssp_linear(self):
        self.run_sssp_test([(0, 1, 5), (1, 2, 2), (2, 3, 1)], n=4)

    def test_sssp_triangle_inequality(self):
        # Путь 0->2 напрямую стоит 10, но в обход через 1 стоит 5+1=6
        self.run_sssp_test([(0, 1, 5), (0, 2, 10), (1, 2, 1)], n=3)

    def test_sssp_isolated(self):
        self.run_sssp_test([(0, 1, 3)], n=4) # Вершины 2 и 3 изолированы

    def test_sssp_two_paths(self):
        # Два пути из 0 в 3. Верхний: 0->1->3 (вес 10+10=20). Нижний: 0->2->3 (вес 2+2=4).
        self.run_sssp_test([(0, 1, 10), (1, 3, 10), (0, 2, 2), (2, 3, 2)], n=4)

    def test_sssp_complex_graph(self):
        edges = [(0, 1, 4), (0, 2, 1), (2, 1, 2), (1, 3, 1), (2, 3, 5), (3, 4, 3)]
        self.run_sssp_test(edges, n=5)

    # ==========================================
    # 5. ТЕСТЫ: TC (Подсчет треугольников)
    # ==========================================

    def run_tc_test(self, edges, n):
        graph_c, A_s = self.build_tc_graph(edges, n)
        for i in range(n): graph_c[i].sort()
        self.assertEqual(tc_c(graph_c, n), tc_s(A_s))

    def test_tc_k4(self):
        self.run_tc_test([(0,1), (0,2), (0,3), (1,2), (1,3), (2,3)], n=4)

    def test_tc_bipartite(self):
        self.run_tc_test([(0,1), (1,2), (2,3), (3,0)], n=4) # 0 треугольников

    def test_tc_two_triangles(self):
        self.run_tc_test([(0,1), (1,2), (2,0), (3,4), (4,5), (5,3)], n=6) # 2 треугольника

    def test_tc_empty(self):
        self.run_tc_test([], n=3) # 0 треугольников

    def test_tc_one_edge(self):
        self.run_tc_test([(0,1)], n=3) # 0 треугольников

    # ==========================================
    # 6. ТЕСТЫ: PageRank
    # ==========================================

    def run_pr_test(self, edges, n, alpha=0.85, eps=1e-5):
        adj_in, out_degree, A_s = self.build_pr_graph(edges, n, alpha)
        p_c, _ = pr_c(adj_in, out_degree, n, alpha, eps)
        p_s_vec, _ = pr_s(A_s, alpha, eps)
        idx, vals = p_s_vec.to_lists()
        p_s = [0.0] * n
        for k in range(len(idx)):
            p_s[idx[k]] = vals[k]
        self.check_lists_close(p_c, p_s, tol=1e-3)

    def test_pr_star(self):
        self.run_pr_test([(1, 0), (2, 0), (3, 0)], n=4)

    def test_pr_chain(self):
        self.run_pr_test([(0, 1), (1, 2)], n=3)

    def test_pr_complete(self):
        self.run_pr_test([(0,1), (0,2), (0,3), (1,2), (1,3), (2,3)], n=4)

    def test_pr_disconnected(self):
        self.run_pr_test([(0, 1), (2, 3)], n=4)

if __name__ == '__main__':
    unittest.main()
c
