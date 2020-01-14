from __future__ import division
# This file is only used in the scripts for doing auto testing

from builtins import range
from past.utils import old_div
from builtins import object
import operator
from collections import defaultdict
import sys
from functools import reduce


if sys.version[0] == "3":
    # If we're in python3, integers have unlimited size
    long = int


class Vector(object):
    def __init__(self,value=0,size=0):
        self.v = [value] * size
    def store_in_mem(self,addr):
        pass
    def binop(self,other,op):
        res = Vector()
        if isinstance(other, Vector):
            res.v = [op(self.v[i], other.v[i]) for i in range(len(self.v))]
        else:
            res.v = [op(self.v[i], other) for i in range(len(self.v))]
        return res
    def rop(self,other,op):
        res = Vector()
        res.v = [op(other, self.v[i]) for i in range(len(self.v))]
        return res
    def __add__(self,other):
        return self.binop(other,operator.add)
    def __mul__(self,other):
        return self.binop(other,operator.mul)
    def __sub__(self,other):
        return self.binop(other,operator.sub)
    def __div__(self,other):
        return self.binop(other,operator.div)
    def __mod__(self,other):
        return self.binop(other,operator.mod)
    def __pow__(self,other):
        return self.binop(other,operator.pow)
    def __lt__(self,other):
        return self.binop(other,operator.lt)
    def __gt__(self,other):
        return self.binop(other,operator.gt)
    def __le__(self,other):
        return self.binop(other,operator.le)
    def __ge__(self,other):
        return self.binop(other,operator.ge)
    def __eq__(self,other):
        return self.binop(other,operator.eq)
    def __ne__(self,other):
        return self.binop(other,operator.ne)
    def __and__(self,other):
        return self.binop(other,operator.and_)
    def __or__(self,other):
        return self.binop(other,operator.or_)
    def __xor__(self,other):
        return self.binop(other,operator.xor)
    def __lshift__(self,other):
        return self.binop(other,operator.lshift)
    def __rshift__(self,other):
        return self.binop(other,operator.rshift)
    __radd__ = __add__
    __rmul__ = __mul__
    __rand__ = __and__
    __ror__ = __or__
    __rxor__ = __xor__
    def __rsub__(self,other):
        return self.rop(other,operator.sub)
    def __rdiv__(self, other):
        return self.rop(other,operator.div)
    def __rmod__(self,other):
        return self.rop(other,operator.mod)
    def __rpow__(self,other):
        return self.rop(other,operator.pow)
    def __rlshift__(self,other):
        return self.rop(other,operator.lshift)
    def __rrshift__(self,other):
        return self.rop(other,operator.rshift)
    def __neg__(self):
        return 0 - self
    def __invert__(self):
        res = Vector()
        res.v = [~i for i in self.v]
        return res
    def bit_decompose(self, bit_length=None):
        return [Vector(self[0], len(self))] * (bit_length or 100)
    def __getitem__(self,index):
        return self.v[index]
    def __len__(self):
        return len(self.v)

load_int = lambda x=0,size=None: int(x) if size is None else Vector(x,size)
load_int.load_mem = load_int
load_int_to_secret = load_int
def load_int_to_secret_vector(vector):
    res = Vector()
    res.v = vector
    return res
get_random_bit = lambda size=None: 1 if size is None else Vector(1,size)
get_random_int = lambda x,size=None: 2 ** x - 1 if size is None else Vector(2**x-1,size)

