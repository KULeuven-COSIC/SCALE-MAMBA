
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

import math
import operator

def format_trace(trace, prefix='  '):
    if trace is None:
        return '<omitted>'
    else:
        return ''.join('\n%sFile "%s", line %s, in %s\n%s  %s' %
                       (prefix,i[0],i[1],i[2],prefix,i[3][0].strip()) \
                           for i in reversed(trace))

def tuplify(x):
    if isinstance(x, (list, tuple)):
        return tuple(x)
    else:
        return (x,)

def untuplify(x):
    if len(x) == 1:
        return x[0]
    else:
        return x

def greater_than(a, b, bits):
    if isinstance(a, int) and isinstance(b, int):
        return a > b
    else:
        return a.greater_than(b, bits)

def pow2(a, bits):
    if isinstance(a, int):
        return 2**a
    else:
        return a.pow2(bits)

def mod2m(a, b, bits, signed):
    if isinstance(a, int):
        return a % 2**b
    else:
        return a.mod2m(b, bits, signed=signed)

def right_shift(a, b, bits):
    if isinstance(a, int):
        return a >> b
    else:
        return a.right_shift(b, bits)

def bit_decompose(a, bits):
    if isinstance(a, (int,long)):
        return [int((a >> i) & 1) for i in range(bits)]
    else:
        return a.bit_decompose(bits)

def bit_compose(bits):
    bits = list(bits)
    try:
        if bits:
            return bits[0].bit_compose(bits)
        else:
            return 0
    except AttributeError:
        return sum(b << i for i,b in enumerate(bits))

def series(a):
    sum = 0
    for i in a:
        yield sum
        sum += i
    yield sum

def if_else(cond, a, b):
    try:
        if isinstance(cond, (bool, int)):
            if cond:
                return a
            else:
                return b
        return cond.if_else(a, b)
    except:
        print cond, a, b
        raise

def cond_swap(cond, a, b):
    if isinstance(cond, (bool, int)):
        if cond:
            return a, b
        else:
            return b, a
    return cond.cond_swap(a, b)

def log2(x):
    #print 'Compute log2 of', x
    return int(math.ceil(math.log(x, 2)))

def tree_reduce(function, sequence):
    n = len(sequence)
    if n == 1:
        return sequence[0]
    else:
        reduced = [function(sequence[2*i], sequence[2*i+1]) for i in range(n/2)]
        return tree_reduce(function, reduced + sequence[n/2*2:])

def pow2(bits):
    powers = [b.if_else(2**2**i, 1) for i,b in enumerate(bits)]
    return tree_reduce(operator.mul, powers)

def irepeat(l, n):
    return reduce(operator.add, ([i] * n for i in l))

def int_len(x):
    return len(bin(x)) - 2

def reveal(x):
    if isinstance(x, str):
        return x
    try:
        return x.reveal()
    except AttributeError:
        pass
    try:
        return [reveal(y) for y in x]
    except TypeError:
        pass
    return x

def is_constant(x):
    return isinstance(x, (int, long, bool))

