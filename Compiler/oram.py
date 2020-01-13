from __future__ import print_function
from __future__ import division
from builtins import next
from builtins import map
from builtins import str
from builtins import zip
from builtins import range
from past.utils import old_div
from builtins import object
import random
import math
import collections
import itertools
import operator
import sys
from functools import reduce

if 'Emulation' in sys.path:
    print('emulation mode')
    from Emulation.library import *
    from Emulation.types import *
    from Emulation.types import _secret
    from Emulation.program import Program
    from Emulation import floatingpoint,comparison,permutation
else:
    print('compilation mode')
    from Compiler.types import *
    from Compiler.types import _secret
    from Compiler.library import *
    from Compiler.program import Program
    from Compiler import floatingpoint,comparison,permutation

from Compiler.util import *

print_access = False
sint_bit_length = 6
max_demux_bits = 3
debug = False
use_binary_search = False
n_parallel = 1024
n_threads = None
detailed_timing = False
optimal_threshold = None
n_threads_for_tree = None
debug_online = False
crash_on_overflow = False
use_insecure_randomness = False
debug_ram_size = False

def maybe_start_timer(n):
    if detailed_timing:
        start_timer(n)

def maybe_stop_timer(n):
    if detailed_timing:
        stop_timer(n)

class Block(object):
    def __init__(self, value, lengths):
        self.value = self.value_type.hard_conv(value)
        self.lengths = tuplify(lengths)
    def get_slice(self):
        res = []
        for length,start in  zip(self.lengths, series(self.lengths)):
            res.append(util.bit_compose((self.bits[start:start+length])))
        return res
    def __repr__(self):
        return '<' + str(self.value) + '>'

class intBlock(Block):
    """ Bit slicing for modp. """
    value_type = sint
    def __init__(self, value, start, lengths, entries_per_block):
        Block.__init__(self, value, lengths)
        length = sum(self.lengths)
        self.n_bits = length * entries_per_block
        self.start = self.value_type.hard_conv(start * length)
        self.lower, self.shift = \
            floatingpoint.Trunc(self.value, self.n_bits, self.start, \
                                    Program.prog.security, True)
        trunc = old_div((self.value - self.lower), self.shift)
        self.slice = trunc.mod2m(length, self.n_bits, False)
        self.upper = (trunc - self.slice) * self.shift
    def get_slice(self):
        total_length = sum(self.lengths)
        if len(self.lengths) == 1:
            self.bits = self.slice.bit_decompose(total_length)
            return super(intBlock, self).get_slice()
        else:
            res = []
            remainder = self.slice
            for length,start in zip(self.lengths[:-1],series(self.lengths)):
                res.append(remainder.mod2m(length, total_length - start, False))
                remainder -= res[-1]
                remainder /= floatingpoint.two_power(length)
            res.append(remainder)
            return res
    def set_slice(self, value):
        value = sum(v << start for v,start in zip(value, series(self.lengths)))
        self.value = self.upper + self.lower + value * self.shift
        return self

class gf2nBlock(Block):
    """ Bit slicing for GF2n. """
    value_type = sgf2n
    def __init__(self, value, start, lengths, entries_per_block):
        Block.__init__(self, value, lengths)
        length = sum(self.lengths)
        Program.prog.curr_tape.\
            start_new_basicblock(name='gf2n-block-init-%d' % entries_per_block)
        used_bits = entries_per_block * length
        if entries_per_block == 2:
            value_bits = bit_decompose(self.value, used_bits)
            prod_bits = [start * bit for bit in value_bits]
            anti_bits = [v - p for v,p in zip(value_bits,prod_bits)]
            self.lower = sum(bit << i for i,bit in enumerate(prod_bits[:length]))
            self.bits = list(map(operator.add, anti_bits[:length], prod_bits[length:])) + \
                anti_bits[length:]
            self.adjust = if_else(start, 1 << length, cgf2n(1))
        elif entries_per_block < 4:
            value_bits = bit_decompose(self.value, used_bits)
            l = log2(entries_per_block)
            start_bits = bit_decompose(start, l)
            choice_bits = demux(start_bits)
            inv_bits = [1 - bit for bit in floatingpoint.PreOR(choice_bits, None)]
            mask_bits = sum(([x] * length for x in inv_bits), [])
            lower_bits = list(map(operator.mul, value_bits, mask_bits))
            self.lower = sum(bit << i for i,bit in enumerate(lower_bits))
            self.bits = [sum(map(operator.mul, choice_bits, value_bits[i::length])) \
                                 for i in range(length)]
            self.adjust = sum(bit << (i * length) \
                                  for i,bit in enumerate(choice_bits))
        else:
            value_bits = bit_decompose(self.value, used_bits)
            l = log2(entries_per_block)
            start_bits = bit_decompose(start, l)
            powers = [2**(2**i) for i in range(l)]
            selected = [power * bit + (1 - bit) \
                            for bit,power in zip(start_bits,powers)]
            power_start = floatingpoint.KOpL(operator.mul, selected)
            bits = bit_decompose(power_start, entries_per_block)
            adjust = sum(bit << (i * length) for i,bit in enumerate(bits))
            pre_bits = floatingpoint.PreOpL(lambda x,y,z=None: x + y, bits)
            inv_bits = [1 - bit for bit in pre_bits]
            mask_bits = sum(([x] * length for x in inv_bits), [])
            lower_bits = list(map(operator.mul, value_bits, mask_bits))
            masked = self.value - sum(bit << i for i,bit in enumerate(lower_bits))
            self.lower = sum(bit << i for i,bit in enumerate(lower_bits))
            self.bits = (old_div(masked, adjust)).bit_decompose(used_bits)
            self.adjust = adjust
        Program.prog.curr_tape.\
            start_new_basicblock(name='gf2n-block-init-end-%d' % entries_per_block)
    def set_slice(self, value):
        upper_bits = self.bits[sum(self.lengths):]
        upper = (sum(b << i for i,b in enumerate(upper_bits)) * \
                     self.adjust) << sum(self.lengths)
        value = sum(v << start for v,start in zip(value, series(self.lengths)))
        self.value = self.lower + value * self.adjust + upper
        return self

block_types = { sint: intBlock,
                sgf2n: gf2nBlock,
}

def get_block(x, y, *args):
    for t in block_types:
        if isinstance(x, t):
            return block_types[t](x, y, *args)
        elif isinstance(y, t):
            return block_types[t](x, y, *args)
    raise CompilerError('appropiate block type not found')

def get_bit(x, index, bit_length):
    if isinstance(x, sgf2n):
        bits = x.bit_decompose(bit_length)
        choice_bits = cgf2n(1 << index).bit_decompose(bit_length)
        return sum(map(operator.mul, bits, choice_bits))
    else:
        return get_block(x, index, 1, bit_length).get_slice()[0]

def demux(x):
    """ Demuxing like in the Galois paper. """
    # res = Array(2**len(x), x[0].reg_type)
    # for i,v in enumerate(demux_list(x)):
    #     res[i] = v
    # return res

    if 2**len(x) <= n_parallel:
        return demux_list(x)
    else:
        return demux_array(x)

def demux_list(x):
    n = len(x)
    if n == 0:
        return [1]
    elif n == 1:
        return [1 - x[0], x[0]]
    a = demux_list(x[:old_div(n,2)])
    b = demux_list(x[old_div(n,2):])
    n_a = len(a)
    a *= len(b)
    b = reduce(operator.add, ([i] * n_a for i in b))
    res = list(map(operator.mul, a, b))
    return res

def demux_array(x):
    n = len(x)
    res = Array(2**n, type(x[0]))
    if n == 1:
        res[0] = 1 - x[0]
        res[1] = x[0]
    else:
        a = Array(2**(old_div(n,2)), type(x[0]))
        a.assign(demux(x[:old_div(n,2)]))
        b = Array(2**(n-old_div(n,2)), type(x[0]))
        b.assign(demux(x[old_div(n,2):]))
        @for_range_multithread(get_n_threads(len(res)), \
                                   max(1, old_div(n_parallel, len(b))), len(a))
        def f(i):
            @for_range_parallel(n_parallel, len(b))
            def f(j):
                res[j * len(a) + i] = a[i] * b[j]
    return res