class _register(long):
    store_in_mem = lambda x,y: None
    load_mem = classmethod(lambda cls,addr,size=None: cls(0) if size is None else Vector(cls(0),size))
    get_random = classmethod(lambda cls,*args: cls(0))
    __add__ = lambda x,y: type(x)(int(x) + y)
    __sub__ = lambda x,y: type(x)(int(x) - y)
    __rsub__ = lambda x,y: type(x)(y - int(x))
    __mul__ = lambda x,y: type(x)(int(x) * y)
    __div__ = lambda x,y: type(x)(old_div(int(x), y))
    __rdiv__ = lambda x,y: type(x)(old_div(y, int(x)))
    __mod__ = lambda x,y: type(x)(int(x) % y)
    __rmod__ = lambda x,y: type(x)(y % int(x))
    __neg__ = lambda x: type(x)(-int(x))
    __pow__ = lambda x,y: type(x)(int(x) ** y)
    __lshift__ = lambda x,y: type(x)(int(x) << y)
    __rshift__ = lambda x,y: type(x)(int(x) >> y)
    __rlshift__ = lambda x,y: type(x)(y << int(x))
    __rrshift__ = lambda x,y: type(x)(y >> int(x))
    __radd__ = __add__
    __rmul__ = __mul__


class _sbit(_register):
    __and__ =  lambda self,other,x=None,y=None: sbit(bool(self) & other) if (isinstance(other, _sbit)) else sregint(bool(self) * other)
    __or__ = lambda self, other, x=None, y=None: sbit(bool(self) | other)
    __xor__ = lambda self, other, x=None, y=None: sbit(bool(self) ^ other)
    __neg__ = lambda self, x=None: sbit(1 - bool(self))
    reveal = lambda self: regint(self)
sbit = lambda x=0,size=None: (x if isinstance(x, Vector) else _sbit(x)) if size is None else Vector(_sbit(x),size)
sbit.basic_type = _sbit
sbit.type = _sbit


class _sregint(_register):
    __and__ =  lambda self,other,x=None,y=None: sregint(int(self) * other) if (isinstance(other, _sbit)) else sregint(int(self) & other)
    __or__ = lambda self, other, x=None, y=None: sregint(int(self) | other)
    __xor__ = lambda self, other, x=None, y=None: sregint(int(self) ^ other)

    mul_2_sint = lambda self, other, x = None, y = None: (sregint((int (self) * other ) >> 64), sregint((int (self) * other ) % (2 ** 64)))
    reveal = lambda self: regint(self)
    less_than = lambda self,other,x=None,y=None: sbit(int(self) < other)
    greater_than = lambda self,other,x=None,y=None: sbit(int(self) > other)
    less_equal = lambda self,other,x=None,y=None: sbit(int(self) <= other)
    greater_equal = lambda self,other,x=None,y=None: sbit(int(self) >= other)
    equal = lambda self,other,x=None,y=None: sbit(int(self) == other)
    not_equal = lambda self,other,x=None,y=None: sbit(int(self) != other)

    __lt__ = less_than
    __gt__ = greater_than
    __le__ = less_equal
    __ge__ = greater_equal
    __eq__ = equal
    __ne__ = not_equal
    __neg__ = lambda self: sregint(-int(self))

    __rand__ = __and__
    __ror__ = __or__
    __rxor__ = __xor__



sregint = lambda x=0,size=None: (x if isinstance(x, Vector) else _sregint(x)) if size is None else Vector(_sregint(x),size)
sregint.load_mem = _sregint.load_mem
sregint.basic_type = _sregint
sregint.type = _sregint

class _sint(_register):
    less_than = lambda self,other,x=None,y=None: sint(int(self) < other)
    greater_than = lambda self,other,x=None,y=None: sint(int(self) > other)
    less_equal = lambda self,other,x=None,y=None: sint(int(self) <= other)
    greater_equal = lambda self,other,x=None,y=None: sint(int(self) >= other)
    equal = lambda self,other,x=None,y=None: sint(int(self) == other)
    not_equal = lambda self,other,x=None,y=None: sint(int(self) != other)
    reveal = lambda self: cint(self)
    mod2m = lambda self,other,x=None,y=None: self % 2**other
    pow2 = lambda self,x=None,y=None: 2**self
    right_shift = lambda self,other,x=None,y=None: self >> other
    bit_decompose = lambda self,length=None,*args: \
        [(self >> i) & 1 for i in range(length or program.bit_length)]
    __lt__ = less_than
    __gt__ = greater_than
    __le__ = less_equal
    __ge__ = greater_equal
    __eq__ = equal
    __ne__ = not_equal
    __neg__ = lambda self: sint(-int(self))

