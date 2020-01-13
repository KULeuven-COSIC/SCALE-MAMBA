from __future__ import print_function
from __future__ import division
from builtins import map
from builtins import zip
from builtins import range
from builtins import object
from past.utils import old_div
from functools import reduce
if '_Array' not in dir():
    from oram import *
    import permutation
    _Array = Array

import oram

#import pdb

class Counter(object):
    def __init__(self, val=0, max_val=None, size=None, value_type=sgf2n):
        if value_type is sgf2n:
            if isinstance(val, int):
                val = 1 << val
            if max_val is not None:
                self.bit_length = max_val+1
            else:
                self.bit_length = sgf2n.bit_length
        elif value_type is sint:
            self.bit_length = log2(max_val+1)
        else:
            raise CompilerError('Invalid value type for Counter')
        self.value = value_type(val)
        self.value_type = value_type

        if isinstance(val, sgf2n):
            self._used = True
        else:
            self._used = False

    def used(self):
        return self._used

    def increment(self, b):
        """ Increment counter by a secret bit """
        if self.value_type is sgf2n:
            prod = self.value * b
            self.value = (2*prod + self.value - prod)
        else:
            self.value = (self.value + b)
        self._used = True

    def decrement(self, b):
        """ Decrement counter by a secret bit """
        if self.value_type is sgf2n:
            inv_2 = old_div(cgf2n(1), cgf2n(2))
            prod = self.value * b
            self.value = (inv_2*prod + self.value - prod)
        self._used = True

    def reset(self):
        if self.value_type is sgf2n:
            self.value = self.value_type(1)
        else:
            self.value = self.value_type(0)
        self._used = False

    def equal(self, i):
        """ Equality with clear int """
        if self.value_type is sgf2n:
            d = self.value - sgf2n(2**i)
            bits = d.bit_decompose(self.bit_length)
            return 1 - bits[i]
        else:
            return self.value.equal(i, self.bit_length)

    def equal_range(self, i):
        """ Vector of equality bits for 0, 1, ..., i-1 """
        return self.value.bit_decompose(self.bit_length)[:i]

def XOR(a, b):
    if isinstance(a, int) and isinstance(b, int):
        return a^b
    elif isinstance(a, sgf2n) or isinstance(b, sgf2n):
        return a + b
    else:
        try:
            return a ^ b
        except TypeError:
            return a + b - 2*a*b

def pow2_eq(a, i, bit_length=40):
    """ Test for equality with 2**i, when a is a power of 2 (gf2n only)"""
    d = a - sgf2n(2**i)
    bits = d.bit_decompose(bit_length)
    return 1 - bits[i]

def empty_entry_sorter(a, b):
    """ Sort by entry's empty bit (empty <= not empty) """
    return (1 - a.empty()) * b.empty()

def empty_entry_list_sorter(a, b):
    """ Sort a list by looking at first element's emptiness """
    return (1 - a[0].empty()) * b[0].empty()

def bucket_size_sorter(x, y):
    """ Sort buckets by their sizes. Bucket is a list of the form
        [entry_0, entry_1, ..., entry_Z, size],
        
        where size is a GF(2^n) element with a single 1 in the position
        corresponding to the bucket size """
    Z = len(x) - 1
    xs = x[-1]
    ys = y[-1]
    t = old_div(2**Z * xs, ys)
    # xs <= yx if bits 0 to Z of t are 0
    return 1 - reduce(lambda x,y: x*y, t.bit_decompose(2*Z)[:Z])


def shuffle(x, config=None, value_type=sgf2n, reverse=False):
    """ Simulate secure shuffling with Waksman network for 2 players.

    TODO: input config bits. Take mod 2 for active sec.

    Returns the network switching config so it may be re-used later.  """
    n = len(x)
    if n & (n-1) != 0:
        raise CompilerError('shuffle requires n a power of 2')
    if config is None:
        config = permutation.configure_waksman(permutation.random_perm(n))
        for i,c in enumerate(config):
            config[i] = [value_type(b) for b in c]
    permutation.waksman(x, config, reverse=reverse)
    permutation.waksman(x, config, reverse=reverse)

    return config


