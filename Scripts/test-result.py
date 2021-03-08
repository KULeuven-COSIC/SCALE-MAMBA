#!/usr/bin/python2

# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.


import struct
import inspect
import sys
import operator
import math
from optparse import OptionParser
from Compiler.core import *
from Compiler.core import _sfix, _cfix
from Compiler import types, util


def extended_gcd(aa, bb):
    lastremainder, remainder = abs(aa), abs(bb)
    x, lastx, y, lasty = 0, 1, 1, 0
    while remainder:
        lastremainder, (quotient, remainder) = remainder, divmod(lastremainder, remainder)
        x, lastx = lastx - quotient*x, x
        y, lasty = lasty - quotient*y, y
    return lastremainder, lastx * (-1 if aa < 0 else 1), lasty * (-1 if bb < 0 else 1)

def modinv(a, m):
    g, x, y = extended_gcd(a, m)
    if g != 1:
        raise ValueError
    return x % m


usage = "usage: %prog [options] sourcefile"
parser = OptionParser(usage=usage)
parser.add_option("-s", "--stop-on-fail", action="store_true", dest="stop_on_fail")
options,args = parser.parse_args()
if len(args) != 1:
    parser.print_help()
    exit(2)

sshare = open('Data/SharingData.txt')
p = int(sshare.readline())
print 'p = %d' % p
param = int(math.ceil(p.bit_length()/8.0))*8
print 'param = %d' % param
byte_length = int(math.ceil(param / 64.0)) * 8
print 'byte_length = %d' % byte_length
#print byte_length
R = 2 ** (8 * byte_length) % p
R_inv = modinv(R, p)
assert (R_inv * R % p) == 1
#print "R = %s" % hex(R)

program.P = p
program.bit_length = param
program.name = args[0]
sfloat.plen = types.sfloat.plen
sfloat.vlen = types.sfloat.vlen

cfloat.plen = types.cfloat.plen
cfloat.vlen = types.cfloat.vlen


sfix.f = types.sfix.f
sfix.k = types.sfix.k

cfix.f = types.cfix.f
cfix.k = types.cfix.k

mem = open('Data/Memory-P0')
#print hex(mem.tell())
size_c = int(mem.readline())
base = mem.tell()
mem.seek(size_c * byte_length, 1)
#print hex(mem.tell())
size_s = int(mem.readline())
int_base = mem.tell()
print 'size_c = %d  size_s =%d\n' % (size_c, size_s)

def read_modp(address):
    # print '\tread_modp'
    # print '\ta %d' % address
    mem.seek(base + address * byte_length)
    # print '\tb %s' % hex(mem.tell())
    n = 0
    for i in range(byte_length / 8):
        n += struct.unpack('<Q', mem.read(8))[0] << (i * 64)
        # print '\tc %d %s' %(i,hex(n))
    res = n * R_inv % p
    if res > p / 2:
        res -= p
    return res

def read_int(address):
    # print '\tread_int'
    # print '\ta %d' % address
    mem.seek(int_base + address * 8)
    # print '\tb %s' % hex(mem.tell())
    return struct.unpack('<q', mem.read(8))[0]

def read_mem(address, value=None):
    if isinstance(value, MemValue):
        value = value.value
    if isinstance(value, (regint, bool)):
        return read_int(address)
    elif isinstance(value, (sregint.type, sbit.type)):
        return read_int(address)
    else:
        return read_modp(address)
# supported fixed error
# absoulte error.  Delta between correct answer
# and calculated answer
# math.abs(mpc_math.sin(sfix(1)) - math.sin(1))) < fx_error
fx_error = 1e-3

# supported floating point error
# relative error.  Difference between expected precision
# and obtained precision +/- some tolerance bits (fl_error) 
# if difference > 1, we hard set fl_error to 1:
# math.abs(mpc_math.sin(sfix(1)) - math.sin(1))) < /
# 2 ** (int(math.floor(math.log(abs(value), 2))) - t.vlen + 1)
# else:
# math.abs(mpc_math.sin(sfix(1)) - math.sin(1))) < /
# 2 ** (- t.vlen + fl_error)
fl_error = 3