sint = lambda x=0,size=None: (x if isinstance(x, Vector) else _sint(x)) if size is None else Vector(_sint(x),size)
sint.load_mem = _sint.load_mem
sint.get_random_bit = get_random_bit
sint.get_random_int = get_random_int
sint.get_random_triple = lambda size=None: (0, 0, 0) if size is None else (Vector(0, size), Vector(0, size), Vector(0, size))
sint.get_random_square = lambda size=None: (0, 0) if size is None else (Vector(0, size), Vector(0, size))
sint.basic_type = _sint
sint.type = _sint
reveal = lambda x: x

class _cint(_register):
    print_reg = lambda x,y=None: None

cint = lambda x=0,size=None: (x if isinstance(x, Vector) else _cint(x)) if size is None else Vector(_cint(x),size)
cint.load_mem = cint
cint.get_random = cint

class A(object):
    def malloc(self, size, reg_type):
        pass
    def run_tape(self, f, x):
        global arg
        arg = x
        f()
    new_tape = lambda self,f,*args: f
    join_tape = lambda self,*args: None
    set_bit_length = lambda *args: None
    set_security = lambda *args: None
A.options = A()

comparison = A()
comparison.PRandInt = lambda x,y: None
comparison.PRandM = lambda x,y,z,a,b,c: None
comparison.ld2i = lambda x,y: None
comparison.PreMulC_with_inverses = lambda x,y: None
comparison.PreMulC_without_inverses = lambda x,y: None
comparison.BitLTC1 = lambda x,y,z,a: ([None] * 100, [None] * 100, [None] * 100, [None] * 100, [[None] * 100] * 100, [[None] * 100] * 100, [None] * 100, [None] * 100)
comparison.BitLTL = lambda x,y,z,a: ([None] * 100, [None] * 100)
comparison.CarryOut = lambda x,y,z,a,b: None
comparison.Mod2m = lambda *args: [None] * 5 + [[None] * 6] + [None]
comparison.Mod2 = lambda *args: None
comparison.PreMulC = lambda x: [None] * 100
comparison.KMulC = lambda x: None
comparison.PreMulC_with_inverses_and_vectors = lambda x,y: [[[None] * 100] * 100] * 8
comparison.LTZ = lambda x,y,z,a: None
comparison.TruncRoundNearest = lambda x,y,z,a: None
class F(float):
    v = p = z = s = None
    __add__ = lambda x,y: F(float(x) + y)
    __sub__ = lambda x,y: F(float(x) - y)
    __mul__ = lambda x,y: F(float(x) * y)
    __div__ = lambda x,y: F(0) if y == 0 else F(float(x) / y)
    __pow__ = lambda x,y: F(float(x)**y)
    __lt__ = lambda x,y: sint(float(x) < y)
    __gt__ = lambda x,y: sint(float(x) > y)
    __le__ = lambda x,y: sint(float(x) <= y)
    __ge__ = lambda x,y: sint(float(x) >= y)
    __eq__ = lambda x,y: sint(float(x) == y)
    __ne__ = lambda x,y: sint(float(x) != y)
    __neg__ = lambda x: F(-float(x))
sfloat = lambda x,y=None,z=None,a=None,err=None,size=None: F(x) if size is None else Vector(F(x), size)
sfloat.vlen = 24
sfloat.plen = 8
sfloat.error = None

class CF(float):
    v = p = z = s = None
    __add__ = lambda x,y: CF(float(x) + y)
    __sub__ = lambda x,y: CF(float(x) - y)
    __mul__ = lambda x,y: CF(float(x) * y)
    __div__ = lambda x,y: CF(0) if y == 0 else CF(float(x) / y)
    __pow__ = lambda x,y: CF(float(x)**y)
    __lt__ = lambda x,y: cint(float(x) < y)
    __gt__ = lambda x,y: cint(float(x) > y)
    __le__ = lambda x,y: cint(float(x) <= y)
    __ge__ = lambda x,y: cint(float(x) >= y)
    __eq__ = lambda x,y: cint(float(x) == y)
    __ne__ = lambda x,y: cint(float(x) != y)
    __neg__ = lambda x: CF(-float(x))

