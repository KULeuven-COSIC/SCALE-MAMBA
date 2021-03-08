# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

from math import log, floor, ceil
from Compiler.instructions import *
from Compiler.instructions_base import *
from Compiler.config import *
from Compiler import util
import program
import types
import math
import util
import operator


"""
The simple Advanced operations from the manual, related to integers.

Most protocols come from [1], with a few subroutines described in [2].

Function naming of comparison routines is as in [1,2], with k always
representing the integer bit length, and kappa the statistical security
parameter.

Some of these routines were implemented before the cint/sint classes, so use
the old-fasioned Register class and assembly instructions instead of operator
overloading.

[1] https://www1.cs.fau.de/filepool/publications/octavian_securescm/smcint-scn10.pdf
[2] https://www1.cs.fau.de/filepool/publications/octavian_securescm/SecureSCM-D.9.2.pdf
"""


####################################
######### BASIC OPERATORS  #########
####################################


def or_op(a, b, void=None):
    return a + b - a * b

def mul_op(a, b, void=None):
    return a * b

def addition_op(a,b):
    return a + b

def xor_op(a, b, void=None):
    return a + b - 2* a * b

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

# Register version of carry...
def reg_carry(b, a, compute_p):
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



####################################
######### HELPER FUNCTIONS #########
####################################


def two_power(n):
    if isinstance(n, int) and n < 31:
        return 2**n
    else:
        max = types.cint(1) << 31
        res = 2**(n%31)
        for i in range(n / 31):
            res *= max
        return res

inverse_of_two = {}

def divide_by_two(res, x):
    """ Faster clear division by two using a cached value of 2^-1 mod p """
    from program import Program
    import types
    block = Program.prog.curr_block
    if len(inverse_of_two) == 0 or block not in inverse_of_two:
        inverse_of_two[block] = types.cint(1) / 2
    mulc(res, x, inverse_of_two[block])

def bits(a,m):
    """ Get the bits of an int """
    if isinstance(a, (int, long)):
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

def ld2i(c, n):
    """ Load immediate 2^n into clear GF(p) register c """
    t1 = program.curr_block.new_reg('c')
    ldi(t1, 2 ** (n % 30))
    for i in range(n / 30):
        t2 = program.curr_block.new_reg('c')
        mulci(t2, t1, 2 ** 30)
        t1 = t2
    movc(c, t1)


####################################
#### SECTION 14.1 OF THE MANUAL ####
####################################

def Inv(a):
    """ Invert a non-zero value """
    t = [types.sint() for i in range(2)]
    c = [types.cint() for i in range(2)]
    one = types.cint()
    ldi(one, 1)
    square(t[0],t[1]);
    s = t[0]*a
    asm_open(c[0], s)
    divc(c[1], one, c[0])
    return c[1]*t[0]


####################################
#### SECTION 14.2 OF THE MANUAL ####
####################################


def KOpL(op, a):
    k = len(a)
    if k == 1:
        return a[0]
    else:
        t1 = KOpL(op, a[:k/2])
        t2 = KOpL(op, a[k/2:])
        return op(t1, t2)

# KOR is used so much we have a short cut
def KOR(items):
  return KOpL(or_op, items)

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
        for j in range(kmax/(2**(i+1))):
            y = two_power(i) + j*two_power(i+1) - 1
            for z in range(1, 2**i+1):
                if y+z < k:
                    output[y+z] = op(output[y], output[y+z], j != 0)
    return output

# PreOR is used so much we have a short cut
def PreOR(items):
  return PreOpL(or_op, items)

def PreOpN(op, items):
    """ Naive PreOp algorithm """
    k = len(items)
    output = [None]*k
    output[0] = items[0]
    for i in range(1, k):
        output[i] = op(output[i-1], items[i])
    return output

def SumBits(x_bits, k):
    pow_of_two= [x_bits[i] * 2 ** i  for i in range(k)]
    return KOpL(addition_op, pow_of_two)

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


# from WP9 report
# length of a is even
def CarryOutAux(d, a):
    #from Compiler.library import print_ln, print_str
    k = len(a)
    if k > 1 and k % 2 == 1:
        a.append(None)
        k += 1
    u = [None]*(k/2)
    a = a[::-1]
    if k > 1:
        #print_ln('\tCarryOutAux : %s',k)
        for i in range(k/2):
            u[i] = reg_carry(a[2*i+1], a[2*i], i != k/2-1)
        #xxxxx
        #u_open0 = [types.cint() for i in range(k/2)]
        #u_open1 = [types.cint() for i in range(k/2)]
        #for i in range(k/2):
        #  print_str('\t\t\t u.%s : ',i)
        #  if i!=(k/2-1):
        #    asm_open(u_open0[i], u[i][0])
        #    print_reg(u_open0[i])
        #  else:
        #    print_str('-')
        #  asm_open(u_open1[i], u[i][1])
        #  print_str(' ')
        #  print_reg(u_open1[i])
        #  print_ln('')
        # xxxxx
        CarryOutAux(d, u[:k/2][::-1])
    else:
        movs(d, a[0][1])

