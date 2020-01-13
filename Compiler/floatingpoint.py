from __future__ import print_function
from __future__ import division
##
# @file
# floating point operations
# File documentation
#
from builtins import zip
from builtins import range
from past.utils import old_div
from math import log, floor, ceil
from Compiler.instructions import *
import types
import comparison
import program


##
## Helper functions for floating point arithmetic
##


def two_power(n):
    if isinstance(n, int) and n < 31:
        return 2**n
    else:
        max = types.cint(1) << 31
        res = 2**(n%31)
        for i in range(old_div(n, 31)):
            res *= max
        return res

def shift_two(n, pos):
    if pos < 63:
        return n >> pos
    else:
        res = (n >> (pos%63))
        for i in range(old_div(pos, 63)):
            res >>= 63
        return res

##
# Simplified less than test for sfloat.
# Returns 0 if there is error.
# @param fl_a: always an sfloat
# @param fl_b: can be sfloat or cfloat
# @return sint \{0,1\} where a < 0.
def FLLT (fl_a, fl_b):
    t = fl_a.err
    if isinstance(fl_b, types.sfloat):
        t = t + fl_b.err
    t = t == 0
    z1 = fl_a.z
    z2 = fl_b.z
    s1 = fl_a.s
    s2 = fl_b.s
    a = fl_a.p.less_than(fl_b.p, fl_a.plen, fl_a.kappa)
    c = EQZ(fl_a.p - fl_b.p, fl_a.plen, fl_a.kappa)
    d = ((1 - 2 * fl_a.s) * fl_a.v).less_than((1 - 2 * fl_b.s) * fl_b.v, fl_a.vlen + 1, fl_a.kappa)
    cd = c * d
    ca = c * a
    b1 = cd + a - ca
    b2 = cd + 1 + ca - c - a
    s12 = fl_a.s * fl_b.s
    z12 = fl_a.z * fl_b.z
    b = (z1 - z12) * (1 - s2) + (z2 - z12) * s1 + (1 + z12 - z1 - z2) * \
        (s1 - s12 + (1 + s12 - s1 - s2) * b1 + s12 * b2) * t
    return b


def EQZ(a, k, kappa):
    r_dprime = types.sint()
    r_prime = types.sint()
    c = types.cint()
    d = [None]*k
    r = [types.sint() for i in range(k)]
    comparison.PRandM(r_dprime, r_prime, r, k, k, kappa)
    startopen(a + two_power(k) * r_dprime + r_prime)# + 2**(k-1))
    stopopen(c)
    for i,b in enumerate(bits(c, k)):
        d[i] = b + r[i] - 2*b*r[i]
    return 1 - KOR(d, kappa)


##
# Simplified less than  zero test for sfloat.
# Returns 0 if there is error.
# @param a: input to be zero tested
# @return sint \{0,1\} where a < 0.
def FLLTZ(a):
    return (a.s * (1 - a.z)) * (a.err == 0)


##
# Simplified zero test for sfloat.
# Returns 0 if there is error.
# @param a: input to be zero tested
# @return sint \{0,1\} where a == 0.
def FLEQZ(a):
    return a.z * (a.err == 0)


##
# Simplified greater than zero test for sfloat.
# Returns 0 if there is error.
# @param a: input to be zero tested
# @return sint \{0,1\} where a > 0.
def FLGTZ(a):
    return ((1 - a.s) * (1 - a.z)) * (a.err == 0)


##
# Simplified less equal than zero test for sfloat.
# Returns 0 if there is error.
# @param a: input to be zero tested
# @return sint \{0,1\} where a <= 0.
def FLLEZ(a):
    return a.s * (a.err == 0)


##
# Simplified greater equal than zero test for sfloat.
# Returns 0 if there is error.
# @param a: input to be zero tested
# @return sint \{0,1\} where a >= 0.
def FLGEZ(a):
    return (1 - a.s) * (a.err == 0)


def bits(a,m):
    """ Get the bits of an int """
    if isinstance(a, int):
        res = [None]*m
        for i in range(m):
            res[i] = a & 1
            a >>= 1
    else:
        c = [[types.cint() for i in range(m)] for i in range(2)]
        res = [types.cint() for i in range(m)]
        modci(res[0], a, 2)
        c[1][0] = a
        for i in range(1,m):
            subc(c[0][i], c[1][i-1], res[i-1])
            divci(c[1][i], c[0][i], 2)
            modci(res[i], c[1][i], 2)
    return res