cfloat = lambda x,y=None,z=None,a=None,size=None: CF(x) if size is None else Vector(CF(x), size)
cfloat.vlen = 24
cfloat.plen = 8


class _fixregister(float):
    v = None

    @classmethod
    def set_precision(cls, f, k = None):
        if k is None:
            k = 2 * f 
        cls.f = f
        cls.k = k
    store_in_mem = lambda x,y: None
    load_mem = classmethod(lambda cls,addr,size=None: cls(0) if size is None else Vector(cls(0),size))

    __add__ = lambda x,y: type(x)(float(x) + y)
    __sub__ = lambda x,y: type(x)(float(x) - y)
    __rsub__ = lambda x,y: type(x)(y - float(x))
    __mul__ = lambda x,y: type(x)(float(x) * y)
    __div__ = lambda x,y: type(x)(float(x) / y)
    __rdiv__ = lambda x,y: type(x)(y / float(x))
    __neg__ = lambda x: type(x)(-float(x))
    __pow__ = lambda x,y: type(x)(float(x) ** y)
    __radd__ = __add__
    __rmul__ = __mul__

    load_int = lambda x, y: type(x)(float(x))


_fixregister.set_precision(20, 41)

class _sfix(_fixregister):

    less_than = lambda x,y,z=None: sint(float(x) < y)
    greater_than = lambda self,other,x=None,y=None: sint(float(self) > other)
    less_equal = lambda self,other,x=None,y=None: sint(float(self) <= other)
    greater_equal = lambda self,other,x=None,y=None: sint(float(self) >= other)
    equal = lambda self,other,x=None,y=None: sint(float(self) == other)
    not_equal = lambda self,other,x=None,y=None: sint(float(self) != other)

    __lt__ = less_than
    __gt__ = greater_than
    __le__ = less_equal
    __ge__ = greater_equal
    __eq__ = equal
    __ne__ = not_equal

    __neg__ = lambda self: sfix(-float(self))
    reveal = lambda self: cfix(self)

sfix = lambda x=0,size=None: (x if isinstance(x, Vector) else _sfix(x)) if size is None else Vector(_sfix(x),size)
sfix.load_mem = _sfix.load_mem
sfix.reveal = lambda x: x

class _cfix(_fixregister):

    less_than = lambda x,y,z=None: sint(float(x) < y) if (isinstance(y, _sfix) or isinstance(y,F)) \
        else regint(float(x) < y)
    less_equal = lambda x,y,z=None: sint(float(x) <= y) if (isinstance(y, _sfix) or isinstance(y,F))\
        else regint(float(x) <= y)
    greater_equal = lambda x,y,z=None: sint(float(x) >= y) if (isinstance(y, _sfix) or isinstance(y,F)) \
        else regint(float(x) >= y)
    greater_than = lambda x,y,z=None: sint(float(x) > y) if (isinstance(y, _sfix) or isinstance(y,F)) \
        else regint(float(x) > y)
    equal = lambda x,y,z=None: sint(float(x) == y) if (isinstance(y, _sfix) or isinstance(y,F)) \
        else regint(float(x) == y)
    not_equal = lambda x,y,z=None: sint(float(x) != y) if (isinstance(y, _sfix) or isinstance(y,F)) \
        else regint(float(x) != y)

    __lt__ = less_than
    __gt__ = greater_than
    __le__ = less_equal
    __ge__ = greater_equal
    __eq__ = equal
    __ne__ = not_equal

cfix = lambda x=0,size=None: (x if isinstance(x, Vector) else _cfix(x)) if size is None else Vector(_cfix(x),size)
cfix.load_mem = cfix