# carry out with carry-in bit c
def CarryOut(res, a, b, c):
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

    CarryOutAux(res, d[::-1])

def BitAdd(a, b, bits_to_compute=None):
    """ Add the bits a[k-1], ..., a[0] and b[k-1], ..., b[0], return k+1
        bits s[0], ... , s[k] """
    k = len(a)
    if not bits_to_compute:
        bits_to_compute = range(k)
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
        print 'a: ',
        for i in range(k):
            print '%d ' % a[i].value,
        print '\nb: ',
        for i in range(k):
            print '%d ' % b[i].value,
        print '\nd: ',
        for i in range(k):
            print '%d ' % d[i][0].value,
        print '\n   ',
        for i in range(k):
            print '%d ' % d[i][1].value,
        print '\n\npg:',
        for i in range(k):
            print '%d ' % pg[i][0].value,
        print '\n    ',
        for i in range(k):
            print '%d ' % pg[i][1].value,
        print ''

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
            print '#assertion failed in BitAdd for s[%d]' % i
            print_state()
    s[k] = c[k-1]
    #print_state()
    return s

def BitIncrement(a, bits_to_compute=None):
    """ Add the bits a[k-1], ..., a[0] and 0, ..., 0, 1, return k+1
        bits s[0], ... , s[k] """
    k = len(a)
    if not bits_to_compute:
        bits_to_compute = range(k)
    d = [None] * k
    for i in range(1,k):
        d[i] = (a[i], 0)
        #assert(a[i].value == 0 or a[i].value == 1)
        #assert(d[i][0].value == 0 or d[i][0].value == 1)
    d[0] = (None, a[0])
    pg = PreOpL(carry, d)
    c = [pair[1] for pair in pg]

    # (for testing)
    def print_state():
        print 'a: ',
        for i in range(k):
            print '%d ' % a[i].value,
        print '\nd: ',
        for i in range(k):
            print '%d ' % d[i][0].value,
        print '\n   ',
        for i in range(k):
            print '%d ' % d[i][1].value,
        print '\n\npg:',
        for i in range(k):
            print '%d ' % pg[i][0].value,
        print '\n    ',
        for i in range(k):
            print '%d ' % pg[i][1].value,
        print ''

    for bit in c:
        pass#assert(bit.value == 0 or bit.value == 1)
    s = [None] * (k+1)
    if 0 in bits_to_compute:
        s[0] = a[0] + 1 - 2*c[0]
        bits_to_compute.remove(0)
    #assert(c[0].value == a[0].value*b[0].value)
    #assert(s[0].value == 0 or s[0].value == 1)
    for i in bits_to_compute:
        s[i] = a[i] + c[i-1] - 2*c[i]
        try:
            pass#assert(s[i].value == 0 or s[i].value == 1)
        except AssertionError:
            print '#assertion failed in BitIncrement for s[%d]' % i
            print_state()
    s[k] = c[k-1]
    #print_state()
    return s


def BitLT(res, a, b, kappa):
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
    CarryOut(t[0], a_bits[::-1], s[0][::-1], 1)
    subsfi(res, t[0], 1)
    return a_bits, s[0]

# LT bit comparison on shared bit values
#  Assumes b has the larger size
#   - From the paper
#        Unconditionally Secure Constant-RoundsMulti-party Computation
#        for Equality,Comparison, Bits and Exponentiation
def BitLTFull(a, b, bit_length):
  from Compiler.types import sint
  e = [sint(0)]*bit_length
  g = [sint(0)]*bit_length
  h = [sint(0)]*bit_length
  for i in range(bit_length):
    e[bit_length-i-1]=a[i]+b[i]-2*a[i]*b[i] # Compute the XOR (reverse order of e for PreOpL)
  f = PreOR(e)
  g[bit_length-1] = f[0]
  for i in range(bit_length-1):
    g[i] = f[bit_length-i-1]-f[bit_length-i-2]  # reverse order of f due to PreOpL
  ans = sint(0)
  for i in range(bit_length):
    h[i] = g[i]*b[i]
    ans = ans + h[i]
  return ans



