# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

from math import log, floor, ceil
from Compiler.instructions import *
from Compiler.instructions_base import *
from Compiler.config import *
from Compiler import util
from Compiler import AdvInteger
import program
import types
import math
import util
import operator


"""
The Fixed Point Advanced operations from the manual, related to integers.

Most protocols come from [1]

[1] Octavian Catrina and Amitabh Saxena. Secure computation with fixed-point numbers.

"""
##
# Returns the absolute value of a fix point number.
# The method is also applicable to sfloat,
# however, more efficient mechanisms can be devised.
#
# @param x: (sfix)
#
# @return (sfix) unsigned
def abs_fx(x):
    s = x < 0
    return (1 - 2 * s) * x

##
# Floors the input and stores the value into a sflix register
# @param x: coefficient to be floored.
#
# @return floored sint value of x
def floor_fx(x):
    return load_sint(AdvInteger.Trunc(x.v, x.k - x.f, x.f, x.kappa), type(x))


def AppRcr(b, k, f, kappa, simplex_flag=False):
    """
        Approximate reciprocal of [b]:
        Given [b], compute [1/b]
    """
    alpha = types.cint(int(2.9142 * 2**k))
    c, v = Norm(b, k, f, kappa, simplex_flag)
    #v should be 2**{k - m} where m is the length of the bitwise repr of [b]
    d = alpha - 2 * c
    w = d * v
    w = AdvInteger.TruncPr(w, 2 * k, 2 * (k - f))
    # now w * 2 ^ {-f} should be an initial approximation of 1/b
    return w

def Norm(b, k, f, kappa, simplex_flag=False):
    """
        Computes secret integer values [c] and [v_prime] st.
        2^{k-1} <= c < 2^k and c = b*v_prime
    """
    # For simplex, we can get rid of computing abs(b)
    temp = None
    if simplex_flag == False:
        temp = types.sint(b < 0)
    elif simplex_flag == True:
        temp = types.cint(0)

    sign = 1 - 2 * temp # 1 - 2 * [b < 0]
    absolute_val = sign * b

    #next 2 lines actually compute the SufOR for little indian encoding
    bits = absolute_val.bit_decompose(k)[::-1]
    suffixes = AdvInteger.PreOR(bits)[::-1]

    z = [0] * k
    for i in range(k - 1):
        z[i] = suffixes[i] - suffixes[i+1]
    z[k - 1] = suffixes[k-1]

    #doing complicated stuff to compute v = 2^{k-m}
    acc = types.cint(0)
    for i in range(k):
        acc += AdvInteger.two_power(k-i-1) * z[i]

    part_reciprocal = absolute_val * acc
    signed_acc = sign * acc

    return part_reciprocal, signed_acc

##
# Produces the fixed point division of a by b.
# Uses the Goldschmidt method as presented in Catrina10,
def Div(a, b, k, f, kappa, simplex_flag=False):
    #from Compiler.library import print_ln, print_str
    theta = int(ceil(log(k/3.5) / log(2)))
    alpha = AdvInteger.two_power(2*f)
    w = AppRcr(b, k, f, kappa, simplex_flag)
    x = alpha - b * w

    y = a * w
    y = AdvInteger.TruncPr(y, 2*k, f, kappa)

    for i in range(theta):
        y = y * (alpha + x)
        x = x * x
        y = AdvInteger.TruncPr(y, 2*k, 2*f, kappa)
        x = AdvInteger.TruncPr(x, 2*k, 2*f, kappa)

    y = y * (alpha + x)
    y = AdvInteger.TruncPr(y, 2*k, 2*f, kappa)
    return y

##
# For other divisions needed by sfix we have the
# following three routines
from math import ceil, log

def approximate_reciprocal(divisor, k, f, theta):
    """
        returns aproximation of 1/divisor
        where type(divisor) = cint
    """
    from types import cint, Array, MemValue, regint
    from library import for_range, if_ 

    def twos_complement(x):
        bits = x.bit_decompose(k)[::-1]
        bit_array = Array(k, cint)
        bit_array.assign(bits)

        twos_result = MemValue(cint(0))
        @for_range(k)
        def block(i):
            val = twos_result.read()
            val <<= 1
            val += 1 - bit_array[i]
            twos_result.write(val)

        return twos_result.read() + 1

    bit_array = Array(k, cint)
    bits = divisor.bit_decompose(k)[::-1]
    bit_array.assign(bits)

    cnt_leading_zeros = MemValue(regint(0))

    flag = MemValue(regint(0))
    cnt_leading_zeros = MemValue(regint(0))
    normalized_divisor = MemValue(divisor)

    @for_range(k)
    def block(i):
        flag.write(flag.read() | bit_array[i] == 1)
        @if_(flag.read() == 0)
        def block():
            cnt_leading_zeros.write(cnt_leading_zeros.read() + 1)
            normalized_divisor.write(normalized_divisor << 1)

    q = MemValue(AdvInteger.two_power(k))
    e = MemValue(twos_complement(normalized_divisor.read()))

    @for_range(theta)
    def block(i):
        qread = q.read()
        eread = e.read()
        qread += (qread * eread) >> k
        eread = (eread * eread) >> k

        q.write(qread)
        e.write(eread)

    res = q >> cint(2*k - 2*f - cnt_leading_zeros)

    return res