def LT(a, b):
    a_bits = bit_decompose(a)
    b_bits = bit_decompose(b)
    u = cgf2n()
    BitLTC1(u, a_bits, b_bits, 16)

class PathORAM(TreeORAM):
    def __init__(self, size, value_type=sgf2n, value_length=1, entry_size=None, \
                     bucket_oram=TrivialORAM, tau=3, sigma=5, stash_size=None, \
                     bucket_size=2, init_rounds=-1):
        #if size <= k:
        #    raise CompilerError('ORAM size too small')
        print('create oram of size', size)
        self.bucket_oram = bucket_oram
        self.bucket_size = bucket_size
        self.D = log2(size)
        self.logD = log2(self.D) + 1
        self.value_type = value_type
        if entry_size is not None:
            self.value_length = len(tuplify(entry_size))
        else:
            self.value_length = value_length
        self.index_size = log2(size)
        self.size = size
        self.entry_type = Entry.get_empty(*self.internal_value_type()).types()

        self.buckets = RAM(self.bucket_size * 2**(self.D+1), self.entry_type)
        if init_rounds != -1:
            # put memory initialization in different timer
            stop_timer()
            start_timer(1)
        self.buckets.init_mem(self.empty_entry())
        if init_rounds != -1:
            stop_timer(1)
            start_timer()
        self.index = self.index_structure(size, self.D, value_type, init_rounds, True)

        # deterministic eviction strategy from Gentry et al.
        self.deterministic_eviction = True
        if stash_size is None:
            if self.deterministic_eviction:
                if self.bucket_size == 2:
                    # Z=2 more efficient without sigma/tau limits
                    tau = 20
                    sigma = 20
                    stash_size = 20
                elif self.bucket_size == 3:
                    tau = 20
                    sigma = 20
                    stash_size = 2
                elif self.bucket_size == 4:
                    tau = 3
                    sigma = 5
                    stash_size = 2
                else:
                    raise CompilerError('Bucket size %d not supported' % self.bucket_size)
            else:
                tau = 3
                sigma = 5
                stash_size = 48

        self.tau = tau
        self.sigma = sigma

        self.stash_capacity = stash_size
        self.stash = TrivialORAM(stash_size, *self.internal_value_type(), \
                                     index_size=self.index_size)

        # temp storage for the path + stash in eviction
        self.temp_size = stash_size + self.bucket_size*(self.D+1)
        self.temp_storage = RAM(self.temp_size, self.entry_type)
        self.temp_levels =  [0] * self.temp_size # Array(self.temp_size, 'c')
        for i in range(self.temp_size):
            self.temp_levels[i] = 0

        # these include a read value from the stash
        self.read_value = [Array(self.D + 2, self.value_type)
                               for i in range(self.value_length)]
        self.read_empty = Array(self.D + 2, self.value_type.bit_type)

        self.state = MemValue(self.value_type(0))
        self.eviction_count = MemValue(cint(0))

        # bucket and stash sizes counter
        #self.sizes = [Counter(0, max_val=4) for i in range(self.D + 1)]
        self.stash_size = Counter(0, max_val=stash_size)

        self.read_path = MemValue(value_type.clear_type(0))

        @function_block
        def evict():
            if self.value_type == sgf2n:
                self.use_shuffle_evict = True
            else:
                self.use_shuffle_evict = True
            
            leaf = random_block(self.D, self.value_type).reveal()
            if oram.use_insecure_randomness:
                leaf = self.value_type(regint.get_random(self.D)).reveal()
            if self.deterministic_eviction:
                leaf = 0
                ec = self.eviction_count.read()
                # leaf bits already reversed so just use counter
                self.eviction_count.write((ec + 1) % 2**self.D)
                leaf = self.value_type.clear_type(ec)

            self.state.write(self.value_type(leaf))

            print('eviction leaf =', leaf)

            # load the path
            for i, ram_indices in enumerate(self.bucket_indices_on_path_to(leaf)):
                for j, ram_index in enumerate(ram_indices):
                    self.temp_storage[i*self.bucket_size + j] = self.buckets[ram_index]
                    self.temp_levels[i*self.bucket_size + j] = i
                    self.buckets[ram_index] = Entry.get_empty(self.value_type, 1)

            # load the stash
            for i in range(len(self.stash.ram)):
                self.temp_levels[i + self.bucket_size*(self.D+1)] = 0
            #for i, entry in enumerate(self.stash.ram):
            @for_range(len(self.stash.ram))
            def f(i):
                entry = self.stash.ram[i]
                self.temp_storage[i + self.bucket_size*(self.D+1)] = entry
                
                te = Entry.get_empty(self.value_type, 1)
                self.stash.ram[i] = te

            self.path_regs = [None] * self.bucket_size*(self.D+1)
            self.stash_regs = [None] * len(self.stash.ram)

            for i, ram_indices in enumerate(self.bucket_indices_on_path_to(leaf)):
                for j, ram_index in enumerate(ram_indices):
                    self.path_regs[j + i*self.bucket_size] = self.buckets[ram_index]
            for i in range(len(self.stash.ram)):
                self.stash_regs[i] = self.stash.ram[i]

            #self.sizes = [Counter(0, max_val=4) for i in range(self.D + 1)]
            if self.use_shuffle_evict:
                if self.bucket_size == 4:
                    self.size_bits = [[self.value_type.bit_type(i) for i in (0, 0, 0, 1)] for j in range(self.D+1)]
                elif self.bucket_size == 2 or self.bucket_size == 3:
                    self.size_bits = [[self.value_type.bit_type(i) for i in (0, 0)] for j in range(self.D+1)]
            else:
                self.size_bits = [[self.value_type.bit_type(0) for i in range(self.bucket_size)] for j in range(self.D+1)]
            self.stash_size = Counter(0, max_val=len(self.stash.ram))

            leaf = self.state.read().reveal()

            if self.use_shuffle_evict:
                # more efficient eviction using permutation networks
                self.shuffle_evict(leaf)
            else:
                # naive eviction method
                for i,(entry, depth) in enumerate(zip(self.temp_storage, self.temp_levels)):
                    self.evict_block(entry, depth, leaf)
    
                for i, entry in enumerate(self.stash_regs):
                    self.stash.ram[i] = entry
                for i, ram_indices in enumerate(self.bucket_indices_on_path_to(leaf)):
                    for j, ram_index in enumerate(ram_indices):
                        self.buckets[ram_index] = self.path_regs[i*self.bucket_size + j]

        self.evict = evict

    @method_block
    def read_and_remove_levels(self, u):
        #print 'reading path to', self.read_path
        leaf = self.read_path.read()
        for level in range(self.D + 1):
            ram_indices = list(self.bucket_indices_on_path_to(leaf))[level]
            #print 'level %d, bucket %d' % (level, ram_indices[0]/self.bucket_size)
            #for j in range(self.bucket_size):
            #    #bucket.bucket.ram[j].v.reveal().print_reg('lev%d' % level)
            #    print str(self.buckets[ram_indices[j]]) + ', ',
            #print '\n'
            #value, empty = bucket.bucket.read_and_remove(u, 1)

            empty_entry = self.empty_entry(False)
            skip = 1
            found = Array(self.bucket_size, self.value_type.bit_type)
            entries = [self.buckets[j] for j in ram_indices]
            indices = [e.v for e in entries]
            empty_bits = [e.empty() for e in entries]

            for j in range(self.bucket_size):
                found[j] = indices[j].equal(u, self.index_size) * \
                    (1 - empty_bits[j])

            # at most one 1 in found
            empty = 1 - sum(found)
            prod_entries = list(map(operator.mul, found, entries))
            read_value = sum((entry.x.skip(skip) for entry in prod_entries), \
                                 empty * empty_entry.x.skip(skip))
            for i,(j, entry, prod_entry) in enumerate(zip(ram_indices, entries, prod_entries)):
                self.buckets[j] = entry - prod_entry + found[i] * empty_entry

            value, empty = [MemValue(v) for v in read_value], MemValue(empty)

            for v,w in zip(self.read_value, value):
                v[level] = w.read()
            self.read_empty[level] = empty.read()
            #print 'post-rar from', bucket
            #p_bucket.write(bucket.p_children(self.read_path & 1))
            #self.read_path.irshift(1)
            self.check()

        value, empty = self.stash.read_and_remove(u, 1)
        for v, w in zip(self.read_value, value):
            v[self.D+1] = w
        self.read_empty[self.D+1] = empty

    def empty_entry(self, apply_type=True):
        vtype, vlength = self.internal_value_type()
        return Entry.get_empty(vtype, vlength, apply_type)

    def shuffle_evict(self, leaf):
        """ Evict using oblivious shuffling etc """
        evict_debug = False
        levels = [None] * len(self.temp_storage)

        bucket_sizes = Array(self.D + 2, cint)
        for i in range(self.D + 2):
            bucket_sizes[i] = regint(0)

        Program.prog.curr_tape.start_new_basicblock()
        leaf = self.state.read().reveal()

        if evict_debug:
            print_ln('\tEviction leaf: %s', leaf)

        for i,(entry, depth) in enumerate(zip(self.temp_storage, self.temp_levels)):
            lca_lev, cbits = self.compute_lca(entry.x[0], leaf, 1 - entry.empty())

            level_bits = self.adjust_lca(cbits, depth, 1 - entry.empty())
            # last bit indicates stash
            levels[i] = [sum(level_bits[j]*j for j in range(self.D+2)), level_bits[-1]]

            if evict_debug:
                @if_(1 - entry.empty().reveal())
                def f():
                    print_ln('entry (%s, %s) going to level %s', entry.v.reveal(), entry.x[0].reveal(), levels[i][0].reveal())
                    print_ln('%s ' * len(level_bits), *[b.reveal() for b in level_bits])
        if evict_debug:
            print_ln("")
        
        # sort entries+levels by emptiness: buckets already sorted so just perform a
        # sequence of merges on these and the stash
        buckets = [[[self.temp_storage[j]] + levels[j] for j in range(self.bucket_size*i,self.bucket_size*(i+1))] for i in range(self.D+1)]
        stash = [None] * (self.stash_capacity)
        
        for i in range(self.stash_capacity):
            j = i+self.bucket_size*(self.D+1)
            stash[i] = [self.temp_storage[j]] + levels[j]

        merged_entries = buckets + [stash]
        
        merged_entries = [m for sl in merged_entries for m in sl]
        me_len = len(merged_entries)
        while len(merged_entries) & (len(merged_entries)-1) != 0:
            merged_entries.append(None)
        # sort taking into account stash etc. (GF(2^n) ONLY atm)
        permutation.odd_even_merge_sort(merged_entries, lambda a,b: a[0].empty() * (a[-1] - 1 + b[-1]) + 1 - a[-1])
        
        merged_entries = merged_entries[:me_len]

        # and sort assigned positions by emptiness (non-empty first)
        empty_bits_and_levels = [[0]*self.bucket_size for i in range(self.D+1)]
        stash_bits = 0

        if evict_debug:
            print_str('Size bits: ')

        # convert bucket size bits to bits flagging emptiness for each position
        for j in range(self.D+1):
            s = self.size_bits[j]
            #for b in s:
            #    b.reveal().print_reg('u%d' % j)
            if self.bucket_size == 4:
                c = s[0]*s[1]
                if self.value_type == sgf2n:
                    empty_bits_and_levels[j][0] = [1 - self.value_type.bit_type(s[0] + s[1] + s[2] + c), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][1] = [1 - self.value_type.bit_type(s[1] + s[2]), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][2] = [1 - self.value_type.bit_type(c + s[2]), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][3] = [1 - self.value_type.bit_type(s[2]), self.value_type.clear_type(j)]
                else:
                    empty_bits_and_levels[j][0] = [1 - self.value_type.bit_type(s[0] + s[1] - c + s[2]), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][1] = [1 - self.value_type.bit_type(s[1] + s[2]), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][2] = [1 - self.value_type.bit_type(c + s[2]), self.value_type.clear_type(j)]
                    empty_bits_and_levels[j][3] = [1 - self.value_type.bit_type(s[2]), self.value_type.clear_type(j)]
            elif self.bucket_size == 2:
                if evict_debug:
                    print_str('%s,%s,', s[0].reveal(), s[1].reveal())
                empty_bits_and_levels[j][0] = [1 - self.value_type.bit_type(s[0] + s[1]), self.value_type.clear_type(j)]
                empty_bits_and_levels[j][1] = [1 - self.value_type.bit_type(s[1]), self.value_type.clear_type(j)]
            elif self.bucket_size == 3:
                c = s[0]*s[1]
                empty_bits_and_levels[j][0] = [1 - self.value_type.bit_type(s[0] + s[1] - c), self.value_type.clear_type(j)]
                empty_bits_and_levels[j][1] = [1 - self.value_type.bit_type(s[1]), self.value_type.clear_type(j)]
                empty_bits_and_levels[j][2] = [1 - self.value_type.bit_type(c), self.value_type.clear_type(j)]

        if evict_debug:
            print_ln()

        empty_bits_and_levels = [x for sl in empty_bits_and_levels for x in sl]
        while len(empty_bits_and_levels) & (len(empty_bits_and_levels)-1) != 0:
            empty_bits_and_levels.append(None)

        permutation.odd_even_merge_sort(empty_bits_and_levels, permutation.bitwise_list_comparator)

        empty_bits_and_levels = [e for e in empty_bits_and_levels if e is not None]

        # assign levels to empty positions
        stash_level = self.value_type.clear_type(self.D + 1)


        if evict_debug:
            print_ln('Bits and levels: ')
        for i, entrylev in enumerate(merged_entries):
            entry = entrylev[0]
            level = entrylev[1]

            if i < len(empty_bits_and_levels):
                new_level = (empty_bits_and_levels[i][1] - level) * entry.empty() + level
                if evict_debug:
                    print_ln('\t(empty pos %s, entry %s: empty lev %s, entry %s: new %s)', empty_bits_and_levels[i][0].reveal(), entry.empty().reveal(),
                        empty_bits_and_levels[i][1].reveal(), level.reveal(), new_level.reveal())
            else:
                new_level = level + stash_level * entry.empty()
                if evict_debug:
                    print_ln('\t(entry %s: level %s: new %s)', entry.empty().reveal(),
                        level.reveal(), new_level.reveal())
            merged_entries[i] = [entry, new_level]
        if evict_debug:
            print_ln()
        
        # shuffle entries and levels
        while len(merged_entries) & (len(merged_entries)-1) != 0:
            merged_entries.append(None) #self.root.bucket.empty_entry(False))
        permutation.rec_shuffle(merged_entries, value_type=self.value_type)
        merged_entries = [e for e in merged_entries if e is not None]

        # need to copy entries/levels to memory for re-positioning
        entries_ram = RAM(self.temp_size, self.entry_type)
        levels_array = Array(self.temp_size, cint)
        
        for i,entrylev in enumerate(merged_entries):
            if entrylev is not None:
                entries_ram[i] = entrylev[0]
                levels_array[i] = entrylev[1].reveal()
        Program.prog.curr_tape.start_new_basicblock()

        # reveal shuffled levels
        @for_range(self.temp_size)
        def f(i):
            level = regint(levels_array[i])
            sz = regint(bucket_sizes[level])
            self.temp_storage[level*self.bucket_size + sz] = entries_ram[i]
            bucket_sizes[level] += 1

        if evict_debug:
            for i in range(self.D+1):
                @if_(bucket_sizes[i] != self.bucket_size)
                def f():
                    print_str('Sizes: ')
                    for i in range(self.D+2):
                        print_str('%s,', bucket_sizes[i])
                    print_ln()
                    runtime_error('Incorrect bucket sizes')

        Program.prog.curr_tape.start_new_basicblock()
        for i, ram_indices in enumerate(self.bucket_indices_on_path_to(leaf)):
            for j, ram_index in enumerate(ram_indices):
                self.buckets[ram_index] = self.temp_storage[i*self.bucket_size + j]
        for i in range(self.stash_capacity):
            self.stash.ram[i] = self.temp_storage[i + (self.D+1)*self.bucket_size]


    def evict_block(self, entry, level, leaf):
        """ Evict an entry at a given level """
        #leaf = self.state.read().reveal()
        lca_lev, cbits = self.compute_lca(entry.x[0], leaf, 1 - entry.empty()) #, level + self.sigma)

        #new_lca = self.adjust_lca(cbits, level, 1 - entry.empty())
        lev, assigned = self.compute_pos(entry, level, lca_lev, leaf)
        #print 'evicted to lev', lev.value, assigned

    def read_and_remove(self, u):
        self.read_path.write(self.read_and_renew_index(u))
        self.check()
        self.read_and_remove_levels(u)
        values = (ValueTuple(x) for x in zip(*self.read_value))
        not_empty = [1 - x for x in self.read_empty]
        read_empty = 1 - sum(not_empty)
        read_value = sum(list(map(operator.mul, not_empty, values)), \
                             ValueTuple(0 for i in range(self.value_length)))
        self.check(u)
        Program.prog.curr_tape.\
            start_new_basicblock(name='read_and_remove-%d-end' % self.size)
        return read_value, read_empty

    def buckets_on_path_to(self, leaf):
        """ Iterator of buckets on the path to a leaf """
        bucket = RefBucket(MemValue(self.root.mem.address), self, True)
        yield bucket
        for i in range(self.D):
            bucket = bucket.ref_children(leaf & 1)
            leaf >>= 1
            yield bucket
    def bucket_indices_on_path_to(self, leaf):
        leaf = regint(leaf)
        yield list(range(self.bucket_size))
        index = 0
        for i in range(self.D):
            index = 2*index + 1 + regint(cint(leaf) & 1)
            leaf >>= 1
            yield [index*self.bucket_size + i for i in range(self.bucket_size)]

    def get_bucket_indices(self, i, l):
        """ Get RAM indices for the i-th bucket on path to leaf l """
        index = 0
        for j in range(i):
            index = 2*index + 1 + (l & 1)
            l >>= 1
        index = regint(index)
        return [index * self.bucket_size + j for j in range(self.bucket_size)]

    def get_bucket(self, i, l):
        """ Get the i-th bucket on the path to leaf l """
        bucket = RefBucket(MemValue(self.root.mem.address), self, True)
        for j in range(i):
            bucket = bucket.ref_children(l & 1)
            l >>= 1
        return bucket

    def get_children(self, i, l):
        """ Get children of the i-th bucket on level l """
        j = 2**l + i - 1
        return self.buckets[2*j+1], self.buckets[2*j+2]

    def adjust_lca(self, lca_bits, lev, not_empty, prnt=False):
        """ Adjust LCA based on bucket capacities (and original clear level, lev) """
        found = self.value_type.bit_type(0)
        assigned = self.value_type.bit_type(0)
        try_add_here = self.value_type.bit_type(0)
        new_lca = [self.value_type.bit_type(0)] * (self.D + 1)
        
        upper = min(lev + self.sigma, self.D)
        lower = max(lev - self.tau, 0)

        for j in range(upper, lower-1, -1):
            found += lca_bits[j]
            try_add_here += lca_bits[j]
            if self.bucket_size == 4:
                new_lca[j] = try_add_here * (1 - self.size_bits[j][2]) # (not_empty => lca_bits all 0)
                #new_lca[j] = found * (1 - assigned) * (1 - self.size_bits[j][2]) * not_empty
            elif self.bucket_size == 2 or self.bucket_size == 3:
                new_lca[j] = try_add_here * (1 - self.size_bits[j][1])

            if prnt:
                new_lca[j].reveal().print_reg('nl%d' % j)

            assigned += new_lca[j]
            if self.value_type == sgf2n:
                try_add_here += new_lca[j]
            else:
                try_add_here += new_lca[j] - 2*try_add_here*new_lca[j]

            if self.bucket_size == 4:
                t = new_lca[j] * self.size_bits[j][0]
                t2 = t * self.size_bits[j][1]
                # s_0 := s_0 \xor b
                # s_1 := s_1 \xor (s_0 & b)
                # s_2 := s_2 \xor (s_0 & s_1 & b)
                if self.value_type == sgf2n:
                    self.size_bits[j][0] += new_lca[j]
                    self.size_bits[j][1] += t
                    self.size_bits[j][2] += t2 #t * self.size_bits[j][1]
                else:
                    self.size_bits[j][0] += new_lca[j] - 2*t
                    self.size_bits[j][1] += t - 2*t2
                    self.size_bits[j][2] += t2
                # '1 if empty' bit
                #self.size_bits[j][3] *= (1 - new_lca[j])
            elif self.bucket_size == 2 or self.bucket_size == 3:
                t = new_lca[j] * self.size_bits[j][0]
                if self.value_type == sgf2n:
                    self.size_bits[j][0] += new_lca[j]
                else:
                    self.size_bits[j][0] += new_lca[j] - 2*t
                self.size_bits[j][1] += t
            else:
                raise CompilerError('Bucket size %d not supported' % self.bucket_size)

        add_to_stash = not_empty - sum(new_lca)

        #final_level = sum(new_lca[i]*i for i in range(self.D+1)) + add_to_stash * (self.D+1)