def carry(b, a, compute_p=True):
    """ Carry propogation:
        (p,g) = (p_2, g_2)o(p_1, g_1) -> (p_1 & p_2, g_2 | (p_2 & g_1))
    """
    if compute_p:
        t1 = a[0] * b[0]
    else:
        t1 = None
    t2 = a[1] + a[0] * b[1]
    return (t1, t2)

def or_op(a, b, void=None):
    return a + b - a * b

def mul_op(a, b, void=None):
    return a * b

def xor_op(a, b, void=None):
    return a + b - 2* a * b


def PreORC(a, kappa=None, m=None, raw=False):
    k = len(a)
    if k == 1:
        return [a[0]]
    m = m or k
    max_k = int(old_div(log(program.Program.prog.P), log(2))) - kappa
    if k <= max_k:
        p = [None] * m
        if m == k:
            p[0] = a[0]
        t = [types.sint() for i in range(m)]
        b = comparison.PreMulC([a[i] + 1 for i in range(k)])
        for i in range(m):
           comparison.Mod2(t[i], b[k-1-i], k, kappa, False)
           p[m-1-i] = 1 - t[i]
        return p
    else:
        # not constant-round anymore
        s = [PreORC(a[i:i+max_k], kappa, raw=raw) for i in range(0,k,max_k)]
        t = PreORC([si[-1] for si in s[:-1]], kappa, raw=raw)
        return sum(([or_op(x, y) for x in si] for si,y in zip(s[1:],t)), s[0])

def PreOpL(op, items):
    """
    Uses algorithm from SecureSCM WP9 deliverable.
    
    op must be a binary function that outputs a new register
    """
    k = len(items)
    logk = int(ceil(log(k,2)))
    kmax = 2**logk
    output = list(items)
    for i in range(logk):
        for j in range(old_div(kmax,(2**(i+1)))):
            y = two_power(i) + j*two_power(i+1) - 1
            for z in range(1, 2**i+1):
                if y+z < k:
                    output[y+z] = op(output[y], output[y+z], j != 0)
    return output

def PreOpN(op, items):
    """ Naive PreOp algorithm """
    k = len(items)
    output = [None]*k
    output[0] = items[0]
    for i in range(1, k):
        output[i] = op(output[i-1], items[i])
    return output

def PreOR(a, kappa=None, raw=False):
    if comparison.const_rounds:
        return PreORC(a, kappa, raw=raw)
    else:
        return PreOpL(or_op, a)

def KOpL(op, a):
    k = len(a)
    if k == 1:
        return a[0]
    else:
        t1 = KOpL(op, a[:old_div(k,2)])
        t2 = KOpL(op, a[old_div(k,2):])
        return op(t1, t2)

def KORL(a, kappa):
    """ log rounds k-ary OR """
    k = len(a)
    if k == 1:
        return a[0]
    else:
        t1 = KORL(a[:old_div(k,2)], kappa)
        t2 = KORL(a[old_div(k,2):], kappa)
        return t1 + t2 - t1*t2

def KORC(a, kappa):
    return PreORC(a, kappa, 1)[0]

def KOR(a, kappa):
    if comparison.const_rounds:
        return KORC(a, kappa)
    else:
        return KORL(a, None)

def KMul(a):
    if comparison.const_rounds:
        return comparison.KMulC(a)
    else:
        return KOpL(mul_op, a)

def FlAbs(a):
    return types.sfloat(v = a.v, p = a.p, z = a.z, s = types.sint(0), err = a.err)

def Inv(a):
    """ Invert a non-zero value """
    t = [types.sint() for i in range(2)]
    c = [types.cint() for i in range(2)]
    one = types.cint()
    ldi(one, 1)
    square(t[0],t[1]);
    s = t[0]*a
    asm_open(c[0], s)
    # avoid division by zero for benchmarking
    divc(c[1], one, c[0])
    #divc(c[1], c[0], one)
    return c[1]*t[0]