def test_value(value, mem_value, lineno, index, lower=None, upper=None):
    fail = False
    if upper:
        if lower > mem_value or mem_value >= upper:
            fail = True
            print "Failure in line %d, index %d: read %s, expected in [%s,%s), %s" % \
                (lineno, index, mem_value, lower, upper, type(value))
    else:
        if lower is None:
            lower = value
        if isinstance(lower, float):
           if abs(lower - mem_value) > fx_error:
                fail = True
        else:
           if lower % p != mem_value % p:
                fail = True
        if fail:
            print "Failure in line %d, index %d: read %s, expected %s, %s" % \
                (lineno, index, mem_value, lower, type(value))
    if fail and options.stop_on_fail:
        exit(1)


##
# tests all outputs. for sfloat it also tests
# all internat parameters,scuh as its sign flag (s)
# and its zero flag (z)
def test(value, lower=None, upper=None, prec=None):    
    lineno = inspect.currentframe().f_back.f_lineno
    addr = lineno
    if isinstance(value, (Vector, list)):
        addr *= 1000
        values = value
    else:
        if isinstance(value, (F, CF, _sfix, _cfix)):
            addr *= 1000
        values = [value]
    for i,value in enumerate(values):
        t = type(value)
        if isinstance(value, float) and not isinstance(value, (_cfix, _sfix)):
            v = read_mem(1000 + addr + i)
            pp = read_mem(1250 + addr + i)
            z = read_mem(1500 + addr + i)
            s = read_mem(1750 + addr + i)
            t = sfloat
            #close values to 0 are implicitly casted by the following
            #line to 0 promting errors
            mem_value = (1 - 2*s)*(1 - z)*v*float(2**(int(pp)))
            if i == 0:
                print lineno, mem_value
            if value == 0:
                test_value(value, z, lineno, i, 1)
                test_value(value, v, lineno, i, 0)
                #test_value(value, pp, lineno, i, 0)
                #test_value(value, s, lineno, i, 0)
            else:
                test_value(value, v, lineno, i, 2**(t.vlen-1), 2**t.vlen)
                test_value(value, pp, lineno, i, -2**(t.plen-1), 2**(t.plen-1))
            if lower is None:
                if value:
                    # custom precision for any input
                    if prec:
                        if prec > 0:
                            delta = abs(value) * 2**-prec
                        else:
                            delta = abs(value) * 2**-(t.vlen+prec)
                            
                    else:
                        # tests sfloat precision and defines lower upper thresholds
                        if (abs(value)> 1.0):
                            delta = 2 ** (int(math.floor(math.log(abs(value), 2))) - t.vlen + 1)                            
                        else:
                            delta = 2 ** (-t.vlen + fl_error)
                    test_value(value, mem_value, lineno, 0, value - delta, value + delta * 1.001)
                else:
                    test_value(value, mem_value, lineno, 0)
            else:
                test_value(value, mem_value, lineno, 0, lower, upper)
        elif isinstance(value, (_cfix, _sfix)):
            v = read_mem(1000 + addr + i)
            mem_value = v * (2 ** -sfix.f)
            if i == 0:
                print lineno, mem_value
            test_value(value, mem_value, lineno, i, lower, upper)
        else:
            # code for evaluation of functions with 2 parameters
            # specifically mult2sint, but code is generic and could be used
            # by any function that returns 2 parameters
            if type(value) is tuple:
                mem_value = read_mem(1000 + addr + i, value[0])
                if i == 0:
                   print lineno, mem_value
                if  mem_value<0 :
                    mem_value=mem_value+2**64
                test_value(value[0], mem_value, lineno, i, lower, upper)

                mem_value = read_mem(2000 + addr + i, value[1])
                if i == 0:
                    print lineno, mem_value
                if  mem_value<0 :
                    mem_value=mem_value+2**64
                test_value(value[1], mem_value, lineno, i, lower, upper)
            else:
                mem_value = read_mem(1000 + addr + i, value)
                if i == 0:
                   print lineno, mem_value
                test_value(value, mem_value, lineno, i, lower, upper)

def test_mem(value, address, lower=None, upper=None):
    """ Test value in a clear memory address """
    lineno = inspect.currentframe().f_back.f_lineno
    mem_value = read_mem(address, value)
    print address, mem_value
    test_value(value, mem_value, lineno, 0, lower, upper)

test1 = test

sys.path.insert(0, 'Test')
execfile('Programs/%s/%s.mpc' % (args[0],args[0]))
