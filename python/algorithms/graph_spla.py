from pyspla import *

def read_mtx_int(filename):
    I, J, V = [], [], []
    n_vertices = 0
    
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n_vertices == 0:
                n_vertices = int(parts[0])
                continue
                
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            
            I.append(i)
            J.append(j)
            V.append(1)
            
            if i != j:
                I.append(j)
                J.append(i)
                V.append(1)
                
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=INT)

def read_vectors_int(filename):
    with open(filename, 'r') as f:
        raw_I = list(map(int, f.readline().replace(',', ' ').split()))
        raw_J = list(map(int, f.readline().replace(',', ' ').split()))
        
    n_vertices = max(max(raw_I), max(raw_J)) + 1
    I, J, V = [], [], []
    
    for k in range(len(raw_I)):
        i = raw_I[k]
        j = raw_J[k]
        
        I.append(i)
        J.append(j)
        V.append(1)
        
        if i != j:
            I.append(j)
            J.append(i)
            V.append(1)
            
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=INT)

def read_mtx_float(filename):
    I, J, V = [], [], []
    n_vertices = 0
    
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n_vertices == 0:
                n_vertices = int(parts[0])
                continue
                
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            weight = float(parts[2]) if len(parts) > 2 else 1.0
            
            I.append(i)
            J.append(j)
            V.append(weight)
            
            if i != j:
                I.append(j)
                J.append(i)
                V.append(weight)
                
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=FLOAT)

def read_vectors_float(filename):
    with open(filename, 'r') as f:
        raw_I = list(map(int, f.readline().replace(',', ' ').split()))
        raw_J = list(map(int, f.readline().replace(',', ' ').split()))
        raw_V = list(map(float, f.readline().replace(',', ' ').split()))
        
    n_vertices = max(max(raw_I), max(raw_J)) + 1
    I, J, V = [], [], []
    
    for k in range(len(raw_I)):
        i = raw_I[k]
        j = raw_J[k]
        weight = raw_V[k]
        
        I.append(i)
        J.append(j)
        V.append(weight)
        
        if i != j:
            I.append(j)
            J.append(i)
            V.append(weight)
            
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=FLOAT)

def read_mtx_pr(filename, alpha):
    I, J, V = [], [], []
    n_vertices = 0
    
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n_vertices == 0:
                n_vertices = int(parts[0])
                out_degree = [0] * n_vertices
                continue
                
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            
            I.append(i)
            J.append(j)
            out_degree[i] += 1
            
            if i != j:
                I.append(j)
                J.append(i)
                out_degree[j] += 1
                
    for k in range(len(I)):
        u = I[k]
        V.append(alpha / out_degree[u])
        
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=FLOAT)

def read_vectors_pr(filename, alpha):
    with open(filename, 'r') as f:
        raw_I = list(map(int, f.readline().replace(',', ' ').split()))
        raw_J = list(map(int, f.readline().replace(',', ' ').split()))
        
    n_vertices = max(max(raw_I), max(raw_J)) + 1
    out_degree = [0] * n_vertices
    I, J, V = [], [], []
    
    for k in range(len(raw_I)):
        i = raw_I[k]
        j = raw_J[k]
        
        I.append(i)
        J.append(j)
        out_degree[i] += 1
        
        if i != j:
            I.append(j)
            J.append(i)
            out_degree[j] += 1
            
    for k in range(len(I)):
        u = I[k]
        V.append(alpha / out_degree[u])
            
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=FLOAT)