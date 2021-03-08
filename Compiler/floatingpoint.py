
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

##
# @file
# floating point operations
# File documentation
#
from math import log, floor, ceil
from Compiler.instructions import *
from Compiler.types import *
import program
import sys
import types
from Compiler import AdvInteger

##
## Helper functions for floating point arithmetic
##

def shift_two(n, pos):
    if pos < 63:
        return n >> pos
    else:
        res = (n >> (pos%63))
        for i in range(pos / 63):
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
    c = AdvInteger.EQZ(fl_a.p - fl_b.p, fl_a.plen, fl_a.kappa)
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


def FlAbs(a):
    return types.sfloat(v = a.v, p = a.p, z = a.z, s = types.sint(0), err = a.err)



def TruncRoundNearestAdjustOverflow(a, length, target_length, kappa):
    t = AdvInteger.TruncRoundNearest(a, length, length - target_length, kappa)
    overflow = t.greater_equal(AdvInteger.two_power(target_length), target_length + 1, kappa)
    s = (1 - overflow) * t + overflow * t / 2
    return s, overflow


def Int2FL(a, gamma, l, kappa):
    #from Compiler.library import print_ln, print_str
    lam = gamma - 1
    s = types.sint()
    AdvInteger.LTZ(s, a, gamma, kappa)
    z = AdvInteger.EQZ(a, gamma, kappa)
    a = (1 - 2 * s) * a

    a_bits = AdvInteger.BitDec(a, lam, lam, kappa)
    a_bits.reverse()
    b = AdvInteger.PreOR(a_bits)
    t = a * (1 + sum(2**i * (1 - b_i) for i,b_i in enumerate(b)))
    p = - (lam - sum(b))
    if lam > l:
        if types.sfloat.round_nearest:
            v, overflow = TruncRoundNearestAdjustOverflow(t, gamma - 1, l, kappa)
            p = p + overflow
        else:
            v = types.sint()
            AdvInteger.Trunc(v, t, gamma - 1, gamma - l - 1, kappa, False)
            #TODO: Shouldnt this be only gamma
    else:
        v = 2**(l-gamma+1) * t
    p = (p + gamma - 1 - l) * (1 -z)
    return v, p, z, s


##
# Original Function that converts inputs to
# floats, as implemented in legacy code (sfloat).
# It looks like a special adaptation from [ABZ13]
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
            print 'Warning: %e truncated to zero' % vv
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
    AdvInteger.LTZ(a, p1, k, x.kappa)
    b = p1.less_than(-l + 1, k, x.kappa)
    v2, inv_2pow_p1 = AdvInteger.Oblivious_Trunc(v1, l, -a * (1 - b) * x.p, x.kappa, True)
    c = AdvInteger.EQZ(v2, l, x.kappa)
    if mode == -1:
        away_from_zero = 0
        mode = x.s
    else:
        away_from_zero = mode + s1 - 2 * mode * s1
    v = v1 - v2 + (1 - c) * inv_2pow_p1 * away_from_zero
    d = v.equal(AdvInteger.two_power(l), l + 1, x.kappa)
    v = d * AdvInteger.two_power(l-1) + (1 - d) * v
    v = a * ((1 - b) * v + b * away_from_zero * AdvInteger.two_power(l-1)) + (1 - a) * v1
    s = (1 - b * mode) * s1
    z = AdvInteger.or_op(AdvInteger.EQZ(v, l, x.kappa), z1)
    v = v * (1 - z)
    p = ((p1 + d * a) * (1 - b) + b * away_from_zero * (1 - l)) * (1 - z)
    return v, p, z, s


def SDiv(a, b, l, kappa):
    theta = int(ceil(log(l / 3.5) / log(2)))
    alpha = AdvInteger.two_power(2*l)
    beta = 1 / types.cint(AdvInteger.two_power(l))
    w = types.cint(int(2.9142 * AdvInteger.two_power(l))) - 2 * b
    x = alpha - b * w
    y = a * w
    y = AdvInteger.TruncPr(y, 2 * l, l, kappa)
    x2 = types.sint()
    AdvInteger.Mod2m(x2, x, 2 * l + 1, l, kappa, False)
    x1 = (x - x2) * beta
    for i in range(theta-1):
        y = y * (x1 + two_power(l)) + AdvInteger.TruncPr(y * x2, 2 * l, l, kappa)
        y = AdvInteger.TruncPr(y, 2 * l + 1, l + 1, kappa)
        x = x1 * x2 + AdvInteger.TruncPr(x2**2, 2 * l + 1, l + 1, kappa)
        x = x1 * x1 + AdvInteger.TruncPr(x, 2 * l + 1, l - 1, kappa)
        x2 = types.sint()
        AdvInteger.Mod2m(x2, x, 2 * l, l, kappa, False)
        x1 = (x - x2) * beta
    y = y * (x1 + two_power(l)) + AdvInteger.TruncPr(y * x2, 2 * l, l, kappa)
    y = AdvInteger.TruncPr(y, 2 * l + 1, l - 1, kappa)
    return y