load_float_to_secret = sfloat
floatingpoint = A()
floatingpoint.Trunc = lambda x,y,z,a,b=False: (None,None) if b else None
floatingpoint.B2U = lambda x,y,z: ([None] * 100, None)
floatingpoint.Pow2 = lambda x,y,z: 2 ** x
floatingpoint.PreORC = lambda x,y: [type(x[0])()] * len(x)
floatingpoint.bits = lambda x,y: [None] * 100
floatingpoint.BitDec = lambda x,y,z,a: [None] * 100
floatingpoint.two_power = lambda x: 2**x
floatingpoint.PreOpL = lambda x,y: [(None,None)] * 100
floatingpoint.carry = None
floatingpoint.Inv = lambda x: 0
floatingpoint.KOpL = lambda x,y: [None] * 100
floatingpoint.KORL = lambda x,y: None
floatingpoint.KORC = lambda x,y: None
floatingpoint.BitAdd = lambda x,y: [None] * 100
floatingpoint.TruncRoundNearestAdjustOverflow = lambda x,y,z,a: (None, None)
floatingpoint.TruncPr = lambda x,y,z,a: None
floatingpoint.SDiv = lambda x,y,z,a: None
floatingpoint.SDiv_mono = lambda x,y,z,a: None

import math
mpc_math = A()
# currently testing for sfix type
mpc_math.test_sin_fx = lambda x: sfix(math.sin(x))
mpc_math.test_cos_fx = lambda x: sfix(math.cos(x))
mpc_math.test_tan_fx = lambda x: sfix(math.tan(x))
mpc_math.log2_fx = lambda x: sfix(math.log(x, 2))
mpc_math.log_fx = lambda x,y: sfix(math.log(x, y))
mpc_math.pow_fx = lambda x,y: sfix(math.pow(x, y))
mpc_math.exp2_fx = lambda y: sfix(math.pow(2,y))

mpc_math.asin = lambda x: sfix(math.asin(x))
mpc_math.acos = lambda x: sfix(math.acos(x))
mpc_math.atan = lambda x: sfix(math.atan(x))
mpc_math.test_sqrt_no_param = lambda x: sfix(math.sqrt(x))
mpc_math.test_sqrt_param = lambda x, k, f: sfix(math.sqrt(x))

# currently testing for sfloat type
mpc_math.test_sin_float = lambda x: sfloat(math.sin(x))
mpc_math.test_cos_float = lambda x: sfloat(math.cos(x))
mpc_math.test_tan_float = lambda x: sfloat(math.tan(x))

sort = lambda x: x.sort()
chunky_odd_even_merge_sort = sort
chunkier_odd_even_merge_sort = lambda x,**kwargs: x.sort()
loopy_chunkier_odd_even_merge_sort = lambda x,**kwargs: x.sort()
odd_even_merge_sort = sort
loopy_odd_even_merge_sort = sort
cond_swap = lambda x,y: (x,y) if x < y else (y,x)
program = A()
program.restart_main_thread = lambda: None
program.curr_tape = A()
program.curr_tape.start_new_basicblock = lambda: None
program.security = None
program.public_input = lambda x: None

class MPCThread(object):
    def __init__(self, target, name, args=[]):
        target(*args)
    def start(self, arg=None):
        pass
    def join(self):
        pass
load_secret_mem = lambda x: None
load_clear_mem = lambda x: None
store_in_mem = lambda x,y: None
reveal = lambda x: x
print_mem = lambda *args: None
print_reg = lambda *args: None
do_loop = lambda *args, **kwargs: None
if_statement = lambda *args, **kwargs: None
get_thread_number = lambda size=None: regint(0) if size is None else Vector(regint(0),size)
get_arg = lambda size=None: regint(arg) if size is None else Vector(regint(arg),size)
arg = 0

def intify(a):
    if isinstance(a, (tuple, list)):
        return [intify(x) for x in a]
    else:
        return regint(a) if isinstance(a, int) else a
class FunctionTape(object):
    def __init__(self, f):
        self.f = f
    def __call__(self, *args):
        return MPCThread(self.f, '', intify(args))