def get_first_one(x):
    prefix_list = [0] + floatingpoint.PreOR(x, Program.prog.security)
    return [prefix_list[i+1] - prefix_list[i] for i in range(len(x))]

class Value(object):
    def __init__(self, value=None, empty=None):
        if value is None:
            self.empty = 1
            self.value = 0
        else:
            try:
                self.value = next(value)
                self.empty = next(value)
            except TypeError:
                self.empty = 0 if empty is None else empty
                self.value = value
    def __iter__(self):
        yield self.value
        yield self.empty
    def __add__(self, other):
        return Value(self.value + other.value, self.empty + other.empty)
    def __sub__(self, other):
        return Value(self.value - other.value, self.empty - other.empty)
    def __xor__(self, other):
        return Value(self.value ^ other.value, self.empty ^ other.empty)
    def __mul__(self, other):
        return Value(other * self.value, other * self.empty)
    __rmul__ = __mul__
    def equal(self, other, length=None):
        if isinstance(other, (int, int)) and isinstance(self.value, (int, int)):
            return (1 - self.empty) * (other == self.value)
        return (1 - self.empty) * self.value.equal(other, length)
    def reveal(self):
        return Value(reveal(self.value), reveal(self.empty))
    def output(self):
        # @if_e(self.empty)
        # def f():
        #     print_str('<>')
        # @else_
        # def f():
        print_str('<%s:%s>', self.empty, self.value)
    def __index__(self):
        return int(self.value)
    def __repr__(self):
        try:
            value = self.empty
            while True:
                if value in (1, 1):
                    return '<>'
                if value in (0, 0):
                    return '<%s>' % str(self.value)
                value = value.value
        except:
            pass
        return '<%s:%s>' % (str(self.value), str(self.empty))

class ValueTuple(tuple):
    """ Works like a vector. """
    def skip(self, skip):
        return ValueTuple(self[skip:])
    def __add__(self, other):
        return ValueTuple(i + j for i,j in zip(self, other))
    def __sub__(self, other):
        return ValueTuple(i - j for i,j in zip(self, other))
    def __xor__(self, other):
        return ValueTuple(i ^ j for i,j in zip(self, other))
    def __mul__(self, other):
        return ValueTuple(other * i for i in self)
    __rmul__ = __mul__
    __rxor__ = __xor__
    def output(self):
        print_str('(' + ', '.join('%s' for i in range(len(self))) + ')', *self)

class Entry(object):
    """ An (O)RAM entry with empty bit, index, and value. """
    @staticmethod
    def get_empty(value_type, value_length, apply_type=True):
        t = value_type if apply_type else lambda x: x
        bt = value_type.bit_type if apply_type else lambda x: x
        return Entry(t(0), tuple(t(0) for i in range(value_length)), bt(True), t)
    def __init__(self, v, x=None, empty=None, value_type=None):
        self.created_non_empty = False
        if x is None:
            v = iter(v)
            self.is_empty = next(v)
            self.v = next(v)
            self.x = ValueTuple(v)
        else:
            if empty is None:
                self.created_non_empty = True
                empty = value_type.bit_type(False)
            self.is_empty = empty
            self.v = v
            if not isinstance(x, (tuple, list)):
                x = (x,)
            self.x = ValueTuple(x)
    def empty(self):
        return self.is_empty
    def types(self):
        return tuple(type(i) for i in self)
    def values(self):
        yield self.is_empty
        yield self.v
        for i in self.x:
            yield i
    def __iter__(self):
        yield self.is_empty
        yield self.v
        for i in self.x:
            yield i
    def __len__(self):
        return 2 + len(self.x)
    def __repr__(self):
        return '{empty=%s}' % self.is_empty if self.is_empty \
            else '{%s: %s}' % (self.v, self.x)
    def __add__(self, other):
        try:
            return Entry(i + j for i,j in zip(self, other))
        except:
            print(self, other)
            raise
    def __sub__(self, other):
        return Entry(i - j for i,j in zip(self, other))
    def __xor__(self, other):
        return Entry(i ^ j for i,j in zip(self, other))
    def __mul__(self, other):
        try:
            return Entry(other * i for i in self)
        except:
            print(self, other)
            raise
    __rmul__ = __mul__
    def reveal(self):
        return Entry(x.reveal() for x in self)
    def output(self):
        # @if_e(self.is_empty)
        # def f():
        #     print_str('{empty=%s}', self.is_empty)
        # @else_
        # def f():
        #     print_str('{%s: %s}', self.v, self.x)\
        print_str('{%s: %s,empty=%s}', self.v, self.x, self.is_empty)

class RefRAM(object):
    """ RAM reference. """
    def __init__(self, index, oram):
        if debug_ram_size:
            @if_(index >= oram.n_buckets())
            def f():
                print_ln('invalid bucket index %s for %s buckets', \
                             index, oram.n_buckets())
                crash()
        self.size = oram.bucket_size
        self.entry_type = oram.entry_type
        self.l = [t.dynamic_array(self.size, t, array.address + \
                               index * oram.bucket_size) \
                      for t,array in zip(self.entry_type,oram.ram.l)]
        self.index = index
    def init_mem(self, empty_entry):
        print('init ram')
        for a,value in zip(self.l, list(empty_entry.values())):
            a.assign_all(value)
    def get_empty_bits(self):
        return self.l[0]
    def get_indices(self):
        return self.l[1]
    def get_values(self, skip=0):
        return [ValueTuple(x) for x in zip(*self.l[2+skip:])]
    def get_value(self, index, skip=0):
        return ValueTuple(a[index] for a in self.l[2+skip:])
    def get_value_length(self):
        return len(self.l) - 2
    def get_value_arrays(self):
        return self.l[2:]
    def get_value_array(self, index):
        return [Value(self.l[2+index][i], self.l[0][i]) for i in range(self.size)]
    def __getitem__(self, index):
        if print_access:
            print('get', id(self), index)
        return Entry(a[index] for a in self.l)
    def __setitem__(self, index, value):
        if print_access:
            print('set', id(self), index)
        if not isinstance(value, Entry):
            raise Exception('entries only please: %s' % str(value))
        for i,(a,v) in enumerate(zip(self.l, list(value.values()))):
            a[index] = v
    def __len__(self):
        return self.size
    def has_empty_entry(self):
        return 1 - tree_reduce(operator.mul, [1 - bit for bit in self.get_empty_bits()])
    def is_empty(self):
        return tree_reduce(operator.mul, list(self.get_empty_bits()))
    def reveal(self):
        Program.prog.curr_tape.start_new_basicblock()
        res = RAM(self.size, [t.clear_type for t in self.entry_type], self.index)
        for i,a in enumerate(self.l):
            for j,x in enumerate(a):
                res.l[i][j] = x.reveal()
        Program.prog.curr_tape.start_new_basicblock()
        return res
    def output(self):
        print_ln('%s', [x.reveal() for x in self])
    def print_reg(self):
        print_ln('listing of RAM at index %s', self.index)
        Program.prog.curr_tape.start_new_basicblock()
        for i,array in enumerate(self.l):
            for j,reg in enumerate(array):
                print_str('%s:%s ', j, reg)
            print_ln()
        Program.prog.curr_tape.start_new_basicblock()
    def __repr__(self):
        return repr(self.l)

class RAM(RefRAM):
    """ List of entries in memory. """
    def __init__(self, size, entry_type, index=0):
        #print_reg_char4(cint(0), 'r in')
        self.size = size
        self.entry_type = entry_type
        self.l = [t.dynamic_array(self.size, t) for t in entry_type]
        self.index = index

