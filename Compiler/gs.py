
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

import sys
import math

if 'Emulation' in sys.path:
    from Emulation import types
else:
    from Compiler import types

from Compiler.util import *

# emulatable
from oram import OptimalORAM,LinearORAM,RecursiveORAM,TrivialORAM,Entry
from library import for_range,do_while,time,start_timer,stop_timer,if_,print_ln,crash,print_str

class OMatrixRow(object):
    def __init__(self, oram, base, add_type):
        self.oram = oram
        self.base = base
        self.add_type = add_type
    def get_index(self, offset):
        if isinstance(offset, types._secret):
            return self.base + self.add_type.hard_conv(offset)
        else:
            return self.base + offset
    def __getitem__(self, offset):
        return untuplify(self.read(offset)[0])
    def __setitem__(self, offset, item):
        self.oram[self.get_index(offset)] = item
    def read(self, offset):
        return self.oram.read(self.get_index(offset))

class OMatrix:
    def __init__(self, N, M=None, oram_type=OptimalORAM, int_type=types.sint):
        print 'matrix', oram_type
        self.N = N
        self.M = M or N
        self.oram = oram_type(N * self.M, entry_size=log2(N), init_rounds=0, \
                                  value_type=int_type.basic_type)
        self.int_type = int_type
    def __getitem__(self, a):
        if math.log(self.M, 2) % 1 == 0 or self.int_type == types.sint:
            add_type = self.int_type.basic_type
        else:
            class add_type(self.int_type):
                n_bits = log2(self.N * self.M)
            if type(a) == self.int_type.basic_type:
                a = add_type(a)
        return OMatrixRow(self.oram, a * self.M, add_type)

class OReverseMatrixRow(object):
    def __init__(self, oram, index, N, M, basic_type):
        self.oram = oram
        self.N = N
        self.M = M
        self.index = index
        self.basic_type = basic_type
    def __getitem__(self, offset):
        return untuplify(self.read(offset)[0])
    def read(self, offset):
        temp = TrivialORAM(self.M, self.basic_type, 1, log2(self.N))
        prefs = self.oram[self.index]
        for i in range(self.M):
            temp.ram[i] = Entry(prefs[i], i, value_type=self.basic_type)
        return temp.read(offset)

class OReverseMatrix(OMatrix):
    def __init__(self, N, M, oram_type=OptimalORAM, int_type=types.sint):
        self.N = N
        self.M = M
        self.oram = oram_type(N, entry_size=(log2(N),)*M, init_rounds=0, \
                                  value_type=int_type.basic_type)
        self.basic_type = int_type.basic_type
    def __getitem__(self, a):
        return OReverseMatrixRow(self.oram, a, self.N, self.M, self.basic_type)
    def __setitem__(self, index, value):
        self.oram[index] = value

class OStack:
    def __init__(self, N, oram_type=OptimalORAM, int_type=types.sint):
        print 'stack', oram_type
        self.oram = oram_type(N, entry_size=log2(N), init_rounds=0, \
                                  value_type=int_type.basic_type)
        self.size = types.MemValue(int_type(0))
        self.int_type = int_type
    def append(self, item, for_real=True):
        self.oram.access(self.size, item, for_real)
        self.size.iadd(self.int_type(for_real))
    def pop(self):
        self.size.isub(1)
        return self.oram[self.size]