function_tape = lambda x: FunctionTape(x)
function_block = lambda x: lambda *args: intify(x(*intify(args)))
method_block = lambda x: x
range_loop = lambda x,y,z=None,a=None: x(regint((z or y) - (a or 1)))
for_range = lambda x,y=None,z=None: lambda a: (range_loop(a,x,y,z), a)[1]
for_range_parallel = lambda *args: lambda *args: lambda: None
map_reduce_single = for_range_parallel
map_reduce = map_reduce_single
map_sum = lambda a,b,c,d,e: lambda *args: lambda: None if d == 1 else [None] * d
foreach_enumerate = lambda a: lambda f: None

def for_range_multithread(n_threads, n_parallel, n_loops, thread_mem_req={}):
    def decorator(loop_body):
        for i in range(n_loops):
            i = regint(i)
            if thread_mem_req:
                loop_body(i, [regint(0)] * thread_mem_req[regint])
            else:
                loop_body(i)
    return decorator

class Array(list):
    def __init__(self, length, reg_type, address=0):
        try:
            reg_type(0)
        except:
            reg_type = lambda x: x
        self.value_type = reg_type
        self[:] = (reg_type(i) for i in range(length or 1000))
        self.address = address
    def assign(self, other):
        self[:] = (self.value_type(x) for x in other)
    def assign_all(self, other):
        self[:] = [self.value_type(other)] * len(self)
    def __setitem__(self, index, value):
        if isinstance(index, slice):
            list.__setitem__(self, index, (self.value_type(x) for x in value))
        else:
            list.__setitem__(self, index, self.value_type(value))

class Matrix(list):
    def __init__(self,n,m,t,*args):
        self[:] = [Array(m, t) for i in range(n)]

mergesort = lambda x: x.sort()
and_ = lambda *args: lambda: reduce(lambda x,y: x and y(), args, True)
or_ = lambda *args: lambda: reduce(lambda x,y: x or y(), args, False)
not_ = lambda x: lambda: not x
if_then = lambda x: None
else_then = end_if = lambda: None
do_while = lambda x: x()
while_do = lambda y,*args: lambda x: x(*args)
class MemValue(object):
    def __init__(x,y):
        if not isinstance(y, (_sint,_cint,float)):
            y = regint(y)
        x.value = y
    def write(x,y):
        x.value = type(x.value)(y)
    read = lambda x: x.value
    __add__ = lambda x,y: x.value + y
    def iadd(x,y):
        x.value += y
        return x
    def reveal(x):
        return x.value

class MemFloat(MemValue, F):
    v = p = z = s = sint()
    def __init__(x,y):
        x.value = F(y)
    read = lambda x: x.value

class MemFix(MemValue, _sfix):
    v = sint()
    def __init__(x, y):
        x.value = _sfix(x)
    read = lambda x: x.value


sint.MemValue = lambda value: MemValue(sint(value))
sint.Array = lambda size, addr=None: Array(size, sint, addr)
sint.Matrix = lambda n, m, addr=None: Matrix(n, m, sint, addr)

sfix.MemValue = lambda value: MemValue(sfix(value))
sfix.Array = lambda size, addr=None: Array(size, sfix, addr)
sfix.Matrix = lambda n, m, addr=None: Matrix(n, m, sfix, addr)

cfix.MemValue = lambda value: MemValue(cfix(value))
cfix.Array = lambda size, addr=None: Array(size, cfix, addr)
cfix.Matrix = lambda n, m, addr=None: Matrix(n, m, cfix, addr)

sfloat.MemValue = lambda value: MemValue(sfloat(value))
sfloat.Array = lambda size: Array(size, sfloat)
sfloat.Matrix = lambda n, m: Matrix(n, m, sfloat)

gprint_reg = lambda x,y=None: None
time = lambda: None
start_timer = lambda *args: None
stop_timer = lambda *args: None

class regint(_register):
    pass

print_ln = lambda *args: None

public_input = lambda *args: regint()

no_result_testing = lambda: sys.exit()