class AbstractORAM(object):
    """ Implements reading and writing using read_and_remove and add. """
    def read(self, index):
        return self._read(self.value_type.hard_conv(index))
    def write(self, index, value):
        new_value = [self.value_type.hard_conv(v) \
                         for v in (value if isinstance(value, (tuple, list)) \
                                       else (value,))]
        return self._write(self.value_type.hard_conv(index), *new_value)
    def access(self, index, new_value, write, new_empty=False):
        return self._access(self.value_type.hard_conv(index),
            self.value_type.bit_type.hard_conv(write),
            self.value_type.bit_type.hard_conv(new_empty),
            *[self.value_type.hard_conv(v) for v in tuplify(new_value)])
    def read_and_maybe_remove(self, index):
        return self.read_and_remove(self.value_type.hard_conv(index)), \
            self.state.read()
    @method_block
    def _read(self, index):
        return self.access(index, (self.value_type(0),) * self.value_length, \
                               False)
    @method_block
    def _write(self, index, *value):
        self.access(index, value, True)
    @method_block
    def _access(self, index, write, new_empty, *new_value):
        Program.prog.curr_tape.\
            start_new_basicblock(name='abstract-access-remove-%d' % self.size)
        index = MemValue(self.value_type.hard_conv(index))
        read_value, read_empty = self.read_and_remove(index)
        if len(read_value) != self.value_length:
            raise Exception('read_and_remove() of %s returns wrong length of ' \
                                'read value: %d, should be %d' % \
                                (type(self), len(read_value), \
                                     self.value_length))
        Program.prog.curr_tape.\
            start_new_basicblock(name='abstract-access-add-%d' % self.size)
        new_value = ValueTuple(new_value) \
            if isinstance(new_value, (tuple, list)) \
            else ValueTuple((new_value,))
        if len(new_value) != self.value_length:
            raise Exception('wrong length of new value')
        value = tuple(MemValue(i) for i in if_else(write, new_value, read_value))
        empty = self.value_type.bit_type.hard_conv(new_empty)
        self.add(Entry(index, value, if_else(write, empty, read_empty), \
                           value_type=self.value_type))
        return read_value, read_empty
    @method_block
    def delete(self, index, for_real=True):
        self.access(index, (self.value_type(0),) * self.value_length, \
                        for_real, True)
    def __getitem__(self, index):
        res, empty = self.read(index)
        if len(res) == 1:
            res = res[0]
        return res
    __setitem__ = write

class EmptyException(Exception):
    pass

class RefTrivialORAM(object):
    """ Trivial ORAM reference. """
    contiguous = False
    def empty_entry(self, apply_type=True):
        return Entry.get_empty(self.value_type, self.value_length, apply_type)
    def __init__(self, index, oram):
        self.ram = RefRAM(index, oram)
        self.index_size = oram.index_size
        self.value_type, self.value_length = oram.internal_value_type()
        self.size = oram.bucket_size
    def init_mem(self):
        print('init trivial oram')
        self.ram.init_mem(self.empty_entry())
    def search(self, read_index):
        if use_binary_search and self.value_type == sgf2n:
            return self.binary_search(read_index)
        else:
            indices = self.ram.get_indices()
            empty_bits = self.ram.get_empty_bits()
            parallel = 1024
            if comparison.const_rounds:
                parallel /= 4
            if self.size >= 128:
                #n_threads = 8 if self.size >= 8 * parallel else 1
                found = Array(self.size, self.value_type)
                read_index = MemValue(read_index)
                @for_range_multithread(n_threads, parallel, self.size)
                def f(j):
                    found[j] = indices[j].equal(read_index, self.index_size) * \
                        (1 - empty_bits[j])
            else:
                found = [indices[j].equal(read_index, self.index_size) * \
                    (1 - empty_bits[j]) for j in range(self.size)]
            # at most one 1 in found
            empty = 1 - sum(found)
            return found, empty
    def read_and_remove(self, read_index, skip=0):
        empty_entry = self.empty_entry(False)
        self.last_index = read_index
        found, empty = self.search(read_index)
        entries = [entry for entry in self.ram]
        prod_entries = list(map(operator.mul, found, entries))
        read_value = sum((entry.x.skip(skip) for entry in prod_entries), \
                             empty * empty_entry.x.skip(skip))
        for i,(entry, prod_entry) in enumerate(zip(entries, prod_entries)):
            self.ram[i] = entry - prod_entry + found[i] * empty_entry
        self.check(index=read_index, op='rar')
        return read_value, empty
    def read_and_maybe_remove(self, index):
        return self.read_and_remove(index), 0
    def read_and_remove_by_public(self, index):
        empty_entry = self.empty_entry(False)
        entries = [entry for entry in self.ram]
        prod_entries = list(map(operator.mul, index, entries))
        read_entry = reduce(operator.add, prod_entries)
        for i,(entry, prod_entry) in enumerate(zip(entries, prod_entries)):
            self.ram[i] = entry - prod_entry + index[i] * empty_entry
        return read_entry
    @method_block
    def _read(self, index):
        found, empty = self.search(index)
        read_value = sum(list(map(operator.mul, found, self.ram.get_values())), \
                             empty * self.empty_entry(False).x)
        return read_value, empty
    @method_block
    def _access(self, index, write, new_empty, *new_value):
        empty_entry = self.empty_entry(False)
        found, not_found = self.search(index)
        add_here = self.find_first_empty()
        entries = [entry for entry in self.ram]
        prod_values = list(map(operator.mul, found, \
                              (entry.x for entry in entries)))
        read_value = sum(prod_values, not_found * empty_entry.x)
        new_value = ValueTuple(new_value) \
            if isinstance(new_value, (tuple, list)) \
            else ValueTuple((new_value,))
        for i,(entry,prod_value) in enumerate(zip(entries, prod_values)):
            access_here = found[i] + not_found * add_here[i]
            delta_entry = Entry(access_here * (index - entry.v), \
                                    access_here * (new_value - entry.x), \
                                    found[i] - \
                                    if_else(new_empty, 0, access_here))
            self.ram[i] = entry + write * delta_entry
        return read_value, not_found
    def check(self, found=None, index=None, new_entry=None, op=''):
        if debug:
            if found is None:
                found = set()
            for i,entry in enumerate(self.ram):
                if not entry.empty():
                    if entry.v in found:
                        raise Exception('found double %s in %s' % (str(entry.v), str(self.ram.l)))
                    found.add(entry.v)
            if index is not None:
                for i,entry in enumerate(self.ram):
                    if not entry.empty() and index == entry.v:
                        raise Exception('not removed %s in %s' % \
                                            (str(index), str(self.ram.l)))
        if debug_online or debug:
            #cint(0).print_reg(op)
            entries = self.ram.reveal()
            if index is not None:
                index = index.reveal()
            if new_entry is not None:
                new_entry = Entry(x.reveal() for x in new_entry)
                n_found = MemValue(0)
            @for_range(self.size)
            def f(i):
                entry = entries[i]
                @if_(entry.empty() != 1)
                def f():
                    @if_e(entry.empty() == 0)
                    def f():
                        if index is not None:
                            @if_(entry.v == index)
                            def f():
                                entries.print_reg()
                                cint(0).print_reg(op)
                                cint(i).print_reg('trre')
                                entry.empty().print_reg('empt')
                                entry.v.print_reg('v')
                                index.print_reg('idx')
                                crash()
                        if new_entry is not None:
                            @if_(regint(1 - new_entry.empty()))
                            def f():
                                comps = Entry(x == y for x,y in \
                                                  zip(entry,new_entry))
                                @if_(reduce(operator.mul, comps))
                                def f():
                                    n_found.iadd(1)
                    @else_
                    def f():
                        entries.print_reg()
                        cint(0).print_reg(op)
                        cint(i).print_reg('trem')
                        entry.empty().print_reg('empt')
                        crash()
            if new_entry is not None:
                @if_((n_found != 1) * (1 - new_entry.empty()))
                def f():
                    entries.print_reg()
                    cint(0).print_reg(op)
                    cint(0).print_reg('trad')
                    cint(n_found).print_reg('n')
                    new_entry.v.print_reg('v')
                    for i,x in enumerate(new_entry.x):
                        x.print_reg('x%d' % i)
                    crash()

    def binary_search(self, index):
        if (self.size & (self.size-1)) != 0:
            n = 2**(int(math.log(self.size,2)) + 1)
        else:
            n = self.size

        indices = [i for i in self.ram.get_indices()]
        if self.contiguous and n <= 256:
            logn = int(math.log(n,2))
            expand = 5
            for i,x in enumerate(indices):
                indices[i] = sum(y << (j * expand) for j,y in \
                                     enumerate(x.bit_decompose(logn)))
            index = sum(y << (j * expand) for j,y in \
                            enumerate(index.bit_decompose(logn)))
        else:
            expand = 1

        # now search for zero
        logn = int(round(math.log(n,2)))
        mult_tree = [1] * 2*n
        bit_prods = [None] * 2*n
        for i in range(n-1, n-1 + self.size):
            mult_tree[i] = indices[i - n + 1] - index
        for i in range(n-2, -1, -1):
            mult_tree[i] = mult_tree[2*i+1] * mult_tree[2*i+2]

        b = 1 - mult_tree[0].equal(0, 40, expand)

        bit_prods[0] = 1 - b

        for j in range(1,logn+1):
            M = 0
            for k in range(2**(j)):
                t = k + 2**(j) - 1
                if k % 2 == 0:
                    M += bit_prods[old_div((t-1),2)] * mult_tree[t]

            b = 1 - M.equal(0, 40, expand)

            for k in range(2**j):
                t = k + 2**j - 1
                if k % 2 == 0:
                    v = bit_prods[old_div((t-1),2)] * b
                    bit_prods[t] = bit_prods[old_div((t-1),2)] - v
                else:
                    bit_prods[t] = v
        return bit_prods[n-1:n-1+self.size], 1 - bit_prods[0]

    def find_first_empty(self):
        prefix_empty = [0] + \
            floatingpoint.PreOR([empty for empty in self.ram.get_empty_bits()], \
                                                     Program.prog.security)
        return [prefix_empty[i+1] - prefix_empty[i] \
                    for i in range(len(self.ram))]
    def add(self, new_entry, state=None):
        # if self.last_index != new_entry.v:
        #     raise Exception('index mismatch: %s / %s' %
        #                     (str(self.last_index), str(new_entry.v)))
        add_here = self.find_first_empty()
        for i,entry in enumerate(self.ram):
            self.ram[i] = if_else(add_here[i], new_entry, entry)
        if crash_on_overflow:
            @if_(or_op(sum(add_here), new_entry.is_empty).reveal() == 0)
            def f():
                self.output()
                print_ln('New entry: %s:%s (empty: %s)', new_entry.v.reveal(),
                        new_entry.x[0].reveal(), new_entry.is_empty.reveal())
                print_ln('Bucket overflow')
                crash()
        if debug and not sum(add_here) and not new_entry.empty():
            print(self.empty_entry())
            raise Exception('no space for %s in %s' % (str(new_entry), str(self)))
        self.check(new_entry=new_entry, op='add')
    def pop(self):
        self.last_index = None
        empty_entry = self.empty_entry(False)
        prefix_empty = [0] + \
            floatingpoint.PreOR([1 - empty for empty in self.ram.get_empty_bits()], \
                                    Program.prog.security)
        pop_here = [prefix_empty[i+1] - prefix_empty[i] \
                        for i in range(len(self.ram))]
        entries = [entry for entry in self.ram]
        prod_entries = list(map(operator.mul, pop_here, self.ram))
        result = (1 - sum(pop_here)) * empty_entry
        result = sum(prod_entries, result)
        for i,(entry, prod_entry) in enumerate(zip(entries, prod_entries)):
            self.ram[i] = entry - prod_entry + pop_here[i] * empty_entry
        self.check(index=result.v, op='pop')
        if debug_online:
            entry = Entry(x.reveal() for x in result)
            @if_(entry.empty())
            def f():
                for i,x in enumerate((entry.v,) + entry.x):
                    @if_(x != 0)
                    def f():
                        print_ln('pop error:' + ' %s' * len(entry), *entry)
                        print_ln('%s ' * len(pop_here), \
                                     *(x.reveal() for x in pop_here))
                        crash()
        return result
    def output(self):
        self.ram.output()
    def __repr__(self):
        return repr(self.ram)

    def batch_init(self, values):
        for i,value in enumerate(values):
            index = MemValue(self.value_type.hard_conv(i))
            new_value = [MemValue(self.value_type.hard_conv(v)) \
                            for v in (value if isinstance(value, (tuple, list)) \
                            else (value,))]
            self.ram[i] = Entry(index, new_value, value_type=self.value_type)

