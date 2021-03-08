
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

from random import randint
import math

#import sys
#from Test.core import *
if '_Array' not in dir():
    from Compiler.types import *
    from Compiler.types import _secret
    from Compiler.library import *
    from Compiler.program import Program
    _Array = Array

SORT_BITS = []

def predefined_comparator(x, y):
    """ Assumes SORT_BITS is populated with the required sorting network bits """
    if predefined_comparator.sort_bits_iter is None:
        predefined_comparator.sort_bits_iter = iter(SORT_BITS)
    return next(predefined_comparator.sort_bits_iter)
predefined_comparator.sort_bits_iter = None

def list_comparator(x, y):
    """ Uses the first element in the list for comparison """
    return x[0] < y[0]

def normal_comparator(x, y):
    return x < y

def bitwise_list_comparator(x, y):
    """ Uses the first element in the list for comparison """
    return (1 - x[0]) * y[0]

def bitwise_comparator(x, y):
    b = (1 - x) * y
    return b

def cond_swap_bit(x,y, b):
    """ swap if b == 1 """
    if x is None:
        return y, None
    elif y is None:
        return x, None
    if isinstance(x, list):
        t = [(xi - yi) * b for xi,yi in zip(x, y)]
        return [xi - ti for xi,ti in zip(x, t)], \
            [yi + ti for yi,ti in zip(y, t)]
    else:
        t = (x - y) * b
        return x - t, y + t

def cond_swap(x,y, comp):
    if x is None:
        return y, None
    elif y is None:
        return x, None
    b = comp(x, y)
    return cond_swap_bit(x, y, 1 - b)

def odd_even_merge(a, comp):
    if len(a) & (len(a)-1) != 0:
        raise Exception('Length must be a power of 2')
    if len(a) == 1:
        return
    if len(a) == 2:
        a[0], a[1] = cond_swap(a[0], a[1], comp)
    else:
        even = a[::2]
        odd = a[1::2]
        odd_even_merge(even, comp)
        odd_even_merge(odd, comp)
        a[0] = even[0]
        for i in range(1, len(a) / 2):
            a[2*i-1], a[2*i] = cond_swap(odd[i-1], even[i], comp)
        a[-1] = odd[-1]

def odd_even_merge_sort(a, comp=bitwise_comparator):
    if len(a) == 1:
        return
    elif len(a) % 2 == 0:
        lower = a[:len(a)/2]
        upper = a[len(a)/2:]
        odd_even_merge_sort(lower, comp)
        odd_even_merge_sort(upper, comp)
        a[:] = lower + upper
        odd_even_merge(a, comp)
    else:
        raise CompilerError('Length of list must be power of two')

def merge(a, b, comp):
    """ General length merge (pads to power of 2) """
    while len(a) & (len(a)-1) != 0:
        a.append(None)
    while len(b) & (len(b)-1) != 0:
        b.append(None)
    if len(a) < len(b):
        a += [None] * (len(b) - len(a))
    elif len(b) < len(a):
        b += [None] * (len(b) - len(b))
    t = a + b
    odd_even_merge(t, comp)
    for i,v in enumerate(t[::]):
        if v is None:
            t.remove(None)
    return t

def sort(a, comp):
    """ Pads to power of 2, sorts, removes padding """
    length = len(a)
    while len(a) & (len(a)-1) != 0:
        a.append(None)
    odd_even_merge_sort(a, comp)
    del a[length:]

def recursive_merge(a, comp):
    """ Recursively merge a list of sorted lists (initially sorted by size) """
    if len(a) == 1:
        return
    # merge smallest two lists, place result in correct position, recurse
    t = merge(a[0], a[1], comp)
    del a[0]
    del a[0]
    added = False
    for i,c in enumerate(a):
        if len(c) >= len(t):
            a.insert(i, t)
            added = True
            break
    if not added:
        a.append(t)
    recursive_merge(a, comp)

def random_perm(n):
    """ Generate a random permutation of length n

    WARNING: randomness fixed at compile-time, this is NOT secure
    """
    a = range(n)
    for i in range(n-1, 0, -1):
        j = randint(0, i)
        t = a[i]
        a[i] = a[j]
        a[j] = t
    return a

def inverse(perm):
    inv = [None] * len(perm)
    for i, p in enumerate(perm):
        inv[p] = i
    return inv