def BitAdd(a, b, bits_to_compute=None):
    """ Add the bits a[k-1], ..., a[0] and b[k-1], ..., b[0], return k+1
        bits s[0], ... , s[k] """
    k = len(a)
    if not bits_to_compute:
        bits_to_compute = list(range(k))
    d = [None] * k
    for i in range(1,k):
        #assert(a[i].value == 0 or a[i].value == 1)
        #assert(b[i].value == 0 or b[i].value == 1)
        t = a[i]*b[i]
        d[i] = (a[i] + b[i] - 2*t, t)
        #assert(d[i][0].value == 0 or d[i][0].value == 1)
    d[0] = (None, a[0]*b[0])
    pg = PreOpL(carry, d)
    c = [pair[1] for pair in pg]
    
    # (for testing)
    def print_state():
        print('a: ', end=' ')
        for i in range(k):
            print('%d ' % a[i].value, end=' ')
        print('\nb: ', end=' ')
        for i in range(k):
            print('%d ' % b[i].value, end=' ')
        print('\nd: ', end=' ')
        for i in range(k):
            print('%d ' % d[i][0].value, end=' ')
        print('\n   ', end=' ')
        for i in range(k):
            print('%d ' % d[i][1].value, end=' ')
        print('\n\npg:', end=' ')
        for i in range(k):
            print('%d ' % pg[i][0].value, end=' ')
        print('\n    ', end=' ')
        for i in range(k):
            print('%d ' % pg[i][1].value, end=' ')
        print('')
    
    for bit in c:
        pass#assert(bit.value == 0 or bit.value == 1)
    s = [None] * (k+1)
    if 0 in bits_to_compute:
        s[0] = a[0] + b[0] - 2*c[0]
        bits_to_compute.remove(0)
    #assert(c[0].value == a[0].value*b[0].value)
    #assert(s[0].value == 0 or s[0].value == 1)
    for i in bits_to_compute:
        s[i] = a[i] + b[i] + c[i-1] - 2*c[i]
        try:
            pass#assert(s[i].value == 0 or s[i].value == 1)
        except AssertionError:
            print('#assertion failed in BitAdd for s[%d]' % i)
            print_state()
    s[k] = c[k-1]
    #print_state()
    return s

def BitDec(a, k, m, kappa, bits_to_compute=None):
    r_dprime = types.sint()
    r_prime = types.sint()
    c = types.cint()
    r = [types.sint() for i in range(m)]
    comparison.PRandM(r_dprime, r_prime, r, k, m, kappa)
    #assert(r_prime.value == sum(r[i].value*2**i for i in range(m)) % comparison.program.P)
    pow2 = two_power(k + kappa)
    asm_open(c, pow2 + two_power(k) + a - two_power(m)*r_dprime - r_prime)
    #rval = 2**m*r_dprime.value + r_prime.value
    #assert(rval % 2**m == r_prime.value)
    #assert(rval == (2**m*r_dprime.value + sum(r[i].value*2**i for i in range(m)) % comparison.program.P ))
    try:
        pass#assert(c.value == (2**(k + kappa) + 2**k + (a.value%2**k) - rval) % comparison.program.P)
    except AssertionError:
        print('BitDec assertion failed')
        print('a =', a.value)
        print('a mod 2^%d =' % k, (a.value % 2**k))
    return BitAdd(list(bits(c,m)), r, bits_to_compute)[:-1]


def Pow2(a, l, kappa):
    m = int(ceil(log(l, 2)))
    t = BitDec(a, m, m, kappa)
    x = [types.sint() for i in range(m)]
    pow2k = [types.cint() for i in range(m)]
    for i in range(m):
        pow2k[i] = two_power(2**i)
        t[i] = t[i]*pow2k[i] + 1 - t[i]
    return KMul(t)

def B2U(a, l, kappa):
    pow2a = Pow2(a, l, kappa)
    #assert(pow2a.value == 2**a.value)
    r = [types.sint() for i in range(l)]
    t = types.sint()
    c = types.cint()
    for i in range(l):
        bit(r[i])
    comparison.PRandInt(t, kappa)
    asm_open(c, pow2a + two_power(l) * t + sum(two_power(i)*r[i] for i in range(l)))
    comparison.program.curr_tape.require_bit_length(l + kappa)
    c = list(bits(c, l))
    x = [c[i] + r[i] - 2*c[i]*r[i] for i in range(l)]
    #print ' '.join(str(b.value) for b in x)
    y = PreOR(x, kappa)
    #print ' '.join(str(b.value) for b in y)
    return [1 - y[i] for i in range(l)], pow2a