class TrivialORAM(RefTrivialORAM, AbstractORAM):
    """ Trivial ORAM (obviously). """
    ref_type = RefTrivialORAM
    def __init__(self, size, value_type=sint, value_length=1, index_size=None, \
                     entry_size=None, contiguous=True, init_rounds=-1):
        self.index_size = index_size or log2(size)
        self.value_type = value_type
        if entry_size is None:
            self.value_length = value_length
        else:
            self.value_length = len(tuplify(entry_size))
        self.contiguous = contiguous
        entry_type = self.empty_entry().types()
        self.size = size
        self.ram = RAM(size, entry_type)
        if init_rounds != -1:
            # put memory initialization in different timer
            stop_timer()
            start_timer(1)
        self.init_mem()
        if init_rounds != -1:
            stop_timer(1)
            start_timer()

def get_n_threads(n_loops):
    if n_threads is None:
        if n_loops > 2048:
            return 8
        else:
            return 1
    else:
        return n_threads

class LinearORAM(TrivialORAM):
    """ Contiguous ORAM that stores entries in order. """
    def read_and_maybe_remove(self, index):
        return self.read(index), 0
    def add(self, entry, state=None):
        if entry.created_non_empty is True:
            self.write(entry.v, entry.x)
        else:
            self.access(entry.v, entry.x, True, entry.empty())
    def read_and_remove(self, *args):
        raise CompilerError('not implemented')
    @method_block
    def _read(self, index):
        maybe_start_timer(6)
        empty_entry = self.empty_entry(False)
        index_vector = \
            demux_array(bit_decompose(index, self.index_size))
        @map_sum(get_n_threads(self.size), n_parallel, self.size, \
                     self.value_length + 1, [self.value_type.bit_type] + \
                        [self.value_type] * self.value_length)
        def f(i):
            entry = self.ram[i]
            access_here = index_vector[i]
            return access_here * ValueTuple((entry.empty(),) + entry.x)
        not_found = f()[0]
        read_value = ValueTuple(f()[1:]) + not_found * empty_entry.x
        maybe_stop_timer(6)
        return read_value, not_found
    @method_block
    def _write(self, index, *new_value):
        maybe_start_timer(7)
        empty_entry = self.empty_entry(False)
        index_vector = \
            demux_array(bit_decompose(index, self.index_size))
        new_value = make_array(new_value)
        @for_range_multithread(get_n_threads(self.size), n_parallel, self.size)
        def f(i):
            entry = self.ram[i]
            access_here = index_vector[i]
            nv = ValueTuple(new_value)
            delta_entry = \
                Entry(0, access_here * (nv - entry.x), \
                          - access_here * entry.empty())
            self.ram[i] = entry + delta_entry
        maybe_stop_timer(7)
    @method_block
    def _access(self, index, write, new_empty, *new_value):
        empty_entry = self.empty_entry(False)
        index_vector = \
            demux_array(bit_decompose(index, self.index_size))
        new_value = make_array(new_value)
        new_empty = MemValue(new_empty)
        write = MemValue(write)
        @map_sum(get_n_threads(self.size), n_parallel, self.size, \
                     self.value_length + 1, [self.value_type.bit_type] + \
                        [self.value_type] * self.value_length)
        def f(i):
            entry = self.ram[i]
            access_here = index_vector[i]
            nv = ValueTuple(new_value)
            delta_entry = \
                Entry(0, access_here * (nv - entry.x), \
                          access_here * (new_empty - entry.empty()))
            self.ram[i] = entry + write * delta_entry
            return access_here * ValueTuple((entry.empty(),) + entry.x)
        not_found = f()[0]
        read_value = ValueTuple(f()[1:]) + not_found * empty_entry.x
        return read_value, not_found