# Statistically secure BitDec
def BitDec(a, k, m, kappa, bits_to_compute=None):
    r_dprime = types.sint()
    r_prime = types.sint()
    c = types.cint()
    r = [types.sint() for i in range(m)]
    PRandM(r_dprime, r_prime, r, k, m, kappa)
    #assert(r_prime.value == sum(r[i].value*2**i for i in range(m)) % AdvInteger.program.P)
    pow2 = two_power(k + kappa)
    asm_open(c, pow2 + two_power(k) + a - two_power(m)*r_dprime - r_prime)
    #rval = 2**m*r_dprime.value + r_prime.value
    #assert(rval % 2**m == r_prime.value)
    #assert(rval == (2**m*r_dprime.value + sum(r[i].value*2**i for i in range(m)) % AdvInteger.program.P ))
    try:
        pass#assert(c.value == (2**(k + kappa) + 2**k + (a.value%2**k) - rval) % AdvInteger.program.P)
    except AssertionError:
        print 'BitDec assertion failed'
        print 'a =', a.value
        print 'a mod 2^%d =' % k, (a.value % 2**k)
    return BitAdd(list(bits(c,m)), r, bits_to_compute)[:-1]


# Exact BitDec with no need for a statistical gap
#   - From the paper
#        Unconditionally Secure Constant-RoundsMulti-party Computation
#        for Equality,Comparison, Bits and Exponentiation
def BitDecFullBig(a):
    #from Compiler.library import print_ln, print_str
    from Compiler.types import sint, regint
    from Compiler.library import do_while
    p=program.P
    bit_length = p.bit_length()
    abits = [sint(0)]*bit_length
    bbits = [sint(0)]*bit_length
    pbits = list(bits(p,bit_length+1))
    # Loop until we get some random integers less than p
    @do_while
    def get_bits_loop():
       # How can we do this with a vectorized load of the bits? XXXX
       tbits = [sint(0)]*bit_length
       for i in range(bit_length):
         tbits[i] = sint.get_random_bit()
         tbits[i].store_in_mem(i)
       c = regint(BitLTFull(tbits, pbits, bit_length).reveal())
       return (c!=1)
    for i in range(bit_length):
       bbits[i]=sint.load_mem(i)
    b = SumBits(bbits, bit_length)
    c = (a-b).reveal()
    czero = (c==0)
    d = BitAdd(list(bits(c,bit_length)), bbits)
    q = BitLTFull( pbits, d, bit_length+1)
    f = list(bits((1<<bit_length)-p,bit_length))
    g = [sint(0)]*(bit_length+1)
    for i in range(bit_length):
        g[i]=f[i]*q;
    h = BitAdd(d, g)
    for i in range(bit_length):
       abits[i] = (1-czero)*h[i]+czero*bbits[i]
    return abits



# Exact BitDec with no need for a statistical gap
#   - From the paper
#        Multiparty Computation for Interval, Equality, and Comparison without
#        Bit-Decomposition Protocol
#   - For small p only as we convert to regint to make things easier
def BitDecFull(a):
    from Compiler.types import sint, regint
    from Compiler.library import do_while
    p=program.P
    bit_length = p.bit_length()
    if bit_length>63:
        return BitDecFullBig(a)
    abits = [sint(0)]*bit_length
    bbits = [sint(0)]*bit_length
    pbits = list(bits(p,bit_length))
    # Loop until we get some random integers less than p
    @do_while
    def get_bits_loop():
       # How can we do this with a vectorized load of the bits? XXXX
       tbits = [sint(0)]*bit_length
       for i in range(bit_length):
         tbits[i] = sint.get_random_bit()
         tbits[i].store_in_mem(i)
       c = regint(BitLTFull(tbits, pbits, bit_length).reveal())
       return (c!=1)
    for i in range(bit_length):
       bbits[i]=sint.load_mem(i)
    b = SumBits(bbits, bit_length)
    # Reveal c in the correct range
    c = regint((a-b).reveal())
    bit=c<0
    c=c+p*bit
    czero = (c==0)
    t = (p-c).bit_decompose(bit_length)
    q = 1-BitLTFull( bbits, t, bit_length)
    fbar=((1<<bit_length)+c-p).bit_decompose(bit_length)
    fbard=regint(c).bit_decompose(bit_length)
    g = [sint(0)]*(bit_length)
    for i in range(bit_length):
       g[i]=(fbar[i]-fbard[i])*q+fbard[i];
    h = BitAdd(bbits, g)
    for i in range(bit_length):
       abits[i] = (1-czero)*h[i]+czero*bbits[i]
    return abits