def configure_waksman(perm):
    n = len(perm)
    if n == 2:
        return [(perm[0], perm[0])]
    I = [None] * (n/2)
    O = [None] * (n/2)
    p0 = [None] * (n/2)
    p1 = [None] * (n/2)
    inv_perm = [0] * n

    for i, p in enumerate(perm):
        inv_perm[p] = i

    while True:
        try:
            j = 2 * O.index(None)
        except ValueError:
            break
        #print 'j =', j
        O[j/2] = 0
        via = 0
        j0 = j
        while True:
            #print '    I[%d] = %d' % (inv_perm[j]/2, ((inv_perm[j] % 2) + via) % 2)

            i = inv_perm[j]
            #print '    p0[%d] = %d' % (inv_perm[j]/2, j/2)
            p0[i/2] = j/2

            I[i/2] = i % 2
            O[j/2] = j % 2
            #print '    O[%d] = %d' % (j/2, j % 2)
            if i % 2 == 1:
                i -= 1
            else:
                i += 1
            #i, via = set_swapper(I, j, via, inv_perm)

            #print '    O[%d] = %d' % (perm[i]/2, ((perm[i] % 2) + via ) % 2)
            j = perm[i]
            #O[j/2] = j % 2
            if j % 2 == 1:
                j -= 1
            else:
                j += 1
            #j, via = set_swapper(O, i, via, perm)
            #print '    p1[%d] = %d' % (i/2, perm[i]/2)
            p1[i/2] = perm[i]/2

            #print '    i = %d, j =  %d' %(i,j)
            if j == j0:
                break
        if None not in p0 and None not in p1:
            break

    assert sorted(p0) == range(n/2)
    assert sorted(p1) == range(n/2)
    p0_config = configure_waksman(p0)
    p1_config = configure_waksman(p1)
    return [I + O] + [a+b for a,b in zip(p0_config, p1_config)]

def waksman(a, config, depth=0, start=0, reverse=False):
    """ config is a list of log_2(n) configuration lists for the sub-networks """
    n = len(a)
    if n == 2:
        a[0], a[1] = cond_swap_bit(a[0], a[1], config[depth][start])
        return

    a0 = [0] * (n/2)
    a1 = [0] * (n/2)
    for i in range(n/2):
        if reverse:
            a0[i], a1[i] = cond_swap_bit(a[2*i], a[2*i+1], config[depth][i + n/2 + start])
        else:
            a0[i], a1[i] = cond_swap_bit(a[2*i], a[2*i+1], config[depth][i + start])

    waksman(a0, config, depth+1, start, reverse)
    waksman(a1, config, depth+1, start + n/2, reverse)

    for i in range(n/2):
        if reverse:
            a[2*i], a[2*i+1] = cond_swap_bit(a0[i], a1[i], config[depth][i + start])
        else:
            a[2*i], a[2*i+1] = cond_swap_bit(a0[i], a1[i], config[depth][i + n/2 + start])


WAKSMAN_FUNCTIONS = {}
def iter_waksman(a, config, reverse=False):
    """ Iterative Waksman algorithm, compilable for large inputs. Input
    must be an Array. """
    n = len(a)
    #if not isinstance(a, Array):
    #    raise CompilerError('Input must be an Array')

    depth = MemValue(0)
    nblocks = MemValue(1)
    size = MemValue(0)
    a2 = Array(n, a[0].reg_type)
    #config_array = Array(n, a[0].reg_type)
    #reverse = (int(reverse))

    def create_round_fn(n, reg_type):
        # TODO: include reverse-ness
        if (n, reg_type, inwards) in WAKSMAN_FUNCTIONS:
            return WAKSMAN_FUNCTIONS[(n, reg_type, inwards)]
        
        def do_round(size, config_address, a_address, a2_address, inwards):
            A = Array(n, reg_type, a_address)
            A2 = Array(n, reg_type, a2_address)
            C = Array(n, reg_type, config_address)
            inwards = MemValue(inwards)
            outwards = MemValue(1 - inwards)
            
            sizeval = size
            #for k in range(n/2):
            @for_range_parallel(200, n/2)
            def f(k):
                j = cint(k) % sizeval
                i = (cint(k) - j)/sizeval
                base = 2*i*sizeval

                in1, in2 = (base+j+j*inwards), (base+j+j*inwards+1*inwards+sizeval*outwards)
                out1, out2 = (base+j+j*outwards), (base+j+j*outwards+1*outwards+sizeval*inwards)
                
                if inwards:
                    if reverse:
                        c = C[base + j + sizeval]
                    else:
                        c = C[base + j]
                else:
                    if reverse:
                        c = C[base + j]
                    else:
                        c = C[base + j + sizeval]
                A2[out1], A2[out2] = cond_swap_bit(A[in1], A[in2], c)

        fn = function_block(do_round)
        WAKSMAN_FUNCTIONS[(n, reg_type, inwards)] = fn
        return fn
    
    do_round = create_round_fn(n, a[0].reg_type)

    logn = int(math.log(n,2))

    # going into middle of network
    @for_range(logn)
    def f(i):
        size.write(n/(2*nblocks))
        conf_address = MemValue(config.address + depth.read()*n)
        do_round(size, conf_address, a.address, a2.address, cint(1))

        for i in range(n):
            a[i] = a2[i]

        nblocks.write(nblocks*2)
        depth.write(depth+1)

    nblocks.write(nblocks/4)
    depth.write(depth-2)

    # and back out
    @for_range(logn-1)
    def f(i):
        size.write(n/(2*nblocks))
        conf_address = MemValue(config.address + depth.read()*n)
        do_round(size, conf_address, a.address, a2.address, cint(0))

        for i in range(n):
            a[i] = a2[i]

        nblocks.write(nblocks/2)
        depth.write(depth-1)

    ## going into middle of network
    #while nblocks < n:
    #    #for i in range(n):
    #    #    config_array[i] = config[depth][i].read()