def Trunc(a, l, m, kappa, compute_modulo=False):
    """ Oblivious truncation by secret m """
    if l == 1:
        if compute_modulo:
            return a * m, 1 + m
        else:
            return a * (1 - m)
    r = [types.sint() for i in range(l)]
    r_dprime = types.sint(0)
    r_prime = types.sint(0)
    rk = types.sint()
    c = types.cint()
    ci = [types.cint() for i in range(l)]
    d = types.sint()
    x, pow2m = B2U(m, l, kappa)
    #assert(pow2m.value == 2**m.value)
    #assert(sum(b.value for b in x) == m.value)
    for i in range(l):
        bit(r[i])
        t1 = two_power(i) * r[i]
        t2 = t1*x[i]
        r_prime += t2
        r_dprime += t1 - t2
    #assert(r_prime.value == (sum(2**i*x[i].value*r[i].value for i in range(l)) % comparison.program.P))
    comparison.PRandInt(rk, kappa)
    r_dprime += two_power(l) * rk
    #assert(r_dprime.value == (2**l * rk.value + sum(2**i*(1 - x[i].value)*r[i].value for i in range(l)) % comparison.program.P))
    asm_open(c, a + r_dprime + r_prime)
    for i in range(1,l):
        ci[i] = c % two_power(i)
        #assert(ci[i].value == c.value % 2**i)
    c_dprime = sum(ci[i]*(x[i-1] - x[i]) for i in range(1,l))
    #assert(c_dprime.value == (sum(ci[i].value*(x[i-1].value - x[i].value) for i in range(1,l)) % comparison.program.P))
    lts(d, c_dprime, r_prime, l, kappa)
    if compute_modulo:
        b = c_dprime - r_prime + pow2m * d
        return b, pow2m
    else:
        pow2inv = Inv(pow2m)
        #assert(pow2inv.value * pow2m.value % comparison.program.P == 1)
        b = (a - c_dprime + r_prime) * pow2inv - d
    return b

def TruncRoundNearestAdjustOverflow(a, length, target_length, kappa):
    t = comparison.TruncRoundNearest(a, length, length - target_length, kappa)
    overflow = t.greater_equal(two_power(target_length), target_length + 1, kappa)
    s = (1 - overflow) * t + old_div(overflow * t, 2)
    return s, overflow

def Int2FL(a, gamma, l, kappa):
    lam = gamma - 1
    s = types.sint()
    comparison.LTZ(s, a, gamma, kappa)
    z = EQZ(a, gamma, kappa)
    a = (1 - 2 * s) * a

    a_bits = BitDec(a, lam, lam, kappa)
    a_bits.reverse()
    b = PreOR(a_bits, kappa)
    t = a * (1 + sum(2**i * (1 - b_i) for i,b_i in enumerate(b)))
    p = - (lam - sum(b))
    if lam > l:
        if types.sfloat.round_nearest:
            v, overflow = TruncRoundNearestAdjustOverflow(t, gamma - 1, l, kappa)
            p = p + overflow
        else:
            v = types.sint()
            comparison.Trunc(v, t, gamma - 1, gamma - l - 1, kappa, False)
            #TODO: Shouldnt this be only gamma
    else:
        v = 2**(l-gamma+1) * t
    p = (p + gamma - 1 - l) * (1 -z)
    return v, p, z, s


##
# Original Function that converts inputs to
# floats, as implemented in legacy code (sfloat).
# It looks like a special adaptation from
# [ABZ13]
# @param v: int or cint value to be transformed to sfloat
# @param vlen: length of v (usually l) in paper
# @param plen: bit-length of supported precision
# @return all basic components of a floating point as in [ABZ13]
def convert_float(v, vlen, plen):
    if v < 0:
        s = 1
    else:
        s = 0
    if v == 0:
        v = 0
        p = 0
        z = 1
    else:
        p = int(floor(log(abs(v), 2))) - vlen + 1
        vv = v
        v = int(round(abs(v) * 2 ** (-p)))
        if v == 2 ** vlen:
            p += 1
            v /= 2
        z = 0
        if p < -2 ** (plen - 1):
            print('Warning: %e truncated to zero' % vv)
            v, p, z = 0, 0, 1
        if p >= 2 ** (plen - 1):  # use it for the comparison
            raise CompilerError('Cannot convert %s to float ' \
                                'with %d exponent bits' % (vv, plen))
    return v, p, z, s


