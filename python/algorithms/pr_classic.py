import math

def pagerank_naive(Ai, Ax, alpha,eps):
    N=len(Ai)
    p_prev=[1.0/N]*N
    error=eps+1.0
    while error>eps:
        p=[0.0]*N
        for i in range(N):
            s=0.0
            for k in range(len(Ai[i])):
                s+=Ax[i][k]*p_prev[Ai[i][k]]
            p[i]=s+(1.0-alpha)/N
        error=0.0
        for i in range(N):
            diff=p[i]-p_prev[i]
            error+=diff*diff
        error=math.sqrt(error)
        p_prev,p=p,p_prev

    return p_prev