class Matchmaker:
    def init_hard(self, n_loops=None):
        if n_loops is None or n_loops > self.N * self.M:
            inner_loops = self.M
            outer_loops = self.N
        else:
            inner_loops = min(n_loops, self.M)
            outer_loops = n_loops / inner_loops
        self.m_prefs = OMatrix(self.N, self.M, oram_type=self.oram_type, \
                                   int_type=self.int_type)
        @for_range(outer_loops)
        def f(i):
            time()
            types.cint(i).print_reg('mpre')
            @for_range(inner_loops)
            def f(j):
                self.m_prefs[i][j] = (-i + j + self.N - 1) % (self.N - 1)
            if self.M < self.N:
                self.m_prefs[i][self.M-1] = (2 * self.N - 2 - i) % self.N
            else:
                self.m_prefs[i][self.N-1] = self.N - 1
        if self.reverse:
            self.f_ranks = OReverseMatrix(self.N, self.M, \
                                              oram_type=self.oram_type, \
                                              int_type=self.int_type)
        else:
            self.f_ranks = OMatrix(self.N, oram_type=self.oram_type, \
                                       int_type=self.int_type)
        @for_range(outer_loops)
        def f(i):
            time()
            types.cint(i).print_reg('fran')
            @for_range(inner_loops)
            def f(j):
                if self.reverse:
                    self.f_ranks[i] = tuple((-i - j + 2 * self.N - 2) % self.N \
                                                for j in range(self.M))
                else:
                    self.f_ranks[i][(-i - j + 2 * self.N - 2) % self.N] = j
                #self.f_ranks[i][j].reveal().print_reg()

    def init_easy(self):
        self.m_prefs = OMatrix(self.N, self.M, oram_type=self.oram_type, \
                                   int_type=self.int_type)
        @for_range(self.N)
        def f(i):
            time()
            types.cint(i).print_reg('mpre')
            @for_range(self.M)
            def f(j):
                self.m_prefs[i][j] = (i + j) % self.N
        self.f_ranks = OMatrix(self.N, oram_type=self.oram_type, \
                                   int_type=self.int_type)
        @for_range(self.N)
        def f(i):
            time()
            types.cint(i).print_reg('fran')
            @for_range(self.M)
            def f(j):
                self.f_ranks[i][(j-i+self.N)%self.N] = j

    def engage(self, man, woman, for_real):
        self.wives.access(man, woman, for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('a')
        self.husbands.access(woman, man, for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('b')
        #(man * 10 + woman * 1 + for_real * 100).reveal().print_reg('eng')
        # if for_real:
        #     print 'engage', man, woman
        #     self.wives[man] = woman
        #     self.husbands[woman] = man

    def dump(self, man, woman, for_real):
        self.wives.delete(man, for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('c')
        self.husbands.delete(woman, for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('d')
        self.unengaged.append(man, for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('e')
        #(man * 10 + woman + for_real * 100).reveal().print_reg('dump')
        # if for_real:
        #     print 'dump', man, woman
        #     self.wives[man] = clown
        #     self.husbands[woman] = clown

    def propose(self, man, woman, for_real):
        (fiance,), free = self.husbands.read(woman)
        #self.husbands.ram[0].x[0].reveal().print_reg('f')
        engaged = 1 - free
        rank_man = self.f_ranks[woman][man]
        #self.husbands.ram[0].x[0].reveal().print_reg('g')
        (rank_fiance,), worst_fiance = self.f_ranks[woman].read(engaged*fiance)
        #self.husbands.ram[0].x[0].reveal().print_reg('h')
        leaving = self.int_type(rank_man) < self.int_type(rank_fiance)
        if self.M < self.N:
            leaving = 1 - (1 - leaving) * (1 - worst_fiance)
        print_str('woman: %s, man: %s, fiance: %s, worst fiance: %s, ', \
                     *(x.reveal() for x in (woman, man, fiance, worst_fiance)))
        print_ln('rank man: %s, rank fiance: %s, engaged: %s, leaving: %s', \
                     *(x.reveal() for x in \
                           (rank_man, rank_fiance, engaged, leaving)))
        self.dump(fiance, woman, engaged * leaving * for_real)
        self.engage(man, woman, (1 - (engaged * (1 - leaving))) * for_real)
        self.unengaged.append(man, engaged * (1 - leaving) * for_real)
        #self.husbands.ram[0].x[0].reveal().print_reg('i')

    def match(self, n_loops=None):
        if n_loops is None or n_loops > self.N * self.M:
            loop = do_while
            init_rounds = self.N
        else:
            loop = for_range(n_loops)
            init_rounds = n_loops / self.M
        self.wives = \
            self.oram_type(self.N, entry_size=log2(self.N), \
                               init_rounds=0, value_type=self.basic_type)
        self.husbands = \
            self.oram_type(self.N, entry_size=log2(self.N), \
                               init_rounds=0, value_type=self.basic_type)
        propose = \
            self.oram_type(self.N, entry_size=log2(self.N), \
                               init_rounds=0, value_type=self.basic_type)
        self.unengaged = OStack(self.N, oram_type=self.oram_type, \
                                    int_type=self.int_type)
        @for_range(init_rounds)
        def f(i):
            self.unengaged.append(i)
        rounds = types.MemValue(types.regint(0))
        @loop
        def f(i=None):
            rounds.iadd(1)
            time()
            man = self.unengaged.pop()
            #self.husbands.ram[0].x[0].reveal().print_reg('j')
            pref = self.int_type(propose[man])
            if self.M < self.N and n_loops is None:
                @if_((pref == self.M).reveal())
                def f():
                    print_ln('run out of acceptable women')
                    crash()
            #self.husbands.ram[0].x[0].reveal().print_reg('k')
            propose[man] = pref + 1
            #self.husbands.ram[0].x[0].reveal().print_reg('l')
            self.propose(man, self.m_prefs[man][pref], True)
            print_ln('man: %s, pref: %s, left: %s', \
                         *(x.reveal() for x in (man, pref, self.unengaged.size)))
            # self.wives[man].reveal().print_reg('wife')
            return types.regint((self.unengaged.size > 0).reveal())
        print_ln('%s rounds', rounds)
        @for_range(init_rounds)
        def f(i):
            types.cint(i).print_reg('wife')
            self.husbands[i].reveal().print_reg('husb')

    def __init__(self, N, M=None, reverse=False, oram_type=OptimalORAM, \
                     int_type=types.sint):
        self.N = N
        self.M = N if M is None else M
        self.oram_type = oram_type
        self.reverse = reverse
        self.int_type = int_type
        self.basic_type = int_type.basic_type
        print 'match', self.oram_type