def FLRound(x, mode):
    """ Rounding with floating point output.
    *mode*: 0 -> floor, 1 -> ceil, -1 > trunc """
    v1, p1, z1, s1, l, k = x.v, x.p, x.z, x.s, x.vlen, x.plen
    a = types.sint()
    comparison.LTZ(a, p1, k, x.kappa)
    b = p1.less_than(-l + 1, k, x.kappa)
    v2, inv_2pow_p1 = Trunc(v1, l, -a * (1 - b) * x.p, x.kappa, True)
    c = EQZ(v2, l, x.kappa)
    if mode == -1:
        away_from_zero = 0
        mode = x.s
    else:
        away_from_zero = mode + s1 - 2 * mode * s1
    v = v1 - v2 + (1 - c) * inv_2pow_p1 * away_from_zero
    d = v.equal(two_power(l), l + 1, x.kappa)
    v = d * two_power(l-1) + (1 - d) * v
    v = a * ((1 - b) * v + b * away_from_zero * two_power(l-1)) + (1 - a) * v1
    s = (1 - b * mode) * s1
    z = or_op(EQZ(v, l, x.kappa), z1)
    v = v * (1 - z)
    p = ((p1 + d * a) * (1 - b) + b * away_from_zero * (1 - l)) * (1 - z)
    return v, p, z, s

def TruncPr(a, k, m, kappa=None):
    """ Probabilistic truncation [a/2^m + u]
        where Pr[u = 1] = (a % 2^m) / 2^m
    """
    if isinstance(a, types.cint):
        return shift_two(a, m)

    if kappa is None:
       kappa = 40 

    b = two_power(k-1) + a
    r_prime, r_dprime = types.sint(), types.sint()
    comparison.PRandM(r_dprime, r_prime, [types.sint() for i in range(m)],
                      k, m, kappa)
    two_to_m = two_power(m)
    r = two_to_m * r_dprime + r_prime
    c = (b + r).reveal()
    c_prime = c % two_to_m
    a_prime = c_prime - r_prime
    d = old_div((a - a_prime), two_to_m)
    return d

def SDiv(a, b, l, kappa):
    theta = int(ceil(old_div(log(l / 3.5), log(2))))
    alpha = two_power(2*l)
    beta = old_div(1, types.cint(two_power(l)))
    w = types.cint(int(2.9142 * two_power(l))) - 2 * b
    x = alpha - b * w
    y = a * w
    y = TruncPr(y, 2 * l, l, kappa)
    x2 = types.sint()
    comparison.Mod2m(x2, x, 2 * l + 1, l, kappa, False)
    x1 = (x - x2) * beta
    for i in range(theta-1):
        y = y * (x1 + two_power(l)) + TruncPr(y * x2, 2 * l, l, kappa)
        y = TruncPr(y, 2 * l + 1, l + 1, kappa)
        x = x1 * x2 + TruncPr(x2**2, 2 * l + 1, l + 1, kappa)
        x = x1 * x1 + TruncPr(x, 2 * l + 1, l - 1, kappa)
        x2 = types.sint()
        comparison.Mod2m(x2, x, 2 * l, l, kappa, False)
        x1 = (x - x2) * beta
    y = y * (x1 + two_power(l)) + TruncPr(y * x2, 2 * l, l, kappa)
    y = TruncPr(y, 2 * l + 1, l - 1, kappa)
    return y

def SDiv_mono(a, b, l, kappa):
    theta = int(ceil(old_div(log(l / 3.5), log(2))))
    alpha = two_power(2*l)
    w = types.cint(int(2.9142 * two_power(l))) - 2 * b
    x = alpha - b * w
    y = a * w
    y = TruncPr(y, 2 * l + 1, l + 1, kappa)
    for i in range(theta-1):
        y = y * (alpha + x)
        # keep y with l bits
        y = TruncPr(y, 3 * l, 2 * l, kappa)
        x = x**2
        # keep x with 2l bits
        x = TruncPr(x, 4 * l, 2 * l, kappa)
    y = y * (alpha + x)
    y = TruncPr(y, 3 * l, 2 * l, kappa)
    return y

##
# SDiv as annotated in ABZS12. It perfroms 
# division using rapson newton from approx on 
# 2^l
def SDiv_ABZS12(a, b, l, kappa):
    theta = int(ceil(log(l, 2)))
    x = b
    y = a
    for i in range(theta -1):
        y = y * ((2 ** (l + 1)) - x)
        y = TruncPr(y, 2 * l + 1, l, kappa)
        x = x * ((2 ** (l + 1)) - x)
        x = TruncPr(x, 2 * l + 1, l, kappa)
    y = y * ((2 ** (l + 1)) - x)
    y = TruncPr(y, 2 * l + 1, l, kappa)
    return y

def AdditionKOp(a,b):
    return a + b


def SolvedBits(x_bits, k):
    pow_of_two= [x_bits[i] * 2 ** i  for i in range(k)]
    return KOpL(AdditionKOp, pow_of_two)