#
    #    size.write(n/(2*nblocks))
    #    conf_address = config.address + depth*n
    #    do_round_in(size, conf_address, a.address, a2.address)
#
    #    for i in range(n):
    #        a[i] = a2[i]
#
    #    nblocks *= 2
    #    depth += 1
    #
    #nblocks /= 4
    #depth -= 2
    ## and back out
    #while nblocks > 0:
    #    #for i in range(n):
    #    #    config_array[i] = config[depth][i].read()
#
    #    size.write(n/(2*nblocks))
    #    conf_address = config.address + depth*n
    #    do_round_out(size, conf_address, a.address, a2.address)
#
    #    for i in range(n):
    #        a[i] = a2[i]
#
    #    nblocks /= 2
    #    depth -= 1

def rec_shuffle(x, config=None, value_type=sgf2n, reverse=False):
    n = len(x)
    if n & (n-1) != 0:
        raise CompilerError('shuffle requires n a power of 2')
    if config is None:
        config = configure_waksman(random_perm(n))
        for i,c in enumerate(config):
            config[i] = [value_type.bit_type(b) for b in c]
    waksman(x, config, reverse=reverse)
    waksman(x, config, reverse=reverse)


def config_shuffle(n, value_type):
    """ Compute config for oblivious shuffling.
    
    TODO: input config bits. Take mod 2 for active sec. """
    perm = random_perm(n)
    if n & (n-1) != 0:
        # pad permutation to power of 2
        m = 2**int(math.ceil(math.log(n, 2)))
        perm += range(n, m)
    config_bits = configure_waksman(perm)
    # 2-D array
    config = Array(len(config_bits) * len(perm), value_type.reg_type)
    for i,c in enumerate(config_bits):
        for j,b in enumerate(c):
            config[i * len(perm) + j] = b
    return config

def shuffle(x, config=None, value_type=sgf2n, reverse=False):
    """ Simulate secure shuffling with Waksman network for 2 players.
    WARNING: This is not a properly secure implementation but has roughly the right complexity.

    Returns the network switching config so it may be re-used later.  """
    n = len(x)
    m = 2**int(math.ceil(math.log(n, 2)))
    if config is None:
        config = config_shuffle(n, value_type)

    if isinstance(x, list):
        if isinstance(x[0], list):
            length = len(x[0])
            for i in range(length):
                xi = Array(m, value_type.reg_type)
                for j in range(n):
                    xi[j] = x[j][i]
                for j in range(n, m):
                    xi[j] = value_type(0)
                iter_waksman(xi, config, reverse=reverse)
                iter_waksman(xi, config, reverse=reverse)
        else:
            xa = Array(m, value_type.reg_type)
            for i in range(n):
                xa[i] = x[i]
            for i in range(n, m):
                xa[i] = value_type(0)
            iter_waksman(xa, config, reverse=reverse)
            iter_waksman(xa, config, reverse=reverse)
    elif isinstance(x, Array):
        if len(x) != m and config is None:
            raise CompilerError('Non-power of 2 Array input not yet supported')
        iter_waksman(x, config, reverse=reverse)
        iter_waksman(x, config, reverse=reverse)
    else:
        raise CompilerError('Invalid type for shuffle:', type(x))

    return config

def shuffle_entries(x, entry_cls, config=None, value_type=sgf2n, reverse=False, perm_size=None):
    """ Shuffle a list of ORAM entries.

        Randomly permutes the first "perm_size" entries, leaving the rest (empty
        entry padding) in the same position. """
    n = len(x)
    l = len(x[0])
    if n & (n-1) != 0:
        raise CompilerError('Entries must be padded to power of two length.')
    if perm_size is None:
        perm_size = n

    xarrays = [Array(n, value_type.reg_type) for i in range(l)]
    for i in range(n):
        for j,value in enumerate(x[i]):
            if isinstance(value, MemValue):
                xarrays[j][i] = value.read()
            else:
                xarrays[j][i] = value

    if config is None:
        config = config_shuffle(perm_size, value_type)
    for xi in xarrays:
        shuffle(xi, config, value_type, reverse)
    for i in range(n):
        x[i] = entry_cls(xarrays[j][i] for j in range(l))
    return config


def sort_zeroes(bits, x, n_ones, value_type):
    """ Return Array of values in "x" where the corresponding bit in "bits" is
    a 0.

    The total number of zeroes in "bits" must be known.
    "bits" and "x" must be Arrays. """
    config = config_shuffle(len(x), value_type)
    shuffle(bits, config=config, value_type=value_type)
    shuffle(x, config=config, value_type=value_type)
    result = Array(n_ones, value_type.reg_type)

    sz = MemValue(0)
    last_x = MemValue(value_type(0))
    #for i,b in enumerate(bits):
        #if_then(b.reveal() == 0)
        #result[sz.read()] = x[i]
        #sz += 1
        #end_if()
    @for_range(len(bits))
    def f(i):
        found = (bits[i].reveal() == 0)
        szval = sz.read()
        result[szval] = last_x + (x[i] - last_x) * found
        sz.write(sz + found)
        last_x.write(result[szval])
    return result