#
        #if_then(cint(reveal(not_empty)))
        #final_level.reveal().print_reg('lca')
        #for j in range(2):
        #    for k,b in enumerate(self.size_bits[j]):
        #        b.reveal().print_reg('u%dj%d' % (k,j))
        #end_if()
        return new_lca + [add_to_stash]

    def compute_lca(self, a, b, not_empty, limit=None):
        """ Compute depth of the least common ancestor of a and b, upper bounded by limit """
        a_bits = bit_decompose(a, self.D)
        b_bits = bit_decompose(b, self.D)
        found = [None] * self.D
        not_found = self.value_type.bit_type(not_empty) #1
        if limit is None:
            limit = self.D

        for i in range(self.D)[:limit]:
            # find first position where bits differ (i.e. first 0 in 1 - a XOR b)
            t = 1 - XOR(a_bits[i], b_bits[i])
            prev_nf = not_found
            not_found *= t
            found[i] = prev_nf - not_found
        
        if self.use_shuffle_evict:
            return None, found + [not_found]
        else:
            one = self.value_type.clear_type(1)
            lca = sum(found[i]*(one << i) for i in range(self.D)[:limit]) + \
                (one << limit) * not_found
            return Counter(lca, max_val=limit, value_type=self.value_type), found + [not_found]

    def compute_pos(self, entry, lev, levstar, leaf):
        """ Clear integer lev, secret gf2n levstar (rep. as power of 2 with Counter object). """
        pos = 0
        a = 0
        b = 0

        not_empty = 1 - entry.empty()
        
        upper = min(lev + self.sigma, self.D)
        lower = max(lev - self.tau, 0)
        levstar_eq = levstar.equal_range(upper+1)
        e = 0
        b = 0

        for j in range(upper, lower - 1, -1):
            # e = want to place at this level
            e = (1 - b) * ((1 - e)*levstar_eq[j] + e) * not_empty

            # b = can place at this level
            b = e * (1 - self.size_bits[j][-1])
            s = 1 + sgf2n(self.size_bits[j][0])
            t = cgf2n(1)
            for i in range(1, self.bucket_size):
                t <<= 1
                s += t * (self.size_bits[j][i-1] + self.size_bits[j][i])
            size_eq = (s * b).bit_decompose(self.bucket_size)
            
            a += sum(size_eq)

            #self.sizes[j].value.read().reveal().print_reg('sz%d' % j)
            #self.sizes[j].equal(self.bucket_size).reveal().print_reg('eq')
            #b.reveal().print_reg('b')
            #print 'sz%d:' % j, self.sizes[j].value #, levstar.value, b
            for i in range(self.bucket_size):
                c = size_eq[i]               
                #t = cint(c.reveal())
                #def f():
                #    entry.x[1].reveal().print_reg('writ')
                #    t.print_reg('l%di%d' % (j,i))
                #    entry.x[0].reveal().print_reg('w lf')
                #if_statement(t,f)
                #if c.reveal() == 1:
                #    print 'writing block %d at level %d on path to %d' % (i,j,leaf)
                #    print 'writing', entry*c + bucket.ram[i]*(1 - c)
                prev = self.path_regs[i + j*self.bucket_size]
                new = c * (entry - prev) + prev
                self.path_regs[i + j*self.bucket_size] = new

                self.size_bits[j][i] += c

        add_to_stash = not_empty - a # (1-a) * not_empty
        stash_eq = Counter(self.stash_size.value * add_to_stash, len(self.stash.ram)).equal_range(self.stash.size)
        
        for i,s in enumerate(self.stash_regs):
            c = stash_eq[i] #* add_to_stash
            te = c * (entry - s) + s # entry*c + s*(1 - c)
            self.stash_regs[i] = te
        self.stash_size.increment(add_to_stash)

        #if add_to_stash.reveal() == 1:
        #    print 'stash', self.stash_size.value

        return levstar, a

    def add(self, entry, state=None):
        if state is None:
            state = self.state.read()
        l = state
        x = tuple((self.value_type(i.read())) for i in entry.x)

        e = Entry(self.value_type(entry.v.read()), (l,) + x, entry.empty())

        #self.temp_storage[self.temp_size-1] = e * 1
        #self.temp_levels[self.temp_size-1] = 0
        #print 'adding', self.temp_storage[-1][0]
        try:
            self.stash.add(e)
        except Exception:
            print(self)
            raise
        self.evict()

class LocalPathORAM(PathORAM):
    """ Debugging only. Path ORAM using index revealing the access
    pattern. """
    index_structure = LocalPackedIndexStructure

def OptimalORAM(size, *args, **kwargs):
    # threshold set from experiments (lower than in SCSL)
    threshold = 2**10
    if size <= threshold:
        return LinearORAM(size,*args,**kwargs)
    else:
        return RecursivePathORAM(size, *args, **kwargs)

class RecursivePathIndexStructure(PackedIndexStructure):
    storage = staticmethod(OptimalORAM)

class RecursivePathORAM(PathORAM):
    index_structure = RecursivePathIndexStructure

class AtLeastOneRecursionPackedPathORAM(PackedIndexStructure):
    storage = RecursivePathORAM

class AtLeastOneRecursionPackedPathORAMWithEmpty(PackedORAMWithEmpty):
    storage = RecursivePathORAM

class OptimalPackedPathORAMWithEmpty(PackedORAMWithEmpty):
    storage = staticmethod(OptimalORAM)