class RefBucket(object):
    """ Bucket for tree ORAM. Contains an ORAM of some type and
    possibly two children. """
    def __init__(self, index, oram):
        self.bucket = oram.bucket_oram.ref_type(index, oram)
        self.p_children = lambda i: regint.conv((index << 1) + i)
        self.ref_children = lambda i: RefBucket(self.p_children(i), oram)
        self.oram = oram
    def check(self, depth, found=None, index=None):
        if found is None:
            found = set()
        self.bucket.check(found, index)
        if depth:
            for i in (0,1):
                self.ref_children(i).check(depth - 1, found, index)
    def __repr__(self, depth=0):
        result = ' ' * depth + repr(self.bucket) + '\n'
        if depth < self.oram.D:
            result += self.ref_children(0).__repr__(depth + 1) + \
                self.ref_children(1).__repr__(depth + 1)
        return result
    def output(self):
        print_reg_char4(cint(self.depth), 'buck')
        Program.prog.curr_tape.start_new_basicblock()
        self.bucket.output()
        print_reg_char4(cint(self.depth), 'dep')
        Program.prog.curr_tape.start_new_basicblock()
        @if_(self.p_children(1) < oram.n_buckets())
        def f():
            for i in (0,1):
                child = self.ref_children(i)
                print_reg_char4(cint(i), 'chil')
                Program.prog.curr_tape.start_new_basicblock()
                child.output()

def random_block(length, value_type):
    return bit_compose(value_type.bit_type.get_random_bit() for i in range(length))

class List(object):
    """ Debugging only. List which accepts secret values as indices
    and *reveals* them. """
    def __init__(self, size, value_type, value_length=1, init_rounds=None):
        self.value_type = value_type
        self.value_length = value_length
        self.l = [value_type.dynamic_array(size, value_type) \
                      for i in range(value_length)]
        for l in self.l:
            l.assign_all(0)
    __getitem__ = lambda self,index: [self.l[i][regint(reveal(index))] \
                                      for i in range(self.value_length)]
    def __setitem__(self, index, value):
        # print 'set', index, value, cint(reveal(index))
        # print self.l
        Program.prog.curr_tape.start_new_basicblock(name='List-pre-write')
        for i in range(self.value_length):
            self.l[i][regint(reveal(index))] = value[i]
        Program.prog.curr_tape.start_new_basicblock(name='List-post-write')
    read_and_remove = lambda self,i: (self[i], None)
    def read_and_maybe_remove(self, *args, **kwargs):
        return self.read_and_remove(*args, **kwargs), 0
    add = lambda self,entry: self.__setitem__(entry.v.read(), \
                                                  [v.read() for v in entry.x])
    def batch_init(self, values):
        for i,value in enumerate(values):
            index = self.value_type.hard_conv(i)
            new_value = [self.value_type.hard_conv(v) \
                            for v in (value if isinstance(value, (tuple, list)) \
                            else (value,))]
            self.__setitem__(index, new_value)
    def __repr__(self):
        return repr(self.l)

class LocalIndexStructure(List):
    """ Debugging only. Implements a tree ORAM index as list of
    values, *revealing* which elements are accessed. """
    def __init__(self, size, entry_size, value_type=sint, init_rounds=-1, \
                     random_init=False):
        List.__init__(self, size, value_type)
        if init_rounds:
            @for_range(init_rounds if init_rounds > 0 else size)
            def f(i):
                self.l[0][i] = random_block(entry_size, value_type)
        print('index size:', size)
    def update(self, index, value):
        read_value = self[index]
        #print 'read', index, read_value
        #print self.l
        self[index] = (value,)
        return self.value_type(read_value)
    def output(self):
        for i,v in enumerate(self):
            print_reg_char4(v.reveal(), 'i %d' % i)
    __getitem__ = lambda self,index: List.__getitem__(self, index)[0]

def get_n_threads_for_tree(size):
    if n_threads_for_tree is None:
        if size >= 2**13:
            return 8
        else:
            return 1
    else:
        return n_threads_for_tree

