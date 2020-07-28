
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

"""
Functions for secure comparison of GF(p) types.
Most protocols come from [1], with a few subroutines described in [2].

Function naming of comparison routines is as in [1,2], with k always
representing the integer bit length, and kappa the statistical security
parameter.

Most of these routines were implemented before the cint/sint classes, so use
the old-fasioned Register class and assembly instructions instead of operator
overloading.

[1] https://www1.cs.fau.de/filepool/publications/octavian_securescm/smcint-scn10.pdf
[2] https://www1.cs.fau.de/filepool/publications/octavian_securescm/SecureSCM-D.9.2.pdf
"""

# Use constant rounds protocols instead of log rounds
const_rounds = False

import instructions_base

def set_variant(options):
    """ Set flags based on the command-line option provided """
    global const_rounds
    variant = options.comparison
    if variant == 'log':
        const_rounds = False
    elif variant == 'plain':
        const_rounds = True
    elif variant is not None:
        raise CompilerError('Unknown comparison variant: %s' % variant)

def ld2i(c, n):
    """ Load immediate 2^n into clear GF(p) register c """
    t1 = program.curr_block.new_reg('c')
    ldi(t1, 2 ** (n % 30))
    for i in range(n / 30):
        t2 = program.curr_block.new_reg('c')
        mulci(t2, t1, 2 ** 30)
        t1 = t2
    movc(c, t1)

inverse_of_two = {}

def divide_by_two(res, x):
    """ Faster clear division by two using a cached value of 2^-1 mod p """
    from program import Program
    import types
    block = Program.prog.curr_block
    if len(inverse_of_two) == 0 or block not in inverse_of_two:
        inverse_of_two[block] = types.cint(1) / 2
    mulc(res, x, inverse_of_two[block])

def LTZ(s, a, k, kappa):
    """
    s = (a ?< 0)

    k: bit length of a
    """
    t = program.curr_block.new_reg('s')
    Trunc(t, a, k, k - 1, kappa, True)
    subsfi(s, t, 0)

def Trunc(d, a, k, m, kappa, signed):
    """
    d = a >> m

    k: bit length of a
    m: compile-time integer
    signed: True/False, describes a
    """
    a_prime = program.curr_block.new_reg('s')
    t = program.curr_block.new_reg('s')
    c = [program.curr_block.new_reg('c') for i in range(3)]
    c2m = program.curr_block.new_reg('c')
    if m == 0:
        movs(d, a)
        return
    elif m == 1:
        Mod2(a_prime, a, k, kappa, signed)
    else:
        Mod2m(a_prime, a, k, m, kappa, signed)
    subs(t, a, a_prime)
    ldi(c[1], 1)
    ld2i(c2m, m)
    divc(c[2], c[1], c2m)
    mulm(d, t, c[2])

def TruncRoundNearest(a, k, m, kappa):
    """
    Returns a / 2^m, rounded to the nearest integer.

    k: bit length of m
    m: compile-time integer
    """
    from types import sint, cint
    from library import reveal, load_int_to_secret
    if m == 1:
        lsb = sint()
        Mod2(lsb, a, k, kappa, False)
        return (a + lsb) / 2
    r_dprime = sint()
    r_prime = sint()
    r = [sint() for i in range(m)]
    u = sint()
    PRandM(r_dprime, r_prime, r, k, m, kappa)
    c = reveal((cint(1) << (k - 1)) + a + (cint(1) << m) * r_dprime + r_prime)
    c_prime = c % (cint(1) << (m - 1))
    if const_rounds:
        BitLTC1(u, c_prime, r[:-1], kappa)
    else:
        BitLTL(u, c_prime, r[:-1], kappa)
    bit = ((c - c_prime) / (cint(1) << (m - 1))) % 2
    xor = bit + u - 2 * bit * u
    prod = xor * r[-1]
    # u_prime = xor * u + (1 - xor) * r[-1]
    u_prime = bit * u + u - 2 * bit * u + r[-1] - prod
    a_prime = (c % (cint(1) << m)) - r_prime + (cint(1) << m) * u_prime
    d = (a - a_prime) / (cint(1) << m)
    rounding = xor + r[-1] - 2 * prod
    return d + rounding

def Mod2m(a_prime, a, k, m, kappa, signed):
    """
    a_prime = a % 2^m

    k: bit length of a
    m: compile-time integer
    signed: True/False, describes a
    """
    if m >= k:
        movs(a_prime, a)
        return
    r_dprime = program.curr_block.new_reg('s')
    r_prime = program.curr_block.new_reg('s')
    r = [program.curr_block.new_reg('s') for i in range(m)]
    c = program.curr_block.new_reg('c')
    c_prime = program.curr_block.new_reg('c')
    v = program.curr_block.new_reg('s')
    u = program.curr_block.new_reg('s')
    t = [program.curr_block.new_reg('s') for i in range(6)]
    c2m = program.curr_block.new_reg('c')
    c2k1 = program.curr_block.new_reg('c')
    PRandM(r_dprime, r_prime, r, k, m, kappa)
    ld2i(c2m, m)
    mulm(t[0], r_dprime, c2m)
    if signed:
        ld2i(c2k1, k - 1)
        addm(t[1], a, c2k1)
    else:
        t[1] = a
    adds(t[2], t[0], t[1])
    adds(t[3], t[2], r_prime)
    startopen(t[3])
    stopopen(c)
    modc(c_prime, c, c2m)
    if const_rounds:
        BitLTC1(u, c_prime, r, kappa)
    else:
        BitLTL(u, c_prime, r, kappa)
    mulm(t[4], u, c2m)
    submr(t[5], c_prime, r_prime)
    adds(a_prime, t[5], t[4])
    return r_dprime, r_prime, c, c_prime, u, t, c2k1

