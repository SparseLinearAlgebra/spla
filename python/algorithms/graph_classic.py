from collections import defaultdict

def read_mtx_unweighted(filename):
    n = 0
    graph = defaultdict(list)
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            graph[i].append(j)
            if i != j:
                graph[j].append(i)
    return graph, n

def read_vectors_unweighted(filename):
    with open(filename, 'r') as f:
        n_line = f.readline().strip()
        if not n_line:
            return defaultdict(list), 0
        n = int(n_line)
        I = list(map(int, f.readline().split()))
        J = list(map(int, f.readline().split()))
    graph = defaultdict(list)
    for k in range(len(I)):
        i = I[k]
        j = J[k]
        graph[i].append(j)
        if i != j:
            graph[j].append(i)
    return graph, n

def read_mtx_weighted(filename):
    n = 0
    graph = defaultdict(list)
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            v = float(parts[2]) if len(parts) > 2 else 1.0
            graph[i].append((j, v))
            if i != j:
                graph[j].append((i, v))
    return graph, n

def read_vectors_weighted(filename):
    with open(filename, 'r') as f:
        n_line = f.readline().strip()
        if not n_line:
            return defaultdict(list), 0
        n = int(n_line)
        I = list(map(int, f.readline().split()))
        J = list(map(int, f.readline().split()))
        V = list(map(float, f.readline().split()))
    graph = defaultdict(list)
    for k in range(len(I)):
        i = I[k]
        j = J[k]
        v = V[k]
        graph[i].append((j, v))
        if i != j:
            graph[j].append((i, v))
    return graph, n

def read_mtx_pr_classic(filename):
    n = 0
    adj_in = defaultdict(list)
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                out_degree = [0] * n
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            adj_in[j].append(i)
            out_degree[i] += 1
            if i != j:
                adj_in[i].append(j)
                out_degree[j] += 1
    return adj_in, out_degree, n

def read_vectors_pr_classic(filename):
    with open(filename, 'r') as f:
        n_line = f.readline().strip()
        if not n_line:
            return defaultdict(list), [], 0
        n = int(n_line)
        I = list(map(int, f.readline().split()))
        J = list(map(int, f.readline().split()))
    adj_in = defaultdict(list)
    out_degree = [0] * n
    for k in range(len(I)):
        i = I[k]
        j = J[k]
        adj_in[j].append(i)
        out_degree[i] += 1
        if i != j:
            adj_in[i].append(j)
            out_degree[j] += 1
    return adj_in, out_degree, n