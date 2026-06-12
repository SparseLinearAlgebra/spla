def tc_simple(graph, n):
    triangles = 0

    for i in range(n):
        for j in graph[i]:
            for k in graph[j]:
                if k in graph[i]:
                    triangles += 1

    return triangles // 6