def SDiv_mono(a, b, l, kappa):
    theta = int(ceil(log(l / 3.5) / log(2)))
    alpha = two_power(2*l)
    w = types.cint(int(2.9142 * two_power(l))) - 2 * b
    x = alpha - b * w
    y = a * w
    y = AdvInteger.TruncPr(y, 2 * l + 1, l + 1, kappa)
    for i in range(theta-1):
        y = y * (alpha + x)
        # keep y with l bits
        y = AdvInteger.TruncPr(y, 3 * l, 2 * l, kappa)
        x = x**2
        # keep x with 2l bits
        x = AdvInteger.TruncPr(x, 4 * l, 2 * l, kappa)
    y = y * (alpha + x)
    y = AdvInteger.TruncPr(y, 3 * l, 2 * l, kappa)
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
        y = AdvInteger.TruncPr(y, 2 * l + 1, l, kappa)
        x = x * ((2 ** (l + 1)) - x)
        x = AdvInteger.TruncPr(x, 2 * l + 1, l, kappa)
    y = y * ((2 ** (l + 1)) - x)
    y = AdvInteger.TruncPr(y, 2 * l + 1, l, kappa)
    return y

##
# Converts an sfloat to an IEEE sregint
def sfloat_to_ieee(a):
   vq=a.v
   pq=a.p
   sq=a.s
   zq=a.z
   eq=a.err

   # We have to take the top bit off v, if non-zero
   # Easier to do this now before converting to an sregint
   vq=vq-(1-zq)*(1<<(a.vlen-1))

   # Now if sfloat.vlen>64 we cannot convert it as is,
   # as sregint only holds 64 bit values. So we need
   # to shift down. We do this now for values with
   # vlen>64 as doing it now is expensive.
   # We then shift down for  53<= vlen <=64 in the
   # sregint type as this is cheaper there

   effective_vlen=a.vlen
   if a.vlen>64:
     effective_vlen=53
     vq=vq>>(a.vlen-53)

   v2=sregint(vq)

   # Now we shift the v2 value into the correct position
   if effective_vlen<53:
     v2=v2 << (53-effective_vlen)

   if effective_vlen>53:
     v2=v2 >> (effective_vlen-53)

   # ok2 is a flag which says if we should end up with a NaN
   okq=(eq==0)
   ok2=sbit(okq)

   # Now we have to convert the p bit
   p2=sregint((1-zq)*(pq+a.vlen+1023-1))
   # If plen>11 then we need to make sure p2 is not too big
   if (a.plen>11):
     # This is the big pattern 1111..1100..00 with eleven ones at the end
     p2test=p2 & 0xFFFFFFFFFFFFFF800
     ok2=ok2 & p2test.eqz()
     
   # Now convert sq to an sbit
   s2 = sbit(sq)

   # Now pack into a IEEE vector in the sregint space
   temp = (p2 << 52) ^ v2
   x2=sregint()
   sintbit(x2,temp,s2,63)

   # Now need to deal with the zero flag
   z2=sbit(zq)
   x2 = (~z2) & x2

   # Now deal with the error flag
   #   - This bit pattern is an IEEE NaN
   NaN = sregint(0x7FF0000000000001)
   x2 = (ok2 & x2) ^ ((~ok2) & NaN )

   return x2


##
# Converts an IEEE sregint to an sfloat
def ieee_to_sfloat(x2):
   # We take the bit 63 of the IEEE as
   s2 = sbit()
   bitsint(s2,x2,63)
   sq = sint(s2)

   # Test for zero
   z2 = x2.eqz()

   # We know that to get v_q we need to have the most significant bit
   # of v_q = 1 (unless z2=1) so when we whant to shift down the value
   # v_2, we make the first bit 1 as well and it couses to have a
   # negative value which does not effect to any thing but for vlen >= 64
   # which will be solved by using unsiigned conversion of sregint to sint.
   v2 = x2 << 11
   v2a = sregint()
   sintbit(v2a,v2,~z2,63)

   # Here we make the bit 63 zero
   zero = sbit(0)
   x2a = sregint()
   sintbit(x2a,x2,zero,63)

   # Next 11 bits of the IEEE will be placed in exponent p of
   # sfloat but we have to fit into the p_len.
   #   - Note we have no issue if plen>11 since IEEE values
   #     will convert OK
   #   - But if plen<11 we may need to signal an error
   p2 = x2a >> 52
   # To deal with NaN, we need to just check if all the
   # bits of p2 are 1, if yes,then return err_q= 1.
   e2=(p2==2047)

   p2d=(-(sfloat.vlen-1)-1023)+p2
   pq = sint(p2d)
   # Now check for issue re plen (note brackets to avoid too many additions of sregints)
   if (sfloat.plen<11):
      mask=0xFFFFFFFFFFFFFFFF-(2**sfloat.plen-1)
      p2test=p2d & mask
      p2t=sbit()
      bitsint(p2t,p2test,63)
      p2t = p2t | (p2test.eqz()) 
      e2=e2 ^ (~p2t)

   # If vlen is equal to 64 bits, then we need to have an unsigned
   # converstion for sregint to sint in order to avoid having
   # negative number from previous step as in previous step we
   # have 64 bits.
   if sfloat.vlen>=64:
     vqa = sint.convert_unsigned_to_sint(v2a)
     vq = vqa << (sfloat.vlen-64)

   if sfloat.vlen<64:
     v2a = v2a>>(64-sfloat.vlen)
     vq = sint(v2a)

   # Now need to deal with the zero flags
   zq=sint(z2)

   # Convert error flag to sfloat domain
   eq=sint(e2)

   a = sfloat(vq, pq, zq, sq, eq)

   return a