class TreeORAM(AbstractORAM):
    """ Tree ORAM. """
    def __init__(self, size, value_type=sint, value_length=1, entry_size=None, \
                     bucket_oram=TrivialORAM, init_rounds=-1):
        print('create oram of size', size)
        self.bucket_oram = bucket_oram
        # heuristic bucket size
        delta = 3
        k = old_div((math.log(size * size * log2(size) * 100, 2) + 21), (1 + delta))
        # size + 1 for bucket overflow check
        self.bucket_size = min(int(math.ceil((1 + delta) * k)), size + 1)
        self.D = log2(max(old_div(size, k), 2))
        print('bucket size:', self.bucket_size)
        print('depth:', self.D)
        print('complexity:', self.bucket_size * (self.D + 1))
        self.value_type = value_type
        if entry_size is not None:
            self.value_length = len(tuplify(entry_size))
        else:
            self.value_length = value_length
        self.index_size = log2(size)
        self.size = size
        empty_entry = Entry.get_empty(*self.internal_value_type())
        self.entry_type = empty_entry.types()
        self.ram = RAM(self.n_buckets() * self.bucket_size, self.entry_type)
        if init_rounds != -1:
            # put memory initialization in different timer
            stop_timer()
            start_timer(1)
        self.ram.init_mem(empty_entry)
        if init_rounds != -1:
            stop_timer(1)
            start_timer()
        self.root = RefBucket(1, self)
        self.index = self.index_structure(size, self.D, value_type, init_rounds, True)

        self.read_value = Array(self.value_length, value_type)
        self.read_non_empty = MemValue(self.value_type.bit_type(0))
        self.state = MemValue(self.value_type(0))
    @method_block
    def add_to_root(self, state, is_empty, v, *x):
        if len(x) != self.value_length:
            raise CompilerError('value length mismatch: %s, should be %s' % \
                                    (len(x), self.value_length))
        l = state
        self.root.bucket.add(Entry(v, (l,) + x, is_empty))
    def evict_bucket(self, bucket, d):
        #print_reg_char4(cint(0), 'evb')
        #print 'pre', bucket
        entry = bucket.bucket.pop()
        #print 'evict', entry
        #print 'from', bucket
        b = if_else(entry.empty(), self.value_type.bit_type.get_random_bit(), \
                        get_bit(entry.x[0], self.D - 1 - d, self.D))
        block = cond_swap(b, entry, self.root.bucket.empty_entry())
        #print 'empty', entry.empty()
        #print 'b', b
        for b in (0,1):
            # not sure if secure other than with trivial ORAM
            bucket.ref_children(b).bucket.add(block[b])
        #print 'block', block
        #print 'post', bucket
        if debug_online:
            secret_entry = entry
            entry = Entry(x.reveal() for x in entry)
            @if_(1 - entry.empty())
            def f():
                b = regint((entry.x[0] >> self.D - 1 - d) & 1)
                bucket.ref_children(b).bucket.check(new_entry=secret_entry, \
                                                        op='evic')
                bucket.ref_children(1-b).bucket.check(index=secret_entry.v, \
                                                          op='evic')
    @method_block
    def evict2(self, p_bucket1, p_bucket2, d):
        self.evict_bucket(RefBucket(p_bucket1, self), d)
        self.evict_bucket(RefBucket(p_bucket2, self), d)
    @method_block
    def read_and_renew_index(self, u):
        l_star = random_block(self.D, self.value_type)
        if use_insecure_randomness:
            new_path = regint.get_random(self.D)
            l_star = self.value_type(new_path)
        self.state.write(l_star)
        return self.index.update(u, l_star).reveal()
    @method_block
    def read_and_remove_levels(self, u, read_path):
        u = MemValue(u)
        read_path = MemValue(read_path)
        levels = self.D + 1
        parallel = get_parallel(self.index_size, *self.internal_value_type())
        @map_sum(get_n_threads_for_tree(self.size), parallel, levels, \
                     self.value_length + 1, [self.value_type.bit_type] + \
                        [self.value_type] * self.value_length)
        def process(level):
            b_index = regint(cint(2**(self.D) + read_path) >> cint(self.D - level))
            bucket = RefBucket(b_index, self)
            #print 'pre-rar level', i, 'from', bucket
            value, empty = bucket.bucket.read_and_remove(u, 1)
            self.check()
            return (1 - empty,) + value
        self.read_non_empty.write(process()[0])
        self.read_value.assign(process()[1:])
        if debug_online:
            n_found = self.read_non_empty.reveal()
            @if_((n_found != 0) * (n_found != 1))
            def f():
                cint(0).print_reg('rere')
                u.reveal().print_reg('u')
                n_found.print_reg('n')
                for i,x in enumerate(self.read_value):
                    x.reveal().print_reg('x%d' % i)
                Program.prog.curr_tape.start_new_basicblock()
                crash()
    def internal_value_type(self):
        return self.value_type, self.value_length + 1
    def n_buckets(self):
        return 2**(self.D+1)
    @method_block
    def read_and_remove(self, u):
        #print 'rar', id(self)
        #print 'pre-rar', self
        read_path = self.read_and_renew_index(u)
        #print 'rar for', u, self.read_path
        self.check()
        maybe_start_timer(3)
        self.read_and_remove_levels(u, read_path)
        read_empty = 1 - self.read_non_empty
        read_value = self.read_value
        maybe_stop_timer(3)
        self.check(u)
        #print 'rar result', u, read_value, read_empty
        #print 'post-rar', self
        # if empty:
        #     raise EmptyException('read empty value %s at %s, path %s' % \
        #                              (str(res), str(u), str(l)))
        Program.prog.curr_tape.\
            start_new_basicblock(name='read_and_remove-%d-end' % self.size)
        return [MemValue(v) for v in read_value], MemValue(read_empty)
    def add(self, entry, state=None):
        if state is None:
            state = self.state.read()
        #print_reg_char4(cint(0), 'add')
        #print 'add', id(self)
        #print 'pre-add', self
        maybe_start_timer(4)
        self.add_to_root(state, entry.empty(), \
                             self.value_type(entry.v.read()), \
                             *(self.value_type(i.read()) for i in entry.x))
        maybe_stop_timer(4)
        #print 'pre-evict', self
        maybe_start_timer(5)
        self.evict()
        maybe_stop_timer(5)
        #print 'post-evict', self
    def evict(self):
        #print 'evict root', id(self)
        #print_reg_char4(cint(0), 'ev_r')
        self.evict_bucket(self.root, 0)
        self.check()
        if self.D > 1:
            #print 'evict 1', id(self)
            #print_reg_char4(cint(0), 'ev1')
            self.evict2(self.root.p_children(0), self.root.p_children(1), 1)
            self.check()
        if self.D > 2:
            #print_reg_char4(cint(self.D), 'D')
            @for_range(2, self.D)
            def f(d):
                #print_reg_char4(d, 'ev2')
                #print 'evict 2', id(self)
                #print_reg_char4(d, 'evl2')
                s1 = regint.get_random(d)
                s2 = MemValue(regint(0))
                @do_while
                def f():
                    s2.write(regint.get_random(d))
                    return s2 == s1
                #print 's1, s2', s1, s2
                #print 'S', S
                #print 'd, 2^d', d, 1 << d
                self.evict2(s1 + (1 << d), s2 + (1 << d), d)
                self.check()

    def batch_init(self, values):
        """ Batch initalization. Obliviously shuffles and adds N entries to
            random leaf buckets. """
        m = len(values)
        assert((m & (m-1)) == 0)
        if m != self.size:
            raise CompilerError('Batch initialization must have N values.')
        if self.value_type != sint:
            raise CompilerError('Batch initialization only possible with sint.')

        depth = log2(m)
        leaves = [0] * m
        entries = [0] * m
        indexed_values = [0] * m

        # assign indices 0, ..., m-1
        for i,value in enumerate(values):
            index = MemValue(self.value_type.hard_conv(i))
            new_value = [MemValue(self.value_type.hard_conv(v)) \
                         for v in (value if isinstance(value, (tuple, list)) \
                                       else (value,))]
            indexed_values[i] = [index] + new_value

        
        # assign leaves
        for i,index_value in enumerate(indexed_values):
            leaves[i] = random_block(self.D, self.value_type)

            index = index_value[0]
            value = [leaves[i]] + index_value[1:]
            entries[i] = Entry(index, value, \
                self.value_type.hard_conv(False), value_type=self.value_type)
        
        # save unsorted leaves for position map
        unsorted_leaves = [MemValue(self.value_type(leaf)) for leaf in leaves]
        permutation.sort(leaves, comp=permutation.normal_comparator)

        bucket_sz = 0
        # B[i] = (pos, leaf, "last in bucket" flag) for i-th entry
        B = [[0]*3 for i in range(m)]
        B[0] = [0, leaves[0], 0]
        B[-1] = [None, None, sint(1)]
        s = 0

        for i in range(1, m):
            eq = leaves[i].equal(leaves[i-1])
            s = (s + eq) * eq
            B[i][0] = s
            B[i][1] = leaves[i]
            B[i-1][2] = 1 - eq
            #pos[i] = [s, leaves[i]]
            #last_in_bucket[i-1] = 1 - eq

        # shuffle
        permutation.shuffle(B, value_type=sint)
        #cint(0).print_reg('shuf')

        sz = MemValue(0) #cint(0)
        nleaves = 2**self.D
        empty_positions = Array(nleaves, self.value_type)
        empty_leaves = Array(nleaves, self.value_type)
        
        for i in range(m):
            if_then(reveal(B[i][2]))
            #if B[i][2] == 1:
            #cint(i).print_reg('last')
            if isinstance(sz, int):
                szval = sz
            else:
                szval = sz.read()
            #szval.print_reg('sz')
            empty_positions[szval] = B[i][0] #pos[i][0]
            #empty_positions[szval].reveal().print_reg('ps0')
            empty_leaves[szval] = B[i][1] #pos[i][1]
            sz += 1
            end_if()

        pos_bits = []

        for i in range(nleaves):
            leaf = empty_leaves[i]
            # split into 2 if bucket size can't fit into one field elem
            if self.bucket_size + Program.prog.security > 128:
                parity = (empty_positions[i]+1) % 2
                half = old_div((empty_positions[i]+1 - parity), 2)
                half_max = old_div(self.bucket_size, 2)
                
                bits = floatingpoint.B2U(half, half_max, Program.prog.security)[0]
                bits2 = floatingpoint.B2U(half+parity, half_max, Program.prog.security)[0]
                # (doesn't work)
                #bits2 = [0] * half_max
                ## second half with parity bit 
                #for j in range(half_max-1, 0, -1):
                #    bits2[j] = bits[j] + (bits[j-1] - bits[j]) * parity
                #bits2[0] = (1 - bits[0]) * parity
                bucket_bits = [b for sl in zip(bits2,bits) for b in sl]
            else:
                bucket_bits = floatingpoint.B2U(empty_positions[i]+1, self.bucket_size, Program.prog.security)[0]
            pos_bits += [[b, leaf] for b in bucket_bits]
        
        # sort to get empty positions first
        permutation.sort(pos_bits, comp=permutation.bitwise_list_comparator)

        # now assign positions to empty entries
        empty_entries = [0] * (self.bucket_size*2**self.D - m)
        
        for i in range(self.bucket_size*2**self.D - m):
            vtype, vlength = self.internal_value_type()
            leaf = vtype(pos_bits[i][1])
            # set leaf in empty entry for assigning after shuffle
            value = tuple([leaf] + [vtype(0) for j in range(vlength)])
            entry = Entry(vtype(0), value, vtype.hard_conv(True), vtype)
            empty_entries[i] = entry

        # now shuffle, reveal positions and place entries
        entries = entries + empty_entries
        while len(entries) & (len(entries)-1) != 0:
            entries.append(None)
        permutation.shuffle(entries, value_type=sint)
        entries = [entry for entry in entries if entry is not None]
        clear_leaves = [MemValue(entry.x[0].reveal()) for entry in entries]

        Program.prog.curr_tape.start_new_basicblock()

        bucket_sizes = Array(2**self.D, regint)
        for i in range(2**self.D):
            bucket_sizes[i] = 0
        k = 0
        for entry,leaf in zip(entries, clear_leaves):
            leaf = leaf.read()
            k += 1

            # for some reason leaf_buckets is in bit-reversed order
            bits = bit_decompose(leaf, self.D)
            rev_leaf = sum(b*2**i for i,b in enumerate(bits[::-1]))
            bucket = RefBucket(rev_leaf + (1 << self.D), self)
            # hack: 1*entry ensures MemValues are converted to sints
            bucket.bucket.ram[bucket_sizes[leaf]] = 1*entry
            bucket_sizes[leaf] += 1

        self.index.batch_init([leaf.read() for leaf in unsorted_leaves])

    def check(self, index=None):
        if debug:
            self.root.check(self.D, index=index)
    def __repr__(self):
        return repr(self.root) + '\n' + repr(self.index)
    def output(self):
        self.root.output()
        self.index.output()

