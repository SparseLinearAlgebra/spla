"""Tests comparing the SPLA-based graph algorithms against classic references.

The SPLA implementations (python/algorithms) are checked against pure-Python
reference implementations on the small Matrix Market fixtures in ``data/``.
Triangle counting is additionally cross-checked against NetworkX when it is
installed.
"""

import heapq
import math

import networkx as nx
from bfs_classic import bfs_naive as bfs_classic
from bfs_spla import bfs_spla
from graph_spla import read_mtx_pr_spla, read_spla
from pr_classic import pagerank_naive as pr_classic
from pr_spla import pr_spla
from sssp_spla import sssp_spla
from tc_classic import tc_naive as tc_classic
from tc_spla import tc_sandia, tc_spla, tc_spla_mxmT

from pyspla import FLOAT, INT

UNDIRECTED_GRAPH = "undirected.mtx"
DIRECTED_GRAPH = "directed.mtx"
UPPER = "undirected_upper.mtx"


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


class TestBfs:
    def _check(self, data_dir, name, **kwargs):
        A = read_spla(str(data_dir / name), INT)
        n, pairs, _ = split(A)

        ref = bfs_classic(0, adj_plain(n, pairs), n)
        ref = [(x + 1) if x is not None else 0 for x in ref]

        v, _, _ = bfs_spla(0, A, **kwargs)
        got = to_list(v, n, 0)

        assert ref == got

    def test_symmetric_push(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)

    def test_symmetric_pull(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH, push_only=False, pull_only=True, push_pull=False)

    def test_symmetric_push_pull(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH, push_only=False, pull_only=False, push_pull=True)

    def test_directed_push(self, data_dir):
        self._check(data_dir, DIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)


class TestSssp:
    def _check(self, data_dir, name, **kwargs):
        A = read_spla(str(data_dir / name), FLOAT)
        n, pairs, weights = split(A)

        ref = short_distance(n, adj_weighted(n, pairs, weights), 0)

        d = sssp_spla(0, A, **kwargs)
        got = to_list(d, n, math.inf)
        got[0] = 0.0

        for i, (a, b) in enumerate(zip(ref, got)):
            if math.isinf(a) and math.isinf(b):
                continue
            assert abs(a - b) <= 1e-4, f"v{i}: ref={a} got={b}"

    def test_symmetric_push(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)

    def test_symmetric_pull(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH, push_only=False, pull_only=True, push_pull=False)

    def test_directed_push(self, data_dir):
        self._check(data_dir, DIRECTED_GRAPH, push_only=True, pull_only=False, push_pull=False)


class TestPr:
    def _check(self, data_dir, name):
        A = read_mtx_pr_spla(str(data_dir / name))
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
        assert max_diff < 1e-4, f"max diff {max_diff:.2e}"

    def test_symmetric(self, data_dir):
        self._check(data_dir, UNDIRECTED_GRAPH)

    def test_directed(self, data_dir):
        self._check(data_dir, DIRECTED_GRAPH)


class TestTc:
    def _check(self, data_dir, algo):
        # triangle counting is unweighted: read with unit weights
        A = read_spla(str(data_dir / UPPER), INT, unit_weights=True)
        n, pairs, _ = split(A)

        adj = [[] for _ in range(n)]
        for u, v in pairs:
            adj[u].append(v)
            adj[v].append(u)

        ref = tc_classic(adj, n)
        got = algo(A)

        assert ref == got

    def test_sandia(self, data_dir):
        self._check(data_dir, tc_sandia)

    def test_default(self, data_dir):
        self._check(data_dir, tc_spla)

    def test_mxmT(self, data_dir):
        self._check(data_dir, tc_spla_mxmT)

    def test_networkx(self, data_dir):
        A = read_spla(str(data_dir / UPPER), INT, unit_weights=True)
        n, pairs, _ = split(A)

        G = nx.Graph()
        G.add_nodes_from(range(n))
        G.add_edges_from(pairs)
        ref = sum(nx.triangles(G).values()) // 3

        assert tc_sandia(A) == ref
        assert tc_spla_mxmT(A) == ref
