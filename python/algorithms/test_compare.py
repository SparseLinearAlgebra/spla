import warnings

warnings.simplefilter("ignore", DeprecationWarning)

import heapq
import math
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from pyspla import FLOAT, INT
from graph_spla import read_spla, read_mtx_pr_spla
from bfs_spla import bfs_spla
from bfs_classic import bfs_naive as bfs_classic
from sssp_spla import sssp_spla
from pr_spla import pr_spla
from pr_classic import pagerank_naive as pr_classic
from tc_spla import tc_spla
from tc_classic import tc_naive as tc_classic


DATA = Path(__file__).parent.parent.parent / "bench" / "dataset"
UNDIRECTED_GRAPH = DATA / "belgium_osm.mtx"
DIRECTED_GRAPH = DATA / "amazon-2008.mtx"
UPPER = DATA / "upper" / "belgium_osm.mtx"


def split(A):
    i, j, v = A.to_lists()
    return A.n_rows, list(zip(i, j)), list(v)


def to_list(v, n, fill):
    i, x = v.to_lists()
    out = [fill] * n
    for k, val in zip(i, x):
        out[k] = val
    return out


def adj_plain(n, pairs):
    adj = [[] for _ in range(n)]
    for u, v in pairs:
        adj[u].append(v)
    return adj


def adj_weighted(n, pairs, weights):
    adj = [[] for _ in range(n)]
    for (u, v), w in zip(pairs, weights):
        adj[u].append((v, w))
    return adj


def short_distance(n, adj, start):
    dist = [math.inf] * n
    dist[start] = 0.0

    queue = [(0.0, start)]
    while queue:
        d, u = heapq.heappop(queue)
        if d > dist[u]:
            continue
        for v, w in adj[u]:
            new = d + w
            if new < dist[v]:
                dist[v] = new
                heapq.heappush(queue, (new, v))

    return dist


class TestBfs(unittest.TestCase):
    def _check(self, mtx, **kwargs):
        A = read_spla(str(mtx), INT)
        n, pairs, _ = split(A)

        ref = bfs_classic(0, adj_plain(n, pairs), n)
        ref = [(x + 1) if x is not None else 0 for x in ref]

        v, _, _ = bfs_spla(0, A, **kwargs)
        got = to_list(v, n, 0)

        self.assertEqual(ref, got)

    def test_symmetric_push(self):
        self._check(UNDIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)

    def test_symmetric_pull(self):
        self._check(UNDIRECTED_GRAPH, push_only=False, pull_only=True, push_pull=False)

    def test_symmetric_push_pull(self):
        self._check(UNDIRECTED_GRAPH, push_only=False, pull_only=False, push_pull=True)

    def test_directed_push(self):
        self._check(DIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)


class TestSssp(unittest.TestCase):
    def _check(self, mtx, **kwargs):
        A = read_spla(str(mtx), FLOAT)
        n, pairs, weights = split(A)

        ref = short_distance(n, adj_weighted(n, pairs, weights), 0)

        d = sssp_spla(0, A, **kwargs)
        got = to_list(d, n, math.inf)
        got[0] = 0.0

        for i, (a, b) in enumerate(zip(ref, got)):
            if math.isinf(a) and math.isinf(b):
                continue
            self.assertLessEqual(abs(a - b), 1e-4, f"v{i}: ref={a} got={b}")

    def test_symmetric_push(self):
        self._check(UNDIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)

    def test_symmetric_pull(self):
        self._check(UNDIRECTED_GRAPH, push_only=False, pull_only=True, push_pull=False)

    def test_directed_push(self):
        self._check(DIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)


class TestPr(unittest.TestCase):
    def _check(self, mtx):
        A = read_mtx_pr_spla(str(mtx))
        n, pairs, weights = split(A)

        adj = [[] for _ in range(n)]
        wgt = [[] for _ in range(n)]
        for (u, v), w in zip(pairs, weights):
            adj[u].append(v)
            wgt[u].append(w)

        alpha, eps = 0.85, 1e-6
        ref = pr_classic(adj, wgt, alpha, eps)

        p = pr_spla(A, alpha, eps)
        got = to_list(p, n, 0.0)

        max_diff = max(abs(a - b) for a, b in zip(ref, got))
        self.assertLess(max_diff, 1e-4, f"max diff {max_diff:.2e}")

    def test_symmetric(self):
        self._check(UNDIRECTED_GRAPH)

    def test_directed(self):
        self._check(DIRECTED_GRAPH)


class TestTc(unittest.TestCase):
    def test_upper(self):
        A = read_spla(str(UPPER), INT)
        n, pairs, _ = split(A)

        adj = [[] for _ in range(n)]
        for u, v in pairs:
            adj[u].append(v)
            adj[v].append(u)

        ref = tc_classic(adj, n)
        got = tc_spla(A)

        self.assertEqual(ref, got)


if __name__ == "__main__":
    unittest.main(verbosity=2)