class BaseORAM(TreeORAM):
    """ Debugging only. Tree ORAM revealing the access pattern. """
    index_structure = LocalIndexStructure

def put_in_new_block(function):
    def wrapper(*args, **kwargs):
        class BlockCall(object):
            def start(self):
                Program.prog.curr_tape.start_new_basicblock()
                function(*args, **kwargs)
                return self
            def join(self):
                pass
        return BlockCall()
    return wrapper

def get_log_value_size(value_type):
    """ Return log of element size. """
    if value_type == sgf2n:
        return 5
    else:
        return sint_bit_length

def get_value_size(value_type):
    """ Return element size. """
    if value_type == sint:
        return 127 - Program.prog.security
    else:
        return value_type.max_length

def get_parallel(index_size, value_type, value_length):
    """ Returning the number of parallel readings feasible, based on
    experiments. """
    value_size = get_value_size(value_type)
    if value_type == sint:
        value_size *= 2
    res = max(1, min(old_div(50 * 32, (value_length * value_size)), \
                         old_div(800 * 32, (value_length * index_size))))
    if comparison.const_rounds:
        res = max(1, old_div(res, 2))
    if 'Emulation' not in sys.path:
        print('Reading %d buckets in parallel' % res)
    return res

class PackedIndexStructure(object):
    """ Abstract class for ORAM using bit packing. """
    def __init__(self, size, entry_size=None, value_type=sint, init_rounds=-1, \
                     random_init=False):
        self.size = size
        if entry_size is None:
            self.entry_size = (log2(size),)
        else:
            self.entry_size = tuplify(entry_size)
        self.value_type = value_type
        for demux_bits in range(max_demux_bits + 1):
            self.log_entries_per_element = min(size, \
                int(math.floor(math.log(float(get_value_size(value_type)) / \
                    sum(self.entry_size), 2))))
            self.log_elements_per_block = \
                max(0, min(demux_bits, log2(size * sum(self.entry_size)) - \
                               self.log_entries_per_element))
            if self.log_entries_per_element < 0:
                self.entries_per_block = 1
                max_bits = get_value_size(value_type)
                self.split_sizes = [[]]
                for s in self.entry_size:
                    if s > max_bits:
                        raise CompilerError('Inadequate entry size %d, ' \
                                                'maximum %d' % \
                                                (s, max_bits))
                    if sum(self.split_sizes[-1]) + s > max_bits:
                        self.split_sizes.append([])
                    self.split_sizes[-1].append(s)
                self.elements_per_entry = len(self.split_sizes)
                self.log_elements_per_block = log2(self.elements_per_entry)
                self.log_entries_per_element = -self.log_elements_per_block
                print('split sizes:', self.split_sizes)
            self.log_entries_per_block = \
                self.log_elements_per_block + self.log_entries_per_element
            self.elements_per_block = 2**self.log_elements_per_block
            self.entries_per_element = 2**self.log_entries_per_element
            self.entries_per_block = 2**self.log_entries_per_block
            self.used_bits = self.entries_per_element * sum(self.entry_size)
            real_size = -(old_div(-size, self.entries_per_block))
        print('packed size:', real_size)
        print('index size:', size)
        print('entry size:', self.entry_size)
        print('log(entries per element):', self.log_entries_per_element)
        print('entries per element:', self.entries_per_element)
        print('log(entries per block):', self.log_entries_per_block)
        print('entries per block:', self.entries_per_block)
        print('log(elements per block):', self.log_elements_per_block)
        print('elements per block:', self.elements_per_block)
        print('used bits:', self.used_bits)
        if real_size > 1:
            # no need to init underlying ORAM, will be initialized implicitely
            self.l = self.storage(real_size, value_type, self.elements_per_block, \
                                      init_rounds=0)
            self.small = False
        else:
            self.l = List(1, value_type, self.elements_per_block)
            self.small = True
        if init_rounds:
            if init_rounds > 0:
                real_init_rounds = old_div(init_rounds * real_size, size)
            else:
                real_init_rounds = real_size
            print('packed init rounds:', real_init_rounds)
            @for_range(real_init_rounds)
            def f(i):
                if random_init:
                    self.l[i] = [random_block(self.used_bits, self.value_type) \
                                     for j in range(self.elements_per_block)]
                else:
                    self.l[i] = [0] * self.elements_per_block
                time()
                print_ln('packed ORAM init %s/%s', i, real_init_rounds)
        print('index initialized, size', size)
    def translate_index(self, index):
        """ Bit slicing *index* according parameters. Output is tuple
        (storage address, index with storage cell, index within
        element). """
        if self.value_type == sint:
            rem = mod2m(index, self.log_entries_per_block, log2(self.size), False)
            c = mod2m(rem, self.log_entries_per_element, \
                          self.log_entries_per_block, False)
            b = old_div((rem - c), self.entries_per_element)
            if self.small:
                return 0, b, c
            else:
                return old_div((index - rem), self.entries_per_block), b, c
        else:
            index_bits = bit_decompose(index, log2(self.size))
            l1 = self.log_entries_per_element
            l2 = self.log_entries_per_block
            c = bit_compose(index_bits[:l1])
            b = bit_compose(index_bits[l1:l2])
            if self.small:
                return 0, b, c
            else:
                a = bit_compose(index_bits[l2:])
                return a, b, c
            raise CompilerError('Cannot process indices of type', self.value_type)
    class Slicer(object):
        def __init__(self, pack, index):
            self.pack = pack
            self.a, self.b, self.c = pack.translate_index(index)
        def read(self, block):
            self.block = block
            self.index_vector = \
                demux(bit_decompose(self.b, self.pack.log_elements_per_block))
            self.vector = list(map(operator.mul, self.index_vector, block))
            self.element = get_block(sum(self.vector), self.c, \
                                         self.pack.entry_size, \
                                         self.pack.entries_per_element)
            return tuple(self.element.get_slice())
        def write(self, value):
            self.element.set_slice(value)
            anti_vector = list(map(operator.sub, self.block, self.vector))
            updated_vector = [self.element.value * i for i in self.index_vector]
            updated_block = list(map(operator.add, anti_vector, updated_vector))
            return updated_block
    class MultiSlicer(object):
        def __init__(self, pack, index):
            self.pack = pack
            self.a = index
        def read(self, block):
            res = []
            for element,sizes in zip(block,self.pack.split_sizes):
                bits = element.bit_decompose(sum(sizes))
                for size in sizes:
                    res.append(sum(bit << i \
                                       for i,bit in enumerate(bits[-size:])))
                    del bits[-size:]
            return tuple(res)
        def write(self, value):
            res = []
            i = 0
            for sizes in self.pack.split_sizes:
                res.append(0)
                for size in sizes:
                    res[-1] <<= size
                    res[-1] += value[i]
                    i += 1
            return res
    def get_slicer(self, index):
        if self.log_entries_per_element < 0:
            return self.MultiSlicer(self, index)
        else:
            return self.Slicer(self, index)
    def update(self, index, value):
        """ Updating index return current value. Has to be done in one
        step to avoid exponential blow-up in ORAM recursion. """
        return self.access(index, value, True)
    def access(self, index, value, write):
        slicer = self.get_slicer(index)
        block = self.l.read_and_maybe_remove(slicer.a)[0][0]
        read_value = slicer.read(block)
        value = if_else(write, ValueTuple(tuplify(value)), \
                            ValueTuple(read_value))
        self.l.add(Entry(MemValue(self.value_type(slicer.a)), \
                             ValueTuple(MemValue(v) \
                                            for v in slicer.write(value)), \
                             value_type=self.value_type))
        return untuplify(read_value)
    def __getitem__(self, index):
        slicer = self.get_slicer(index)
        return untuplify(slicer.read(self.l[slicer.a]))
    def __setitem__(self, index, value):
        if self.log_entries_per_element < 0:
            # no need for reading first
            self.l[index] = self.get_slicer(index).write(value)
        else:
            self.access(index, value, True)

    def batch_init(self, values):
        """ Initialize m values with indices 0, ..., m-1 """
        m = len(values)
        n_entries = max(1, old_div(m,self.entries_per_block))
        new_values = [0] * n_entries

        for i in range(n_entries):
            block = [0] * self.elements_per_block
            for j in range(self.elements_per_block):
                base = i * self.entries_per_block + j * self.entries_per_element
                for k in range(self.entries_per_element):
                    if base + k < m:
                        block[j] += values[base + k] << (k * self.entry_size)

            new_values[i] = block

        self.l.batch_init(new_values)

    def __repr__(self):
        return repr(self.l)
    def output(self):
        if self.small:
            print_reg_char4(self.l[0].reveal(), 'i0')
            print_reg_char4(self.l[1].reveal(), 'i1')