def PRandM(r_dprime, r_prime, b, k, m, kappa):
    """
    r_dprime = random secret integer in range [0, 2^(k + kappa - m) - 1]
    r_prime = random secret integer in range [0, 2^m - 1]
    b = array containing bits of r_prime
    """
    t = [[program.curr_block.new_reg('s') for j in range(2)] for i in range(m)]
    t[0][1] = b[-1]
    PRandInt(r_dprime, k + kappa - m)
    # r_dprime is always multiplied by 2^m
    program.curr_tape.require_bit_length(k + kappa)
    bit(b[-1])
    for i in range(1,m):
        adds(t[i][0], t[i-1][1], t[i-1][1])
        bit(b[-i-1])
        adds(t[i][1], t[i][0], b[-i-1])
    movs(r_prime, t[m-1][1])


def PRandInt(r, k):
    """
    r = random secret integer in range [0, 2^k - 1]
    """
    t = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(3)]
    t[2][k-1] = r
    bit(t[2][0])
    for i in range(1,k):
        adds(t[0][i], t[2][i-1], t[2][i-1])
        bit(t[1][i])
        adds(t[2][i], t[0][i], t[1][i])

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

def carry(b, a, compute_p):
    """ Carry propogation:
        return (p,g) = (p_2, g_2)o(p_1, g_1) -> (p_1 & p_2, g_2 | (p_2 & g_1))
    """
    if a is None:
        return b
    if b is None:
        return a
    t = [program.curr_block.new_reg('s') for i in range(3)]
    if compute_p:
        muls(t[0], a[0], b[0])        
    muls(t[1], a[0], b[1])
    adds(t[2], a[1], t[1])
    return t[0], t[2]

# from WP9 report
# length of a is even
def CarryOutAux(d, a, kappa):
    k = len(a)
    if k > 1 and k % 2 == 1:
        a.append(None)
        k += 1
    u = [None]*(k/2)
    a = a[::-1]
    if k > 1:
        for i in range(k/2):
            u[i] = carry(a[2*i+1], a[2*i], i != k/2-1)
        CarryOutAux(d, u[:k/2][::-1], kappa)
    else:
        movs(d, a[0][1])

# carry out with carry-in bit c
def CarryOut(res, a, b, c, kappa):
    """
    res = last carry bit in addition of a and b

    a: array of clear bits
    b: array of secret bits (same length as a)
    c: initial carry-in bit
    """
    k = len(a)
    d = [program.curr_block.new_reg('s') for i in range(k)]
    t = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(4)]
    s = [program.curr_block.new_reg('s') for i in range(3)]
    for i in range(k):
        mulm(t[0][i], b[i], a[i])
        mulsi(t[1][i], t[0][i], 2)
        addm(t[2][i], b[i], a[i])
        subs(t[3][i], t[2][i], t[1][i])
        d[i] = [t[3][i], t[0][i]]
    mulsi(s[0], d[-1][0], c)
    adds(s[1], d[-1][1], s[0])
    d[-1][1] = s[1]
    
    CarryOutAux(res, d[::-1], kappa)

def BitLTL(res, a, b, kappa):
    """
    res = a <? b (logarithmic rounds version)

    a: clear integer register
    b: array of secret bits (same length as a)
    """
    k = len(b)
    a_bits = [program.curr_block.new_reg('c') for i in range(k)]
    c = [[program.curr_block.new_reg('c') for i in range(k)] for j in range(2)]
    s = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(2)]
    t = [program.curr_block.new_reg('s') for i in range(1)]
    modci(a_bits[0], a, 2)
    c[1][0] = a
    for i in range(1,k):
        subc(c[0][i], c[1][i-1], a_bits[i-1])
        divide_by_two(c[1][i], c[0][i])
        modci(a_bits[i], c[1][i], 2)
    for i in range(len(b)):
        subsfi(s[0][i], b[i], 1)
    CarryOut(t[0], a_bits[::-1], s[0][::-1], 1, kappa)
    subsfi(res, t[0], 1)
    return a_bits, s[0]


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

def KMulC(a):
    """
    Return just the product of all items in a
    """
    from types import sint, cint
    p = sint()
    PreMulC_without_inverses(p, a)
    return p

def Mod2(a_0, a, k, kappa, signed):
    """
    a_0 = a % 2

    k: bit length of a
    """
    if k <= 1:
        movs(a_0, a)
        return
    r_dprime = program.curr_block.new_reg('s')
    r_prime = program.curr_block.new_reg('s')
    r_0 = program.curr_block.new_reg('s')
    c = program.curr_block.new_reg('c')
    c_0 = program.curr_block.new_reg('c')
    tc = program.curr_block.new_reg('c')
    t = [program.curr_block.new_reg('s') for i in range(6)]
    c2k1 = program.curr_block.new_reg('c')
    PRandM(r_dprime, r_prime, [r_0], k, 1, kappa)
    mulsi(t[0], r_dprime, 2)
    if signed:
        ld2i(c2k1, k - 1)
        addm(t[1], a, c2k1)
    else:
        t[1] = a
    adds(t[2], t[0], t[1])
    adds(t[3], t[2], r_prime)
    startopen(t[3])
    stopopen(c)
    modci(c_0, c, 2)
    mulci(tc, c_0, 2)
    mulm(t[4], r_0, tc)
    addm(t[5], r_0, c_0)
    subs(a_0, t[5], t[4])


# hack for circular dependency
from instructions import *
