from collections import deque

def bfs(start, graph, n):
    depth = [None] * n
    q = deque()
    depth[start] = 0
    q.append(start)

    while q:
        u = q.popleft()
        for v in graph[u]:
            if depth[v] is None:
                depth[v] = depth[u] + 1
                q.append(v)

    return depth