class PackedORAMWithEmpty(AbstractORAM, PackedIndexStructure):
    def __init__(self, size, entry_size=None, value_type=sint, init_rounds=-1):
        if entry_size is None:
            entry_size = log2(size)
        PackedIndexStructure.__init__(self, size, (1,) + tuplify(entry_size), \
                                          value_type, init_rounds=init_rounds)
        self.value_length = len(self.entry_size)
    @method_block
    def _read(self, index):
        res = PackedIndexStructure.__getitem__(self, index)
        return res[1:], 1 - res[0]
    def access(self, index, new_value, write, new_empty=False):
        res = PackedIndexStructure.access(self, index, (1 - new_empty,) + \
                                              tuplify(new_value), write)
        return res[1:], 1 - res[0]
    def read_and_maybe_remove(self, index):
        return self.read(index), 0
    def add(self, entry, state=None):
        self.access(entry.v, entry.x, True, entry.empty())

class LocalPackedIndexStructure(PackedIndexStructure):
    """ Debugging only. Packed tree ORAM index revealing the access
    pattern. """
    storage = staticmethod(lambda *args,**kwargs: List(*args,**kwargs))

class LocalPackedORAM(TreeORAM):
    """ Debugging only. Tree ORAM using index revealing the access
    pattern. """
    index_structure = LocalPackedIndexStructure

class BaseORAMIndexStructure(PackedIndexStructure):
    """ Debugging only. Tree ORAM index revealing the access
    pattern after one recursion. """
    storage = BaseORAM

class OneLevelORAM(TreeORAM):
    """ Debugging only. Tree ORAM using index revealing the access
    pattern after one recursion. """
    index_structure = BaseORAMIndexStructure

def OptimalORAM(size,*args,**kwargs):
    if optimal_threshold is None:
        if n_threads == 1:
            threshold = 2**11
        else:
            threshold = 2**13
    else:
        threshold = optimal_threshold
    if size <= threshold:
        return LinearORAM(size,*args,**kwargs)
    else:
        return RecursiveORAM(size,*args,**kwargs)

class RecursiveIndexStructure(PackedIndexStructure):
    """ Secure index using secure tree ORAM. """
    storage = lambda self,*args,**kwargs: OptimalORAM(*args,**kwargs)

class RecursiveORAM(TreeORAM):
    """ Secure tree ORAM using secure index. """
    index_structure = RecursiveIndexStructure

class TrivialORAMIndexStructure(PackedIndexStructure):
    """ Secure index using trivial ORAM. """
    storage = TrivialORAM

class TrivialIndexORAM(TreeORAM):
    """ Secure tree ORAM using index using trivial ORAM. """
    index_structure = TrivialORAMIndexStructure

class AtLeastOneRecursionIndexStructure(PackedIndexStructure):
    storage = RecursiveORAM

OptimalPackedORAM = RecursiveIndexStructure

class LinearPackedORAM(PackedIndexStructure):
    storage = LinearORAM

class LinearPackedORAMWithEmpty(PackedORAMWithEmpty):
    storage = LinearORAM

class AtLeastOneRecursionPackedORAMWithEmpty(PackedORAMWithEmpty):
    storage = RecursiveORAM

class OptimalPackedORAMWithEmpty(PackedORAMWithEmpty):
    storage = OptimalORAM

def test_oram(oram_type, N, value_type=sint, iterations=100):
    oram = oram_type(N, value_type=value_type, entry_size=32, init_rounds=0)
    print('initialized')
    print_ln('initialized')
    stop_timer()
    # synchronize
    Program.prog.curr_tape.start_new_basicblock(name='sync')
    value_type(0).reveal()
    Program.prog.curr_tape.start_new_basicblock(name='sync')
    start_timer()
    #oram[value_type(0)] = -1
    #iterations = N
    @for_range(iterations)
    def f(i):
        oram[value_type(i % N)] = value_type(i % N)
        #value, empty = oram.read_and_remove(value_type(i))
        #print 'first write'
        time()
        oram[value_type(i % N)].reveal().print_reg('writ')
        #print 'first read'
    @for_range(iterations)
    def f(i):
        x = oram[value_type(i % N)]
        x.reveal().print_reg('read')
    #    print 'second read'
    return oram

def test_oram_access(oram_type, N, value_type=sint, index_size=None, iterations=100):
    oram = oram_type(N, value_type=value_type, entry_size=32, \
                         init_rounds=0)
    print('initialized')
    print_reg_char4(cint(0), 'init')
    stop_timer()
    # synchronize
    Program.prog.curr_tape.start_new_basicblock(name='sync')
    sint(0).reveal()
    Program.prog.curr_tape.start_new_basicblock(name='sync')
    start_timer()
    #oram[value_type(0)] = -1
    @for_range(iterations)
    def f(i):
        oram.access(value_type(i % N), value_type(0), value_type(True))
        oram.access(value_type(i % N), value_type(i % N), value_type(True))
        print('first write')
        time()
        x = oram.access(value_type(i % N), value_type(0), value_type(False))
        x[0][0].reveal().print_reg('writ')
        print('first read')
    # @for_range(iterations)
    # def f(i):
    #     x = oram.access(value_type(i % N), value_type(0), value_type(False), \
    #                         value_type(True))
    #     x[0][0].reveal().print_reg('read')
    #     print 'second read'
    return oram

def test_batch_init(oram_type, N):
    value_type = sint
    oram = oram_type(N, value_type)
    print('initialized')
    print_reg_char4(cint(0), 'init')
    oram.batch_init([value_type(i) for i in range(N)])
    print_reg_char4(cint(0), 'done')
    @for_range(N)
    def f(i):
        x = oram[value_type(i)]
        x.reveal().print_reg('read')

def oram_delete(oram, iterations=100):
    @for_range(iterations)
    def f(i):
        x = oram.access(oram.value_type(i % oram.size), oram.value_type(0), \
                            oram.value_type(True), oram.value_type(True))
