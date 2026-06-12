from collections import deque

INF = int(1e9)

def bfs(start, graph, n):
    visited = [0] * n
    dist = [INF] * n
    q = deque()
    visited[start] = 1
    dist[start] = 0
    q.append(start)
    
    while q:
        u = q.popleft()
        for v in graph[u]:
            if not visited[v]:
                visited[v] = 1
                dist[v] = dist[u] + 1
                q.append(v)
    return dist