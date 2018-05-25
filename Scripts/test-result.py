#!/usr/bin/python

import struct
import inspect
import gmpy2
import sys
import operator
import math
from optparse import OptionParser
from Compiler.core import *
from Compiler.core import _sfix, _cfix
from Compiler import types, util

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
R_inv = gmpy2.invert(R, p)
#print "R = %s" % hex(R)

program.P = p
program.bit_length = param
program.name = args[0]
sfloat.plen = types.sfloat.plen
sfloat.vlen = types.sfloat.vlen

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

def read_modp(address):
    #print address
    mem.seek(base + address * byte_length)
    #print hex(mem.tell())
    n = 0
    for i in range(byte_length / 8):
        n += struct.unpack('<Q', mem.read(8))[0] << (i * 64)
        #print hex(n)
    res = n * R_inv % p
    if res > p / 2:
        res -= p
    return res

def read_int(address):
    #print address
    mem.seek(int_base + address * 8)
    #print hex(mem.tell())
    return struct.unpack('<q', mem.read(8))[0]

def read_mem(address, value=None):
    if isinstance(value, MemValue):
        value = value.value
    if isinstance(value, (regint,bool)):
        return read_int(address)
    else:
        return read_modp(address)

error = 1e-3
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
           if abs(lower - mem_value) > error:
                fail = True
        else:
           if lower % p != mem_value % p:
                fail = True
        if fail:
            print "Failure in line %d, index %d: read %s, expected %s, %s" % \
                (lineno, index, mem_value, lower, type(value))
    if fail and options.stop_on_fail:
        exit(1)

def test(value, lower=None, upper=None, prec=None):
    lineno = inspect.currentframe().f_back.f_lineno
    addr = lineno
    if isinstance(value, (Vector, list)):
        addr *= 1000
        values = value
    else:
        if isinstance(value, (F, _sfix, _cfix)):
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
                    if prec:
                        if prec > 0:
                            delta = value * 2**-prec
                        else:
                            delta = value * 2**-(t.vlen+prec)
                    else:
                        delta = 2 ** (int(math.floor(math.log(abs(value), 2))) - t.vlen + 1)
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
