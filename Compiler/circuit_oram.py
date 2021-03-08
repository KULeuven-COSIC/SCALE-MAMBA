
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.


from Compiler.path_oram import *
from Compiler.util import bit_compose
from Compiler import floatingpoint

def first_diff(a_bits, b_bits):
    length = len(a_bits)
    level_bits = [None] * length
    not_found = 1
    for i in range(length):
        # find first position where bits differ (i.e. first 0 in 1 - a XOR b)
        t = 1 - XOR(a_bits[i], b_bits[i])
        prev_nf = not_found
        not_found *= t
        level_bits[i] = prev_nf - not_found
    return level_bits, not_found

def find_deeper(a, b, path, start, length, compute_level=True):
    a_bits = a.value.bit_decompose(length)
    b_bits = b.value.bit_decompose(length)
    path_bits = path.bit_decompose(length)
    a_bits.reverse()
    b_bits.reverse()
    path_bits.reverse()
    level_bits = [0] * length
    # make sure that winner is set at start if one input is empty
    any_empty = OR(a.empty, b.empty)
    a_diff = [XOR(a_bits[i], path_bits[i]) for i in range(start, length)]
    b_diff = [XOR(b_bits[i], path_bits[i]) for i in range(start, length)]
    diff = [XOR(ab, bb) for ab,bb in zip(a_bits, b_bits)[start:length]]
    diff_preor = AdvInteger.PreOR([any_empty] + diff)
    diff_first = [x - y for x,y in zip(diff_preor, diff_preor[1:])]
    winner = sum((ad * df for ad,df in zip(a_diff, diff_first)), a.empty)
    winner_bits = [if_else(winner, bd, ad) for ad,bd in zip(a_diff, b_diff)]
    winner_preor = AdvInteger.PreOR(winner_bits)
    level_bits = [x - y for x,y in zip(winner_preor, [0] + winner_preor)]
    return [0] * start + level_bits + [1 - sum(level_bits)], winner

def find_deepest(paths, search_path, start, length, compute_level=True):
    if len(paths) == 1:
        return None, paths[0], 1
    l = len(paths) / 2
    _, a, a_index = find_deepest(paths[:l], search_path, start, length, False)
    _, b, b_index = find_deepest(paths[l:], search_path, start, length, False)
    level, winner = find_deeper(a, b, search_path, start, length, compute_level)
    return level, if_else(winner, b, a), if_else(winner, b_index << l, a_index)

def ge_unary_public(a, b):
    return sum(a[b-1:])

def gu_step(high, low):
    greater = high[0] * (1 - high[1])
    not_greater = high[1]
    return if_else(not_greater, 0, high[0] + low[0]), \
        if_else(greater, 0, high[1] + low[1])

def greater_unary(a, b):
    if len(a) == 1:
        return a[0], b[0]
    else:
        l = len(a) / 2
        return gu_step(greater_unary(a[l:], b[l:]), greater_unary(a[:l], b[:l]))

def comp_step(high, low):
    prod = high[0] * high[1]
    greater = high[0] - prod
    smaller = high[1] - prod
    deferred = 1 - greater - smaller
    indicator = greater, smaller, deferred
    return sum(map(operator.mul, indicator, (1, 0, low[0]))), \
        sum(map(operator.mul, indicator, (0, 1, low[1])))

def comp_binary(a, b):
    if len(a) != len(b):
        raise CompilerError('Arguments must have same length: %s %s' % (str(a), str(b)))
    if len(a) == 1:
        return a[0], b[0]
    else:
        l = len(a) / 2
        return comp_step(comp_binary(a[l:], b[l:]), comp_binary(a[:l], b[:l]))

def unary_to_binary(l):
    return sum(x * (i + 1) for i,x in enumerate(l)).bit_decompose(log2(len(l) + 1))

