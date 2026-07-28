INF = 1e9


def sssp(start, graph, n):
    dist = [INF] * n
    dist[start] = 0

    for _ in range(n - 1):
        for u in range(n):
            if dist[u] != INF:
                for v, w in graph[u]:
                    dist[v] = min(dist[v], dist[u] + w)

    return dist