def cint_cint_division(a, b, k, f):
    """
        Goldschmidt method implemented with
        SE aproximation:
        http://stackoverflow.com/questions/2661541/picking-good-first-estimates-for-goldschmidt-division
    """
    from types import cint, Array
    from library import for_range
    # theta can be replaced with something smaller
    # for safety we assume that is the same theta from previous GS method

    theta = int(ceil(log(k/3.5) / log(2)))
    two = cint(2) * AdvInteger.two_power(f)

    sign_b = cint(1) - 2 * cint(b < 0)
    sign_a = cint(1) - 2 * cint(a < 0)
    absolute_b = b * sign_b
    absolute_a = a * sign_a
    w0 = approximate_reciprocal(absolute_b, k, f, theta)

    A = Array(theta, cint)
    B = Array(theta, cint)
    W = Array(theta, cint)

    A[0] = absolute_a
    B[0] = absolute_b
    W[0] = w0
    @for_range(1, theta)
    def block(i):
        A[i] = (A[i - 1] * W[i - 1]) >> f
        B[i] = (B[i - 1] * W[i - 1]) >> f
        W[i] = two - B[i]
    return (sign_a * sign_b) * A[theta - 1]

def sint_cint_division(a, b, k, f, kappa):
    """
        type(a) = sint, type(b) = cint
    """
    from types import cint, sint, Array
    from library import for_range

    theta = int(ceil(log(k/3.5) / log(2)))
    two = cint(2) * AdvInteger.two_power(f)
    sign_b = cint(1) - 2 * cint(b < 0)
    sign_a = sint(1) - 2 * sint(a < 0)
    absolute_b = b * sign_b
    absolute_a = a * sign_a
    w0 = approximate_reciprocal(absolute_b, k, f, theta)

    A = Array(theta, sint)
    B = Array(theta, cint)
    W = Array(theta, cint)

    A[0] = absolute_a
    B[0] = absolute_b
    W[0] = w0

    @for_range(1, theta)
    def block(i):
        A[i] = AdvInteger.TruncPr(A[i - 1] * W[i - 1], 2*k, f, kappa)
        temp = (B[i - 1] * W[i - 1]) >> f
        # no reading and writing to the same variable in a for loop.
        W[i] = two - temp
        B[i] = temp
    return (sign_a * sign_b) * A[theta - 1]


##
# obtains the most significative bit (MSB) 
# of a given input. The size of the vector
# is tuned to the needs of sqrt. 
# @param b: number from which you obtain the
# most significative bit.
# @param k:  number of bits for which
# an output of size (k+1) if even
#  is going to be produced.
# @return z: index array for MSB of size
# k or K+1 if even.
def MSB(b, k):
    # calculation of z
    # x in order 0 - k
    if (k > types.program.bit_length):
        raise OverflowError("The supported bit \
        lenght of the application is smaller than k")

    x_order = b.bit_decompose(k)
    x = [0] * k
    # x i now inverted
    for i in range(k - 1, -1, -1):
        x[k - 1 - i] = x_order[i]
    # y is inverted for PReOR and then restored
    y_order = AdvInteger.PreOR(x)

    # y in order (restored in orginal order
    y = [0] * k
    for i in range(k - 1, -1, -1):
        y[k - 1 - i] = y_order[i]

    # obtain z
    z = [0] * (k + 1 - k % 2)
    for i in range(k - 1):
        z[i] = y[i] - y[i + 1]
    z[k - 1] = y[k - 1]

    return z


##
# Similar to norm_SQ, saves rounds by not 
# calculating v and c. 
#
# @param b: sint input to be normalized. 
# @param k: bitsize of the input, by definition
# its value is either sfix.k or program.bit_lengthh 
# @return m_odd: the parity of  most signficative bit index m
# @return m: index of most significative bit
# @return w: 2^m/2 or 2^ (m-1) /2
def norm_simplified_SQ(b, k):
    z = MSB(b, k)
    # construct m
    m = types.sint(0)
    m_odd = 0
    for i in range(k):
        m = m + (i + 1) * z[i]
        # determine the parity of the input
        if (i % 2 == 0):
            m_odd = m_odd + z[i]

    # construct w,
    k_over_2 = k / 2 + 1
    w_array = [0] * (k_over_2)
    w_array[0] = z[0]
    for i in range(1, k_over_2):
        w_array[i] = z[2 * i - 1] + z[2 * i]

    # w aggregation
    w = types.sint(0)
    for i in range(k_over_2):
        w += (2 ** i) * w_array[i]

    # return computed values
    return m_odd, m, w

##
# Calculates the normSQ of a number
# @param x: number from which the norm is going to be extracted
# @param k: bitsize of x
#
# @return c: where c = x*v where c is bounded by 2^{k-1} and 2^k
# @return v: where v = 2^k-m
# @return m: where m = MSB
# @return w: where w = 2^{m/2} if m is oeven and 2^{m-1 / 2} otherwise
def norm_SQ(b, k): 
    # calculation of z
    # x in order 0 - k
    z = MSB(b,k)
    # now reverse bits of z[i] to generate v
    v = types.sint(0)
    for i in range(k):
        v += (2**(k - i - 1)) * z[i]
    c = b * v
  
    # construct m
    m = types.sint(0)
    for i in range(k):
        m = m + (i+1) * z[i]

    # construct w, changes from what is on the paper
    # and the documentation
    k_over_2= k/2+1#int(math.ceil((k/2.0)))+1
    w_array = [0]*(k_over_2 )
    w_array[0] = z[0]
    for i in range(1, k_over_2):
        w_array[i] = z[2 * i - 1] + z[2 * i]

    w = types.sint(0)
    for i in range(k_over_2):
        w += (2 ** i) * w_array[i]

    # return computed values
    return c, v, m, w

