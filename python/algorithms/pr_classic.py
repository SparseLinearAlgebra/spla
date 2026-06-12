import math

def pagerank_classic(adj_in, out_degree, n, alpha, eps):
    p = [1.0 / n] * n
    addition = (1.0 - alpha) / n
    error = eps + 1.0
    iterations = 0

    while error > eps:
        p_next = [0.0] * n
        for i in range(n):
            sum_pr = 0.0
            for j in adj_in[i]:
                sum_pr += p[j] / out_degree[j]
            p_next[i] = alpha * sum_pr + addition

        error2 = 0.0
        for i in range(n):
            diff = p_next[i] - p[i]
            error2 += diff * diff

        error = math.sqrt(error2)
        p = p_next
        iterations += 1

    return p, iterations