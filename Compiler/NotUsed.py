
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.


def KORL(a, kappa):
    """ log rounds k-ary OR """
    k = len(a)
    if k == 1:
        return a[0]
    else:
        t1 = KORL(a[:k/2], kappa)
        t2 = KORL(a[k/2:], kappa)
        return t1 + t2 - t1*t2

def KORC(a, kappa):
    return PreORC(a, kappa, 1)[0]

def KOR(a, kappa):
    if comparison.const_rounds:
        return KORC(a, kappa)
    else:
        return KORL(a, None)

#Constant round KMul
def KMulC(a):
    """
    Return just the product of all items in a
    """
    from types import sint, cint
    p = sint()
    PreMulC_without_inverses(p, a)
    return p

def KMul(a):
    if comparison.const_rounds:
        return KMulC(a)
    else:
        return AdvInteger.KOpL(AdvInteger.mul_op, a)

# Constant round PreOR
def PreORC(a, kappa=None, m=None, raw=False):
    k = len(a)
    if k == 1:
        return [a[0]]
    m = m or k
    max_k = int(log(program.Program.prog.P) / log(2)) - kappa
    if k <= max_k:
        p = [None] * m
        if m == k:
            p[0] = a[0]
        t = [types.sint() for i in range(m)]
        b = comparison.PreMulC([a[i] + 1 for i in range(k)])
        for i in range(m):
           AdvInteger.Mod2(t[i], b[k-1-i], k, kappa, False)
           p[m-1-i] = 1 - t[i]
        return p
    else:
        # not constant-round anymore
        s = [PreORC(a[i:i+max_k], kappa, raw=raw) for i in range(0,k,max_k)]
        t = PreORC([si[-1] for si in s[:-1]], kappa, raw=raw)
        return sum(([AdvInteger.or_op(x, y) for x in si] for si,y in zip(s[1:],t)), s[0])


def PreOR(a, kappa=None, raw=False):
    if comparison.const_rounds:
        return PreORC(a, kappa, raw=raw)
    else:
        return AdvInteger.PreOpL(AdvInteger.or_op, a)

# Constant round BitLT
def BitLTC1(u, a, b, kappa):
    """
    u = a <? b

    a: array of clear bits
    b: array of secret bits (same length as a)
    """
    k = len(b)
    p = [program.curr_block.new_reg('s') for i in range(k)]
    if instructions_base.get_global_vector_size() == 1:
        b_vec = program.curr_block.new_reg('s', size=k)
        for i in range(k):
            movs(b_vec[i], b[i])
        a_bits = program.curr_block.new_reg('c', size=k)
        d = program.curr_block.new_reg('s', size=k)
        s = program.curr_block.new_reg('s', size=k)
        t = [program.curr_block.new_reg('s', size=k) for j in range(5)]
        c = [program.curr_block.new_reg('c', size=k) for j in range(4)]
    else:
        a_bits = [program.curr_block.new_reg('c') for i in range(k)]
        d = [program.curr_block.new_reg('s') for i in range(k)]
        s = [program.curr_block.new_reg('s') for i in range(k)]
        t = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(5)]
        c = [[program.curr_block.new_reg('c') for i in range(k)] for j in range(4)]
    c[1] = [program.curr_block.new_reg('c') for i in range(k)]
    # warning: computer scientists count from 0
    modci(a_bits[0], a, 2)
    c[1][0] = a
        for i in range(1,k):
        subc(c[0][i], c[1][i-1], a_bits[i-1])
        divide_by_two(c[1][i], c[0][i])
        modci(a_bits[i], c[1][i], 2)
    if instructions_base.get_global_vector_size() == 1:
        vmulci(k, c[2], a_bits, 2)
        vmulm(k, t[0], b_vec, c[2])
        vaddm(k, t[1], b_vec, a_bits)
        vsubs(k, d, t[1], t[0])
        vaddsi(k, t[2], d, 1)
        t[2].create_vector_elements()
        pre_input = t[2].vector[:]
    else:
        for i in range(k):
            mulci(c[2][i], a_bits[i], 2)
            mulm(t[0][i], b[i], c[2][i])
            addm(t[1][i], b[i], a_bits[i])
            subs(d[i], t[1][i], t[0][i])
            addsi(t[2][i], d[i], 1)
            pre_input = t[2][:]
    pre_input.reverse()
    PreMulC_without_inverses(p, pre_input)
    p.reverse()
    for i in range(k-1):
        subs(s[i], p[i], p[i+1])
    subsi(s[k-1], p[k-1], 1)
    subcfi(c[3][0], a_bits[0], 1)
    mulm(t[4][0], s[0], c[3][0])
    for i in range(1,k):
        subcfi(c[3][i], a_bits[i], 1)
        mulm(t[3][i], s[i], c[3][i])
        adds(t[4][i], t[4][i-1], t[3][i])
    Mod2(u, t[4][k-1], k, kappa, False)
    return p, a_bits, d, s, t, c, b, pre_input

def PreMulC_without_inverses(p, a):
    """
    Plain variant with no extra preprocessing.
    """
    k = len(a)
    r = [program.curr_block.new_reg('s') for i in range(k)]
    s = [program.curr_block.new_reg('s') for i in range(k)]
    u = [program.curr_block.new_reg('c') for i in range(k)]
    v = [program.curr_block.new_reg('s') for i in range(k)]
    w = [program.curr_block.new_reg('s') for i in range(k)]
    z = [program.curr_block.new_reg('s') for i in range(k)]
    m = [program.curr_block.new_reg('c') for i in range(k)]
    t = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(2)]
    #tt = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(4)]
    u_inv = [program.curr_block.new_reg('c') for i in range(k)]
    c = [program.curr_block.new_reg('c') for i in range(k)]
    # warning: computer scientists count from 0
    for i in range(k):
        triple(s[i], r[i], t[0][i])
        #adds(tt[0][i], t[0][i], a[i])
        #subs(tt[1][i], tt[0][i], a[i])
        #startopen(tt[1][i])
        startopen(t[0][i])
        stopopen(u[i])
    for i in range(k-1):
        muls(v[i], r[i+1], s[i])
    w[0] = r[0]
    one = program.curr_block.new_reg('c')
    ldi(one, 1)
    for i in range(k):
        divc(u_inv[i], one, u[i])
        # avoid division by zero, just for benchmarking
        #divc(u_inv[i], u[i], one)
    for i in range(1,k):
        mulm(w[i], v[i-1], u_inv[i-1])
    for i in range(1,k):
        mulm(z[i], s[i], u_inv[i])
    for i in range(k):
        muls(t[1][i], w[i], a[i])
        startopen(t[1][i])
        stopopen(m[i])
    PreMulC_end(p, a, c, m, z)

def PreMulC_end(p, a, c, m, z):
    """
    Helper function for all PreMulC variants. Local operation.
    """
    k = len(a)
    c[0] = m[0]
    for j in range(1,k):
        mulc(c[j], c[j-1], m[j])
        if isinstance(p, list):
            mulm(p[j], z[j], c[j])
    if isinstance(p, list):
        p[0] = a[0]
    else:
        mulm(p, z[-1], c[-1])

def PreMulC(a):
    p = [type(a[0])() for i in range(len(a))]
    instructions_base.set_global_instruction_type(a[0].instruction_type)
    PreMulC_without_inverses(p, a)
    instructions_base.reset_global_instruction_type()
    return p