####################################
#### SECTION 14.3 OF THE MANUAL ####
####################################

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
    BitLT(u, c_prime, r, kappa)
    mulm(t[4], u, c2m)
    submr(t[5], c_prime, r_prime)
    adds(a_prime, t[5], t[4])
    return r_dprime, r_prime, c, c_prime, u, t, c2k1

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
    PRandM(r_dprime, r_prime, [types.sint() for i in range(m)], k, m, kappa)
    two_to_m = two_power(m)
    r = two_to_m * r_dprime + r_prime
    c = (b + r).reveal()
    c_prime = c % two_to_m
    a_prime = c_prime - r_prime
    d = (a - a_prime) / two_to_m
    return d

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
    BitLT(u, c_prime, r[:-1], kappa)
    bit = ((c - c_prime) / (cint(1) << (m - 1))) % 2
    xor = bit + u - 2 * bit * u
    prod = xor * r[-1]
    # u_prime = xor * u + (1 - xor) * r[-1]
    u_prime = bit * u + u - 2 * bit * u + r[-1] - prod
    a_prime = (c % (cint(1) << m)) - r_prime + (cint(1) << m) * u_prime
    d = (a - a_prime) / (cint(1) << m)
    rounding = xor + r[-1] - 2 * prod
    return d + rounding

def Oblivious_Trunc(a, l, m, kappa, compute_modulo=False):
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
    #assert(r_prime.value == (sum(2**i*x[i].value*r[i].value for i in range(l)) % AdvInteger.program.P))
    PRandInt(rk, kappa)
    r_dprime += two_power(l) * rk
        #assert(r_dprime.value == (2**l * rk.value + sum(2**i*(1 - x[i].value)*r[i].value for i in range(l)) % AdvInteger.program.P))
    asm_open(c, a + r_dprime + r_prime)
    for i in range(1,l):
        ci[i] = c % two_power(i)
        #assert(ci[i].value == c.value % 2**i)
    c_dprime = sum(ci[i]*(x[i-1] - x[i]) for i in range(1,l))
    #assert(c_dprime.value == (sum(ci[i].value*(x[i-1].value - x[i].value) for i in range(1,l)) % AdvInteger.program.P))
    lts(d, c_dprime, r_prime, l, kappa)
    if compute_modulo:
        b = c_dprime - r_prime + pow2m * d
        return b, pow2m
    else:
        pow2inv = Inv(pow2m)
        #assert(pow2inv.value * pow2m.value % AdvInteger.program.P == 1)
        b = (a - c_dprime + r_prime) * pow2inv - d
    return b

def Pow2(a, l, kappa):
    m = int(ceil(log(l, 2)))
    t = BitDec(a, m, m, kappa)
    x = [types.sint() for i in range(m)]
    pow2k = [types.cint() for i in range(m)]
    for i in range(m):
        pow2k[i] = two_power(2**i)
        t[i] = t[i]*pow2k[i] + 1 - t[i]
    return KOpL(mul_op, t)

def B2U(a, l, kappa):
    pow2a = Pow2(a, l, kappa)
    #assert(pow2a.value == 2**a.value)
    r = [types.sint() for i in range(l)]
    t = types.sint()
    c = types.cint()
    for i in range(l):
        bit(r[i])
    PRandInt(t, kappa)
    asm_open(c, pow2a + two_power(l) * t + sum(two_power(i)*r[i] for i in range(l)))
    program.curr_tape.require_bit_length(l + kappa)
    c = list(bits(c, l))
    x = [c[i] + r[i] - 2*c[i]*r[i] for i in range(l)]
    #print ' '.join(str(b.value) for b in x)
    y = PreOR(x)
    #print ' '.join(str(b.value) for b in y)
    return [1 - y[i] for i in range(l)], pow2a


def LTZ(s, a, k, kappa):
    """
    s = (a ?< 0)

    k: bit length of a
    """
    t = program.curr_block.new_reg('s')
    Trunc(t, a, k, k - 1, kappa, True)
    subsfi(s, t, 0)


def EQZ(a, k, kappa):
    r_dprime = types.sint()
    r_prime = types.sint()
    c = types.cint()
    d = [None]*k
    r = [types.sint() for i in range(k)]
    PRandM(r_dprime, r_prime, r, k, k, kappa)
    startopen(a + two_power(k) * r_dprime + r_prime)# + 2**(k-1))
    stopopen(c)
    for i,b in enumerate(bits(c, k)):
        d[i] = b + r[i] - 2*b*r[i]
    #return 1 - KOR(d, kappa)
    return 1 - KOR(d)
