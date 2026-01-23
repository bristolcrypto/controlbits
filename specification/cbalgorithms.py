""" This functions given in this file are designed to match the algorithms
    and definitions given in the paper. """

import random

def getbit(k, x):
    """ Gets bit k from x. k=0 gives LSB."""
    return (x >> k) & 1

def insertbit(k, b, x):
    """ Inserts bit b into integer x at bit position k."""
    mask = (1 << k) - 1
    lowerbits = x & mask
    upperbits = x - lowerbits
    return (upperbits << 1) + (b << k) + lowerbits

def removebit(k, x):
    """ Removes bit at position k from x."""
    mask = (1 << k) - 1
    lowerbits = x & mask
    upperbits = x - lowerbits
    return (upperbits >> 1) + lowerbits

def oSort(A, Vecpi): 
    """Returns A(pi^{-1}). Same as Bernstein's composeinv. 
       `Stand in' for the decorate-sort-undecorate idiom."""
    return [a for _, a in sorted(zip(Vecpi, A))]

def oLayer(A, C, k):
    """Algorithm 1."""
    for i, c in enumerate(C):
        A[insertbit(k, 0, i)], A[insertbit(k, 1, i)] = A[insertbit(k, 0^c, i)], A[insertbit(k, 1^c, i)]  
    return A

def cbapply(C):
    """Algorithm 2."""
    m = 1
    while (2*m+1) << (m) < len(C):
        m += 1
    assert (2*m+1) << (m) == len(C)

    n = 1 << (m+1)
    pi = list(range(n))
    for j in range(2*m+1):
        k = min(j, 2*m-j)
        oLayer(pi, C[j*n//2:(j+1)*n//2], k)
    
    return pi

def fastCycleMin(i : int, pi : "list[int]"):
    """Algorithm 5"""
    c = range(len(pi))
    p = [pi[x ^ (1 << i)] for x in c]
    q = [pix ^ (1 << i) for pix in pi]
    for _ in range((len(pi) >> 2).bit_length() - i):
        p, q = oSort(p, q), oSort(q, p)
        cp = oSort(c, q)
        c = [min(ci, cip) for (ci, cip) in zip(c, cp)]
    return c

def cbrecursion(P: list[int], C: list[int], m: int, pos: int, step: int):
    """Algorithm 3."""
    n = 1 << (m+1)
    assert len(P) == n

    if m == 0:
        C[pos] = P[0]
        return
    
    Cmin = fastCycleMin(0, P)
    F = [getbit(0, Cmin[insertbit(0, 0, j)])   for j in range(n//2)]
    layer_F = oLayer(list(range(n)), F, 0)
    P = oSort(layer_F, oSort(range(n), P))
    L = [getbit(0, P[insertbit(0, 0, j)]) for j in range(n//2)]
    P = oLayer(P, L, 0) 

    P0 = [P[2*j]//2   for j in range(n//2)]
    P1 = [P[2*j+1]//2 for j in range(n//2)]

    C[pos                 : pos+(n//2)*step       : step] = F
    C[pos+(2*m)*step*n//2 : pos+(2*m+1)*step*n//2 : step] = L
    
    cbrecursion(P0, C, m-1, pos+(n//2)*step,     2*step)
    cbrecursion(P1, C, m-1, pos+(1+(n//2))*step, 2*step)
    

def cbiterative(P, m):
    """Algorithm 4."""
    n = 1 << (m+1)
    C = [0]*( (2*m+1)*n//2 )

    for k in range(m):
        Cmin = fastCycleMin(k, P)
        F = [getbit(k, Cmin[insertbit(k, 0, j)]) for j in range(n//2)]
        layer_F = oLayer(list(range(n)), F, k)
        P = oSort(layer_F, oSort(range(n), P))
        L = [getbit(k, P[insertbit(k, 0, j)])    for j in range(n//2)]
        P = oLayer(P, L, k)
        C[(k)*n//2     : (k+1)*n//2     : 1] = F
        C[(2*m-k)*n//2 : (2*m-k+1)*n//2 : 1] = L
    
    C[m*n//2 : (m+1)*n//2 : 1] = [getbit(m, P[j]) for j in range(n//2)]
    return C



if __name__ == '__main__':
    # We perform control bit calculation on arrays of size n = 2^{m+1}.
    m = 5
    P = list(range(1 << (m+1)))

    for i in range(1):
        random.shuffle(P)

        # cbrecursion is correct
        C = [0]*( (2*m+1)*(1 << (m)))
        cbrecursion(P, C, m, 0, 1)    
        assert cbapply(C) == P
        
        # cbiterative is correct
        assert cbapply(cbiterative(P, m)) == P
        
        # cbiterative and cbrecursion output equal control bits
        assert C == cbiterative(P, m)