class CircuitORAM(PathORAM):
    def __init__(self, size, value_type=sgf2n, value_length=1, entry_size=None, \
                     stash_size=None, bucket_size=2, init_rounds=-1):
        self.bucket_oram = TrivialORAM
        self.bucket_size = bucket_size
        self.D = log2(size)
        self.logD = log2(self.D)
        self.L = self.D + 1
        print 'create oram of size %d with depth %d and %d buckets' \
            % (size, self.D, self.n_buckets())
        self.value_type = value_type
        if entry_size is not None:
            self.value_length = len(tuplify(entry_size))
        else:
            self.value_length = value_length
        self.index_size = log2(size)
        self.size = size
        empty_entry = Entry.get_empty(*self.internal_value_type())
        self.entry_type = empty_entry.types()
        self.ram = RAM(self.bucket_size * 2**(self.D+1), self.entry_type)
        self.buckets = self.ram
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
        stash_size = 20
        self.stash = TrivialORAM(stash_size, *self.internal_value_type(), \
                                     index_size=self.index_size)
        self.t = MemValue(regint(0))
        self.state = MemValue(self.value_type(0))
        self.read_path = MemValue(value_type.clear_type(0))
        # these include a read value from the stash
        self.read_value = [Array(self.D + 2, self.value_type)
                               for i in range(self.value_length)]
        self.read_empty = Array(self.D + 2, self.value_type.bit_type)
    def get_ram_index(self, path, level):
        clear_type = self.value_type.clear_type
        return ((2**(self.D) + clear_type.conv(path)) >> (self.D - (level - 1)))
    def get_bucket_ram(self, path, level):
        if level == 0:
            return self.stash.ram
        else:
            return RefRAM(self.get_ram_index(path, level), self)
    def get_bucket_oram(self, path, level):
        if level == 0:
            return self.stash
        else:
            return RefTrivialORAM(self.get_ram_index(path, level), self)
    def prepare_deepest(self, path):
        deepest = [None] * (self.D + 2)
        deepest_index = [None] * (self.D + 2)
        src = Value()
        stash_empty = self.stash.ram.is_empty()
        level, _, index = find_deepest(self.stash.ram.get_value_array(0), path, 0, self.D)
        goal = if_else(stash_empty, ValueTuple([0] * len(level)), unary_to_binary(level))
        src = if_else(stash_empty, src, Value(0))
        src_index = if_else(stash_empty, 0, index)
        buckets = [self.get_bucket_ram(path, i) for i in range(self.L + 1)]
        bucket_deepest = [(goal, src, src_index, None)]
        for i in range(1, self.L):
            l, _, index = find_deepest(buckets[i].get_value_array(0), path, i - 1, self.D)
            bucket_deepest.append((unary_to_binary(l), Value(i), index, i))
        def op(left, right, void=None):
            goal, src, src_index, _ = left
            l, secret_i, index, i = right
            high, low = comp_binary(l, goal)
            replace = high * (1 - low) * (1 - buckets[i].is_empty())
            goal = if_else(replace, bit_compose(l), \
                           bit_compose(goal)).bit_decompose(len(goal))
            src = if_else(replace, secret_i, src)
            src_index = if_else(replace, index, src_index)
            return goal, src, src_index, i
        preop_bucket_deepest = floatingpoint.PreOpL(op, bucket_deepest)
        for i in range(1, self.L + 1):
            goal, src, src_index, _ = preop_bucket_deepest[i-1]
            high, low = comp_binary(goal, bit_decompose(i, len(goal)))
            cond = 1 - low * (1 - high)
            deepest[i] = if_else(cond, src, Value())
            deepest_index[i] = if_else(cond, src_index, 0)
        return deepest, deepest_index
    def prepare_target(self, path, deepest):
        deepest, deepest_index = deepest
        dest = Value()
        src = Value()
        src_index = 0
        target = [None] * (self.L + 1)
        target_index = [None] * (self.L + 1)
        for i in range(self.L, -1 , -1):
            i_eq_src = src.equal(i, self.logD + 1)
            target[i] = if_else(i_eq_src, dest, Value())
            target_index[i] = if_else(i_eq_src, src_index, 0)
            dest = if_else(i_eq_src, Value(), dest)
            src = if_else(i_eq_src, Value(), src)
            if i == 0:
                break
            cond = or_op(dest.empty * self.get_bucket_ram(path, i).has_empty_entry(), \
                   (1 - target[i].empty)) * (1 - deepest[i].empty)
            src = if_else(cond, deepest[i], src)
            src_index = if_else(cond, deepest_index[i], src_index)
            dest = if_else(cond, Value(i), dest)
        return target, target_index
    def evict_once(self, path):
        deepest = self.prepare_deepest(path)
        Program.prog.curr_tape.start_new_basicblock(name='after-deepest-%d' % self.size)
        target = self.prepare_target(path, deepest)
        Program.prog.curr_tape.start_new_basicblock(name='after-target-%d' % self.size)
        evictor = self.evict_once_fast(path, target)
        next(evictor)
        self.add_evicted(path, next(evictor))
    def evict_once_fast(self, path, target):
        target, target_index = target
        empty_entry = Entry.get_empty(*self.internal_value_type())
        hold = empty_entry
        dest = Value()
        towrite = [None] * (self.L + 1)
        for i in range(self.L + 1):
            cond = (1 - hold.is_empty) * (dest.equal(i, self.logD + 1))
            towrite[i] = if_else(cond, hold, empty_entry)
            hold = if_else(cond, empty_entry, hold)
            dest = if_else(cond, Value(), dest)
            cond = 1 - target[i].empty
            bucket = self.get_bucket_oram(path, i)
            if i != self.L:
                index = target_index[i].bit_decompose(bucket.size)
                hold = if_else(cond, bucket.read_and_remove_by_public(index), hold)
                dest = if_else(cond, target[i], dest)
            if i == 1:
                yield
        yield towrite
    def add_evicted(self, path, towrite):
        # make sure to add after removing
        Program.prog.curr_tape.start_new_basicblock(name='evict-once-fast-add-%d' % self.size)
        for i in range(1, self.L + 1):
            self.get_bucket_oram(path, i).add(towrite[i])
        Program.prog.curr_tape.start_new_basicblock(name='evict-once-fast-end-%d' % self.size)
    def evict_deterministic(self):
        get_path = lambda x: bit_compose(reversed(x.bit_decompose(self.D)))
        paths = [get_path(2 * self.t + i) for i in range(2)]
        deepests = [self.prepare_deepest(path) for path in paths]
        targets = [self.prepare_target(path, deepest) for path,deepest in zip(paths,deepests)]
        evictors = [self.evict_once_fast(path, target) for path,target in zip(paths, targets)]
        for evictor in evictors:
            next(evictor)
            Program.prog.curr_tape.start_new_basicblock(name='evictor-%d' % self.size)
        towrites = [next(evictor) for evictor in evictors]
        for path,towrite in zip(paths, towrites):
            self.add_evicted(path, towrite)
        self.t.iadd(1)
    def evict(self):
        Program.prog.curr_tape.start_new_basicblock(name='circuit-evict-%d' % self.size)
        self.evict_deterministic()
    def bucket_indices_on_path_to(self, leaf):
        # root is at 1, different to PathORAM
        for level in range(self.D + 1):
            base = self.get_ram_index(leaf, level + 1) * self.bucket_size
            yield [base + i for i in range(self.bucket_size)]
    def output(self):
        print_ln('stash')
        self.stash.output()
        @for_range(1, 2**(self.D+1))
        def f(i):
            print_ln('node %s', self.value_type.clear_type(i))
            RefRAM(i, self).output()
        self.index.output()
    def __repr__(self):
        return repr(self.stash) + '\n' + repr(RefBucket(1, self))

class DebugCircuitORAM(CircuitORAM):
    """ Debugging only. Tree ORAM using index revealing the access
    pattern. """
    index_structure = LocalIndexStructure

def OptimalCircuitORAM(size, *args, **kwargs):
    # threshold just guessed for now
    threshold = 2**10
    if size <= threshold:
        return LinearORAM(size,*args,**kwargs)
    else:
        return RecursiveCircuitORAM(size, *args, **kwargs)

class RecursiveCircuitIndexStructure(PackedIndexStructure):
    """ Secure index using secure tree ORAM. """
    storage = staticmethod(OptimalCircuitORAM)

class RecursiveCircuitORAM(CircuitORAM):
    """ Secure tree ORAM using secure index. """
    index_structure = RecursiveCircuitIndexStructure

class AtLeastOneRecursionPackedCircuitORAM(PackedIndexStructure):
    storage = RecursiveCircuitORAM

class AtLeastOneRecursionPackedCircuitORAMWithEmpty(PackedORAMWithEmpty):
    storage = RecursiveCircuitORAM
