"""
This is the collection of all possible types managed by the compiler
that are available for MAMBA. This includes base types and abstract classes.

Main Families of Classes include modp based types:
    sint
    cint
    sfix
    cfix
    sfloat
    cfloat
And mod 2^n types:
    regint
    sregint
    sbit
"""
from __future__ import print_function
from __future__ import division
##@types.py
from builtins import zip
from builtins import str
from builtins import range
from past.utils import old_div
from builtins import object
from Compiler.program import Tape
from Compiler.exceptions import *
from Compiler.instructions import *
from Compiler.instructions_base import *
from floatingpoint import two_power
import comparison, floatingpoint
import math
import util
import operator
from functools import reduce


class MPCThread(object):
    def __init__(self, target, name, args=[], runtime_arg=None):
        """ Create a thread from a callable object. """
        if not callable(target):
            raise CompilerError('Target %s for thread %s is not callable' % (target, name))
        self.name = name
        self.tape = Tape(program.name + '-' + name, program)
        self.target = target
        self.args = args
        self.runtime_arg = runtime_arg
        self.running = 0

    def start(self, runtime_arg=None):
        self.running += 1
        program.start_thread(self, runtime_arg or self.runtime_arg)

    def join(self):
        if not self.running:
            raise CompilerError('Thread %s is not running' % self.name)
        self.running -= 1
        program.stop_thread(self)


def vectorize(operation):
    def vectorized_operation(self, *args, **kwargs):
        if len(args):
            if (isinstance(args[0], Tape.Register) or isinstance(args[0], sfloat)) \
                    and args[0].size != self.size:
                raise CompilerError('Different vector sizes of operands')
        set_global_vector_size(self.size)
        res = operation(self, *args, **kwargs)
        reset_global_vector_size()
        return res

    return vectorized_operation


def vectorized_classmethod(function):
    def vectorized_function(cls, *args, **kwargs):
        size = None
        if 'size' in kwargs:
            size = kwargs.pop('size')
        if size:
            set_global_vector_size(size)
            res = function(cls, *args, **kwargs)
            reset_global_vector_size()
        else:
            res = function(cls, *args, **kwargs)
        return res

    return classmethod(vectorized_function)


def vectorize_init(function):
    def vectorized_init(*args, **kwargs):
        size = None
        if len(args) > 1 and (isinstance(args[1], Tape.Register) or \
                              isinstance(args[1], sfloat)):
            size = args[1].size
            if 'size' in kwargs and kwargs['size'] is not None \
                    and kwargs['size'] != size:
                raise CompilerError('Mismatch in vector size')
        if 'size' in kwargs and kwargs['size']:
            size = kwargs['size']
        if size is not None:
            set_global_vector_size(size)
            res = function(*args, **kwargs)
            reset_global_vector_size()
        else:
            res = function(*args, **kwargs)
        return res

    return vectorized_init


def set_instruction_type(operation):
    def instruction_typed_operation(self, *args, **kwargs):
        set_global_instruction_type(self.instruction_type)
        res = operation(self, *args, **kwargs)
        reset_global_instruction_type()
        return res

    return instruction_typed_operation


def read_mem_value(operation):
    def read_mem_operation(self, other, *args, **kwargs):
        if isinstance(other, MemValue):
            other = other.read()
        return operation(self, other, *args, **kwargs)

    return read_mem_operation


class _number(object):
    @staticmethod
    def bit_compose(bits):
        return sum(b << i for i, b in enumerate(bits))

    def square(self):
        return self * self

    def __add__(self, other):
        if other is 0 or other is 0L:
            return self
        else:
            var = self.add(other)
            return var

    def __mul__(self, other):
        if other is 0 or other is 0L:
            return 0
        elif other is 1 or other is 1L:
            return self
        else:
            return self.mul(other)

    __radd__ = __add__
    __rmul__ = __mul__

    @vectorize
    def __pow__(self, exp):
        if isinstance(exp, int) and exp >= 0:
            if exp == 0:
                return self.__class__(1)
            exp = bin(exp)[3:]
            res = self
            for i in exp:
                res = res.square()
                if i == '1':
                    res *= self
            return res
        else:
            return NotImplemented


class _int(object):
    def if_else(self, a, b):
        return self * (a - b) + b

    def cond_swap(self, a, b):
        prod = self * (a - b)
        return a - prod, b + prod


class _register(Tape.Register, _number):
    @vectorized_classmethod
    def conv(cls, val):
        if isinstance(val, MemValue):
            val = val.read()
        if isinstance(val, cls):
            return val
        elif not isinstance(val, _register):
            try:
                return type(val)(cls.conv(v) for v in val)
            except TypeError:
                pass
        return cls(val)

    @vectorized_classmethod
    @read_mem_value
    def hard_conv(cls, val):
        if type(val) == cls:
            return val
        elif not isinstance(val, _register):
            try:
                return val.hard_conv_me(cls)
            except AttributeError:
                try:
                    return type(val)(cls.hard_conv(v) for v in val)
                except TypeError:
                    pass
        return cls(val)

    @vectorized_classmethod
    @set_instruction_type
    def _load_mem(cls, address, direct_inst, indirect_inst):
        res = cls()
        if isinstance(address, _register):
            indirect_inst(res, regint.conv(address))
        else:
            direct_inst(res, address)
        return res

    @set_instruction_type
    @vectorize
    def _store_in_mem(self, address, direct_inst, indirect_inst):
        if isinstance(address, _register):
            indirect_inst(self, regint.conv(address))
        else:
            direct_inst(self, address)

    @classmethod
    def prep_res(cls, other):
        return cls()

    def __init__(self, reg_type, val, size):
        super(_register, self).__init__(reg_type, program.curr_tape, size=size)
        if isinstance(val, (int, long)):
            self.load_int(val)
        elif val is not None:
            self.load_other(val)

    def sizeof(self):
        return self.size


class _clear(_register):
    __slots__ = []

    @set_instruction_type
    @vectorize
    def load_other(self, val):
        if isinstance(val, type(self)):
            movc(self, val)
        else:
            self.convert_from(val)

    @vectorize
    @read_mem_value
    def convert_from(self, val):
        if not isinstance(val, regint):
            val = regint(val)
        convint(self, val)

    @set_instruction_type
    @vectorize
    def print_reg(self, comment=''):
        print_reg(self)
        print_char4("  # ")
        print_char4(comment)
	print_char("\n")

    @set_instruction_type
    @vectorize
    def print_reg_pl(self):
        print_reg(self)

    @set_instruction_type
    @vectorize
    def raw_output(self):
        raw_output(self)

    @set_instruction_type
    @read_mem_value
    @vectorize
    def clear_op(self, other, c_inst, ci_inst, reverse=False):
        cls = self.__class__
        res = self.prep_res(other)
        if isinstance(other, cls):
            c_inst(res, self, other)
        elif isinstance(other, (int, long)):
            if self.in_immediate_range(other):
                ci_inst(res, self, other)
            else:
                if reverse:
                    c_inst(res, cls(other), self)
                else:
                    c_inst(res, self, cls(other))
        else:
            return NotImplemented
        return res

    @set_instruction_type
    @read_mem_value
    @vectorize
    def coerce_op(self, other, inst, reverse=False):
        cls = self.__class__
        res = cls()
        if isinstance(other, (int, long)):
            other = cls(other)
        elif not isinstance(other, cls):
            return NotImplemented
        if reverse:
            inst(res, other, self)
        else:
            inst(res, self, other)
        return res

    def add(self, other):
        return self.clear_op(other, addc, addci)

    def mul(self, other):
        return self.clear_op(other, mulc, mulci)

    def __sub__(self, other):
        return self.clear_op(other, subc, subci)

    def __rsub__(self, other):
        return self.clear_op(other, subc, subcfi, True)

    def __div__(self, other):
        return self.clear_op(other, divc, divci)

    def __rdiv__(self, other):
        return self.coerce_op(other, divc, True)

    def __eq__(self, other):
        if isinstance(other, (_clear, int, long)):
            return regint(self) == other
        else:
            return NotImplemented

    def __ne__(self, other):
        return 1 - (self == other)

    def __and__(self, other):
        return self.clear_op(other, andc, andci)

    def __xor__(self, other):
        return self.clear_op(other, xorc, xorci)

    def __or__(self, other):
        return self.clear_op(other, orc, orci)

    __rand__ = __and__
    __rxor__ = __xor__
    __ror__ = __or__

    def reveal(self):
        return self


class cint(_clear, _int):
    """ Clear mod p integer type. """
    __slots__ = []
    instruction_type = 'modp'
    reg_type = 'c'

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address to the cint x
              x.load_mem(address)
        """
        return cls._load_mem(address, ldmc, ldmci)

    def store_in_mem(self, address):
        r"""Stores the cint value x to the address
              x.store_in_mem(address)
        """
        self._store_in_mem(address, stmc, stmci)

    @staticmethod
    def in_immediate_range(value):
        return value < 2 ** 31 and value >= -2 ** 31

    def __init__(self, val=None, size=None):
        super(cint, self).__init__('c', val=val, size=size)

    @classmethod
    def public_input(cls, channel=0):
        r"""Get public input from IO channel c
                x=cint.public_input(c)
        """
        res = cls()
        input_clear(res, channel)
        return res

    def public_output(self, channel=0):
        r"""Send public output to IO channel c
                 x.public_output(c)
        """
        output_clear(self, channel);

    @vectorize
    def load_int(self, val):
        if val:
            # +1 for sign
            program.curr_tape.require_bit_length(1 + int(math.ceil(math.log(abs(val)))))
        if self.in_immediate_range(val):
            ldi(self, val)
        else:
            max = 2 ** 31 - 1
            sign = old_div(abs(val), val)
            val = abs(val)
            chunks = []
            while val:
                mod = val % max
                val = old_div((val - mod), max)
                chunks.append(mod)
            sum = cint(sign * chunks.pop())
            for i, chunk in enumerate(reversed(chunks)):
                sum *= max
                if i == len(chunks) - 1:
                    addci(self, sum, sign * chunk)
                elif chunk:
                    sum += sign * chunk

    def __mod__(self, other):
        return self.clear_op(other, modc, modci)

    def __rmod__(self, other):
        return self.coerce_op(other, modc, True)

    def __lt__(self, other):
        if isinstance(other, (sfloat)):
            return other > self

        if isinstance(other, (type(self), int, long)):
            return regint(self) < other
        else:
            return NotImplemented

    def __gt__(self, other):
        if isinstance(other, (sfloat)):
            return other < self

        if isinstance(other, (type(self), int, long)):
            return regint(self) > other
        else:
            return NotImplemented

    def __le__(self, other):
        if isinstance(other, (sfloat)):
            return other >= self

        return 1 - (self > other)

    def __ge__(self, other):
        if isinstance(other, (sfloat)):
            return other <= self

        return 1 - (self < other)

    def __lshift__(self, other):
        return self.clear_op(other, shlc, shlci)

    def __rshift__(self, other):
        return self.clear_op(other, shrc, shrci)

    def __neg__(self):
        return 0 - self

    @vectorize
    def __invert__(self):
        res = cint()
        notc(res, self, program.bit_length)
        return res

    def __rpow__(self, base):
        if base == 2:
            return 1 << self
        else:
            return NotImplemented

    @vectorize
    def __rlshift__(self, other):
        return cint(other) << self

    @vectorize
    def __rrshift__(self, other):
        return cint(other) >> self

    @read_mem_value
    def mod2m(self, other, bit_length=None, signed=None):
        r"""Returns x modulo 2^other
               x.mod2m(other)
         """
        return self % 2 ** other

    @read_mem_value
    def right_shift(self, other, bit_length=None):
        r"""Returns x >> other
                x.right_shift(other)
        """
        return self >> other

    @read_mem_value
    def greater_than(self, other, bit_length=None):
        r"""Returns the value of x>other
                 x.greater_than(other)
        """
        return self > other

    def pow2(self, bit_length=None):
        r"""Returns 2^x
                  x.pow2()
        """
        return 2 ** self

    def bit_decompose(self, bit_length=None):
        if bit_length == 0:
            return []
        bit_length = bit_length or program.bit_length
        return floatingpoint.bits(self, bit_length)

    def legendre(self):
        r"""Returns the legendre symbol of x, wrt the cint prime p
               x.lengendre()
        """
        res = cint()
        legendrec(res, self)
        return res

    def digest(self):
        res = cint()
        digestc(res, self)
        return res


cint.bit_type = cint


class regint(_register, _int):
    """ Clear integer register type. """
    __slots__ = []
    reg_type = 'r'
    instruction_type = 'modp'
    scalars = (int, long)

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address to the regint x
              x.load_mem(address)
        """
        return cls._load_mem(address, ldmint, ldminti)

    def store_in_mem(self, address):
        r"""Stores the regint value x to the address
              x.store_in_mem(address)
        """
        self._store_in_mem(address, stmint, stminti)

    @classmethod
    def public_input(cls, channel=0):
        r"""Get public input from IO channel c
                x=regint.public_input(c)
        """
        res = cls()
        input_int(res, channel)
        return res

    def public_output(self, channel=0):
        r"""Send public output to IO channel c
                 x.public_output(c)
        """
        output_int(self, channel);

    @vectorized_classmethod
    def pop(cls):
        res = cls()
        popint(res)
        return res

    @vectorized_classmethod
    def get_random(cls, bit_length):
        if isinstance(bit_length, int):
            bit_length = regint(bit_length)
        res = cls()
        rand(res, bit_length)
        return res

    @vectorize_init
    def __init__(self, val=None, size=None):
        super(regint, self).__init__(self.reg_type, val=val, size=size)

    @staticmethod
    def in_immediate_range(val):
        return cint.in_immediate_range(val)

    def load_int(self, val):
        if self.in_immediate_range(val):
            ldint(self, val)
        else:
            lower = val % 2 ** 32
            upper = val >> 32
            if lower >= 2 ** 31:
                lower -= 2 ** 32
                upper += 1
            addint(self, regint(upper) * regint(2 ** 16) ** 2, regint(lower))

    @read_mem_value
    def load_other(self, val):
        if isinstance(val, cint):
            convmodp(self, val)
        elif isinstance(val, regint):
            addint(self, val, regint(0))
        else:
            try:
                val.to_regint(self)
            except AttributeError:
                raise CompilerError("Cannot convert '%s' to integer" % \
                                    type(val))

    @vectorize
    @read_mem_value
    def int_op(self, other, inst, reverse=False):
        if isinstance(other, (_secretModp, _secretMod2)):
            return NotImplemented
        elif not isinstance(other, type(self)):
            other = type(self)(other)
        res = regint()
        if reverse:
            inst(res, other, self)
        else:
            inst(res, self, other)
        return res

    def add(self, other):
        return self.int_op(other, addint)

    def __sub__(self, other):
        return self.int_op(other, subint)

    def __rsub__(self, other):
        return self.int_op(other, subint, True)

    def mul(self, other):
        return self.int_op(other, mulint)

    def __neg__(self):
        return 0 - self

    def __div__(self, other):
        return self.int_op(other, divint)

    def __rdiv__(self, other):
        return self.int_op(other, divint, True)

    def __mod__(self, other):
        return self - (old_div(self, other)) * other

    def __rmod__(self, other):
        return regint(other) % self

    def __rpow__(self, other):
        return other ** cint(self)

    def __eq__(self, other):
        return self.int_op(other, eqint)

    def __ne__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return 1 - (self == other)

    def __lt__(self, other):
        return self.int_op(other, ltint)

    def __gt__(self, other):
        return self.int_op(other, gtint)

    def __le__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return 1 - (self > other)

    def __ge__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return 1 - (self < other)

    def __lshift__(self, other):
        if isinstance(other, (int, long)):
            return self * 2 ** other
        else:
            return regint(cint(self) << other)

    def __rshift__(self, other):
        if isinstance(other, (int, long)):
            return old_div(self, 2 ** other)
        else:
            return regint(cint(self) >> other)

    def __rlshift__(self, other):
        return regint(other << cint(self))

    def __rrshift__(self, other):
        return regint(other >> cint(self))

    def __and__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return regint(other & cint(self))

    def __or__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return regint(other | cint(self))

    def __xor__(self, other):
        if isinstance(other, _secretMod2): return NotImplemented
        return regint(other ^ cint(self))

    __rand__ = __and__
    __ror__ = __or__
    __rxor__ = __xor__

    def mod2m(self, *args, **kwargs):
        return cint(self).mod2m(*args, **kwargs)

    def bit_decompose(self, bit_length=None):
        res = []
        x = self
        two = regint(2)
        for i in range(bit_length or program.bit_length):
            y = old_div(x, two)
            res.append(x - two * y)
            x = y
        return res

    @staticmethod
    def bit_compose(bits):
        two = regint(2)
        res = 0
        for bit in reversed(bits):
            res *= two
            res += bit
        return res

    def reveal(self):
        return self

    @vectorize
    def print_reg(self):
        print_int(self)


# Added Types for new Opcodes:

##
# Responsible for basic secret operations that are shared by
# GC and LSSS constructions.
# It serves as a template for GC and LSSS based implementations
class _sec(_register):
    __slots__ = []

    def __init__(self, reg_type, val=None, size=None):
        super(_sec, self).__init__(reg_type, val=val, size=size)

    @set_instruction_type
    @vectorize
    def load_int(self, val):
        return NotImplemented

    @vectorize
    def load_clear(self, val):
        return NotImplemented

    @vectorize
    def load_self(self, val):
        return NotImplemented

    ##
    # instantiate other
    # it includes instantiation of secret_type
    # @params: other param
    # @return: instance of _sec child
    @set_instruction_type
    @read_mem_value
    @vectorize
    def load_other(self, val):
        if isinstance(val, self.clear_type):
            self.load_clear(val)
        elif isinstance(val, type(self)):
            self.load_self(val)
        elif isinstance(val, self.secret_type):
            self.load_secret(val)
        else:
            self.load_clear(self.clear_type(val))

    ##
    # Executes operation whether the instruction is against a secret
    # a mixed operation (secret and clear register) or
    # secret versus a native type. Ideally this function should
    # manage also complex types. But that would also requiere low level
    # opcodes for those.
    # Currently there are no opcodes for native types hence we reuse
    # the opcode for clear values
    @set_instruction_type
    @read_mem_value
    @vectorize
    def secret_op(self, other, s_inst, m_inst = None, si_inst = None, reverse=False):
        cls = self.__class__
        res = self.prep_res(other)

        # isolates the case where there is only one structure
        if m_inst == None and si_inst == None:
            # mimics the reverse structure from types.py
            if reverse:
                s_inst(res, other, self)
            else:
                s_inst(res, self, other)
            return res

        if isinstance(other, regint) and (res.clear_type != regint):
            other = res.clear_type(other)
        if isinstance(other, cls):
            s_inst(res, self, other)
        elif isinstance(other, res.clear_type):
            if reverse:
                m_inst(res, other, self)
            else:
                m_inst(res, self, other)
        elif isinstance(other, (int, long)):
            if self.clear_type.in_immediate_range(other) and (si_inst != m_inst):
                si_inst(res, self, other)
            else:
                if reverse:
                    m_inst(res, res.clear_type(other), self)
                else:
                    m_inst(res, self, res.clear_type(other))
        else:
            return NotImplemented
        return res

    def add(self, other):
        return NotImplemented

    def mul(self, other):
        return NotImplemented

    def __sub__(self, other):
        return NotImplemented

    def __rsub__(self, other):
        return NotImplemented

    @vectorize
    def __div__(self, other):
        return NotImplemented

    @vectorize
    def __rdiv__(self, other):
        return NotImplemented

    @set_instruction_type
    @vectorize
    def reveal(self):
        return NotImplemented

    @set_instruction_type
    def reveal_to(self, player, channel=0):
        return NotImplemented


##
# Class which holds shared functionality
# by all mod2 classses (binary and boolean)
# (from old secret)
# that belongs to both sbit and sregint
class _secretMod2(_sec):
    __slots__ = []

    ##
    # initialization for all types based on 2^n
    def __init__(self, reg_type, val=None, size=None):
        if isinstance(val, self.clear_type):
            size = val.size
        super(_secretMod2, self).__init__(reg_type, val=val, size=size)


    ##
    # reveals input
    # legacy from _secret
    # @params player: int player id
    @set_instruction_type
    def reveal_to(self, player, channel=0):
        private_output(self, player, channel)

##
# secret  class for all integer types mod2n
# this is equivalent to the old _secret class
# but for sregint
class _secretInt(_secretMod2):
    __slots__ = []

    ##
    # initialization paramenter
    def __init__(self, reg_type, val=None, size=None):
        super(_secretInt, self).__init__(reg_type, val=val, size=size)

    ##
    # instantiate an int or long object to sregint
    # @params val: int, long value
    # @return: sregint object containing val
    @set_instruction_type
    @vectorize
    def load_int(self, val):
        if self.clear_type.in_immediate_range(val):
            ldsint(self, val)
        else:
            self.load_clear(self.clear_type(val))

    ##
    # @params val: regint value
    # @return: sregint object containing val
    @vectorize
    def load_clear(self, val):
        convregsreg(self, val)

    # instantiate an sregint object to sregint
    # @params val: sregint value
    # @return: new register sregint object containing val
    @vectorize
    def load_self(self, val):
        movsint(self, val)

    # instantiate an sint object to sregint
    # @params val: sint value
    # @return: sregint object containing val
    @vectorize
    def load_secret(self, val):
        convsintsreg(self, val)


    # legacy from _secret
    # it reveals input
    @set_instruction_type
    @vectorize
    def reveal(self):
        res = self.clear_type()
        opensint(res, self)
        return res


    def add(self, other):
        r"""
        returns the self + other, add secret, add mix, add int
        """
        return self.secret_op(other, addsint, addsintc, addsintc)


    def mul(self, other):
        r"""
        returns the self * other, mul secret, mul mix, mul int
        """
        return self.secret_op(other, mulsint, mulsintc, mulsintc)

    def div(self, other, reverse = False):
        r"""
        returns the self/other or other/sefl
        """
        local_other = parse_sregint(other)
        res = sregint()

        if ~reverse:
            divsint(res, self, local_other)
        else:
            divsint(res, local_other, self)

        return res

    def __sub__(self, other):
        r"""
        returns the self - other, sub secret, sub mix, sub int
        """
        return self.secret_op(other, subsint, subsintc, subsintc)

    def __rsub__(self, other):
        r"""
        returns the self - other, sub secret, sub mix, sub int
        """
        return self.secret_op(other, subsint, subcints, subcints, True)

    def __div__(self, other):
        r"""
        returns the self / other, div secret 
        """
        #return self.secret_op(other, divsint)
        return self.div(other)

    def __rdiv__(self, other):
        r"""
        returns the other /  self, div secret
        """
        # return self.secret_op(other, divsint, True)
        return self.div(other, True)

    def __lshift__(self, other):
        return self.secret_op(other, shlsint)


    def __rshift__(self, other):
        return self.secret_op(other, shrsint)


##
# secret class for modp operations
# (old class secret)
# it implementes basic functionality of sint
# changed to better align namewise and
# with new generic sec functionality class sec.
class _secretModp(_sec):
    __slots__ = []

    @vectorized_classmethod
    @set_instruction_type
    def get_random_triple(cls):
        res = (cls(), cls(), cls())
        triple(*res)
        return res

    @vectorized_classmethod
    @set_instruction_type
    def get_random_bit(cls):
        res = cls()
        bit(res)
        return res

    @vectorized_classmethod
    @set_instruction_type
    def get_random_square(cls):
        res = (cls(), cls())
        square(*res)
        return res

    ##
    # initialization paramenter
    def __init__(self, reg_type, val=None, size=None):
        if isinstance(val, self.clear_type):
            size = val.size
        super(_secretModp, self).__init__(reg_type, val=val, size=size)


    #defined at the same level as in _secretInt
    @set_instruction_type
    @vectorize
    def load_int(self, val):
        if self.clear_type.in_immediate_range(val):
            ldsi(self, val)
        else:
            self.load_clear(self.clear_type(val))

    @vectorize
    def load_clear(self, val):
        addm(self, self.__class__(0), val)

    @vectorize
    def load_self(self, val):
        movs(self, val)


    # instantiate an sregint object to sint
    # @params val: sregint value
    # @return: sint object containing val
    @vectorize
    def load_secret(self, val):
        convsregsint(self, val)

    def add(self, other):
        return self.secret_op(other, adds, addm, addsi)

    def mul(self, other):
        return self.secret_op(other, muls, mulm, mulsi)

    def __sub__(self, other):
        return self.secret_op(other, subs, subml, subsi)

    def __rsub__(self, other):
        return self.secret_op(other, subs, submr, subsfi, True)

    @vectorize
    def __div__(self, other):
        return self * (old_div(self.clear_type(1), other))

    @vectorize
    def __rdiv__(self, other):
        a, b = self.get_random_square()
        return old_div(other * a, (a * self).reveal())

    @set_instruction_type
    @vectorize
    def square(self):
        res = self.__class__()
        sqrs(res, self)
        return res

    @set_instruction_type
    @vectorize
    def reveal(self):
        res = self.clear_type()
        asm_open(res, self)
        return res

    @set_instruction_type
    def reveal_to(self, player, channel=0):
        private_output(self, player, channel)

##
# Base 2 secret bit type
# Returned by Comparison Tests in sregint
class sbit(_secretMod2):
    """ Shared  mod 2 type. """
    __slots__ = []
    instruction_type = 'mod2n'
    clear_type = regint
    reg_type = 'sb'

    def __init__(self, val=None, size=None):
        super(sbit, self).__init__('sb', val=val, size=size)

    @set_instruction_type
    @vectorize
    def reveal(self):
        res = self.clear_type()
        opensbit(res, self)
        return res

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address to the sint x
              x.load_mem(address)
        """
        return NotImplemented  # cls._load_mem(addres  s, ldsint, ldsint)

    # TODO: when opcodes for inter-types these calls should be made by secret_op

    def __xor__(self, other):
        res = sbit()
        if isinstance(other, type(self)):
            xorsb(res, self, other)
        return res

    def __and__(self, other):
        res = sbit()
        if isinstance(other, type(self)):
            andsb(res, self, other)
        elif isinstance(other, _secretInt):
            res = other & self
        return res

    def __or__(self, other):
        res = sbit()
        if isinstance(other, type(self)):
            orsb(res, self, other)
        return res

    # There are two ways of doing negation, neg and invert
    def __invert__(self):
        res = sbit()
        negb(res, self)
        return res

    def __neg__(self):
        res = sbit()
        negb(res, self)
        return res

##
# Base 2^64 secret int type
# It uses the new opcodes from mod 2^n arithmetic
class sregint(_secretInt):
    """ Shared  mod 2^64 integer type. """
    __slots__ = []
    instruction_type = 'mod2n'
    clear_type = regint
    # This is the other secret type it can convert to
    secret_type = _secretModp
    reg_type = 'sr'

    def __init__(self, val=None, size=None):
        super(sregint, self).__init__('sr', val=val, size=size)

    # this method should use mem methods instead
    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address to the sregint x
              x.load_mem(address)
        """
        return cls._load_mem(address, ldmsint, ldmsinti)

    def __neg__(self):
        res = sregint()
        neg(res, self)
        return res

    def store_in_mem(self, address):
        r"""Stores the sregint value x to the address
              x.store_in_mem(address)
        """
        self._store_in_mem(address, stmsint, stmsinti)

    def mul_2_sint(self, other):
        r"""
        calculates the product of 2 sregints mod 128 bits
        :param other: sregint 64 bits
        :return: 2 sregints 64 bits
        """
        msw = sregint()
        lsw = sregint()
        if isinstance(other, sregint):
            mul2sint(msw, lsw, self, other)
        return msw, lsw

    def eqz(self):
        res = sbit()
        eqzsint(res, self)
        return res

    def ltz(self):
        res = sbit()
        ltzsint(res, self)
        return res


    # comparison implementation for new opcodes
    def __lt__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            other_local = self-other_local;
            ltzsint(res, other_local)
        return res

    def __gt__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            other_local=other_local-self;
            ltzsint(res, other_local)
        return res

    def __eq__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            other_local = self-other_local;
            eqzsint(res, other_local)
        return res

    def __le__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            res = -(self > other_local)
        return res

    def __ge__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            res = -(self < other_local)
        return res

    def __ne__(self, other):
        res = sbit()
        other_local = parse_sregint(other)
        if isinstance(other_local, type(self)):
            res = -(self == other_local)
        return res

    # bitwise operations over sregint
    def and_op(self, other):
        r"""
        returns the self & other, and secret, and mix, and int
        """
        res = sregint()
        if isinstance(other, sbit):
            sand(res, self, other)
        else:
            res = self.secret_op(other, andsint, andsintc, andsintc)
        return res

    def or_op(self, other):
        r"""
        returns the self | other, or secret, or mix, or int
        """
        return self.secret_op(other, orsint, orsintc, orsintc)

    def xor_op (self, other):
        r"""
        returns the self ^ other, or secret, or mix, or int
        """
        return self.secret_op(other, xorsint, xorsintc, xorsintc)

    # and operations
    def __and__(self, other):
        return self.and_op(other)

    # or operations
    def __or__(self, other):
        return self.or_op(other)

    # xor operations
    def __xor__(self, other):
        return self.xor_op(other)

    __rand__ = __and__
    __rxor__ = __xor__
    __ror__ = __or__

    # inversion
    def __invert__(self):
        res = sregint()
        invsint(res, self)
        return res


##
# secret integer type mod p
class sint(_secretModp, _int):
    """ Shared mod p integer type. """
    __slots__ = []
    instruction_type = 'modp'
    clear_type = cint
    # This is the other secret type it can convert to
    secret_type = _secretInt
    reg_type = 's'

    @vectorized_classmethod
    def get_random_int(cls, bits):
        r"""
          Usage
               a=sint.get_private_input_from(n)
          assigns a to be a random integer in the range [0..,2^n]
        """
        res = sint()
        comparison.PRandInt(res, bits)
        return res

    @classmethod
    def get_private_input_from(cls, player, channel=0):
        r"""
          Usage
               a=sint.get_private_input_from(p,c)
          obtains a from player p using IO channel c
        """
        res = cls()
        private_input(res, player, channel)
        return res

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address to the sint x
              x.load_mem(address)
        """
        return cls._load_mem(address, ldms, ldmsi)

    def store_in_mem(self, address):
        r"""Stores the sint value x to the address
              x.store_in_mem(address)
        """
        self._store_in_mem(address, stms, stmsi)

    def __init__(self, val=None, size=None):
        super(sint, self).__init__('s', val=val, size=size)

    @vectorize
    def __neg__(self):
        return 0 - self

    @read_mem_value
    @vectorize
    def __lt__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other > self

        res = sint()
        comparison.LTZ(res, self - other, bit_length or program.bit_length + 1,
                       security or program.security)
        return res

    @read_mem_value
    @vectorize
    def __gt__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other < self

        res = sint()
        comparison.LTZ(res, other - self, bit_length or program.bit_length + 1,
                       security or program.security)
        return res

    def __le__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other >= self
        return 1 - self.greater_than(other, bit_length, security)

    def __ge__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other <= self

        return 1 - self.less_than(other, bit_length, security)

    @read_mem_value
    @vectorize
    def __eq__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other == self
        return floatingpoint.EQZ(self - other, bit_length or program.bit_length,
                                 security or program.security)

    def __ne__(self, other, bit_length=None, security=None):
        if isinstance(other, (cfloat, sfloat)):
            return other != self
        return 1 - self.equal(other, bit_length, security)

    less_than = __lt__
    greater_than = __gt__
    less_equal = __le__
    greater_equal = __ge__
    equal = __eq__
    not_equal = __ne__

    @vectorize
    def __mod__(self, modulus):
        if isinstance(modulus, (int, long)):
            l = math.log(modulus, 2)
            if 2 ** int(round(l)) == modulus:
                return self.mod2m(int(l))
        raise NotImplementedError('Modulo only implemented for powers of two.')

    @read_mem_value
    def mod2m(self, m, bit_length=None, security=None, signed=True):
        bit_length = bit_length or program.bit_length
        security = security or program.security
        if isinstance(m, int):
            if m == 0:
                return 0
            if m >= bit_length:
                return self
            res = sint()
            if m == 1:
                comparison.Mod2(res, self, bit_length, security, signed)
            else:
                comparison.Mod2m(res, self, bit_length, m, security, signed)
        else:
            res, pow2 = floatingpoint.Trunc(self, bit_length, m, security, True)
        return res

    @vectorize
    def __rpow__(self, base):
        if isinstance(base, (int,int)):
            if base == 2:
                return self.pow2()
            else:
                raise NotImplementedError('Modulo only implemented for powers of two.')
        else:
            raise NotImplementedError('Modulo only implemented for integer types.')

    def pow2(self, bit_length=None, security=None):

        return floatingpoint.Pow2(self, bit_length or program.bit_length, \
                                  security or program.security)

    def __lshift__(self, other):
        return self * 2 ** other

    @vectorize
    @read_mem_value
    def __rshift__(self, other, bit_length=None, security=None):
        bit_length = bit_length or program.bit_length
        security = security or program.security
        if isinstance(other, int):
            if other == 0:
                return self
            res = sint()
            comparison.Trunc(res, self, bit_length, other, security, True)
            return res
        elif isinstance(other, sint):
            return floatingpoint.Trunc(self, bit_length, other, security)
        else:
            return floatingpoint.Trunc(self, bit_length, sint(other), security)

    right_shift = __rshift__

    def __rlshift__(self, other):
        return other * 2 ** self

    @vectorize
    def __rrshift__(self, other):
        return floatingpoint.Trunc(other, program.bit_length, self, program.security)

    def bit_decompose(self, bit_length=None, security=None):
        if bit_length == 0:
            return []
        bit_length = bit_length or program.bit_length
        security = security or program.security
        return floatingpoint.BitDec(self, bit_length, bit_length, security)


sint.bit_type = sint
sint.basic_type = sint


# converts type sregint depending on the case
# @param other: input of  any of supported scalar, clear types
# @return sregint cast input.
def parse_sregint(other):
    if isinstance(other, regint):
        return sregint(other)
    if isinstance(other, regint.scalars):
        return sregint(other)
    else:
        return other

# converts type to cfix/sfix depending on the case
# @param other: input of  any of supported scalar, clear and secret types
# @return cfix/sfix cast input.
def parse_type(other):
    if isinstance(other, cfix.scalars):
        return cfix(other)
    elif isinstance(other, cint):
        tmp = cfix(0)
        tmp.load_int(other)
        return tmp
    elif isinstance(other, sint):
        tmp = sfix(0)
        tmp.load_int(other)
        return tmp
    elif isinstance(other, sfloat):
        tmp = sfix(other)
        return tmp
    else:
        return other


##
# parses any input into some float type
# depending on whether the input was either
# a scalar, a clear register or a secret shared register
#
# @param other: input of  any of supported scalar, clear and secret types
# @return cfloat/sfloat cast input.
def parse_float(other):
    # converts type to cfloat/sfloat depending on the case
    if isinstance(other, cfloat.scalars):
        return cfloat(other)
    elif isinstance(other, cfloat.clears):
        return cfloat(other)
    elif isinstance(other, cfloat.secrets):
        return sfloat(other)
    elif isinstance(other, (cfloat, sfloat)):
        return other
    else:
        raise CompilerError('Missmatching input type')

##
# clear fixed point class based on mod p sint.
class cfix(_number):
    """ Clear fixed point type. """
    __slots__ = ['value', 'f', 'k', 'size']
    reg_type = 'c'
    scalars = (int, long, float, regint)

    @classmethod
    def set_precision(cls, f, k=None):
        r"""Defines k as the bitlength of the fixed point system and f as the bitlength of the decimal part
        So a cfix x is held as an integer y in [0,...,2^k-1] and
                    x = y/2^f
        """
        # k is the whole bitlength of fixed point
        # f is the bitlength of decimal part
        cls.f = f
        if k is None:
            cls.k = 2 * f
        else:
            cls.k = k

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address of the cint memory to the cfix x
              x.load_mem(address)
        """
        res = []
        res.append(cint.load_mem(address))
        return cfix(*res)

    @vectorize_init
    def __init__(self, v, size=None):
        self.f = cfix.f
        self.k = cfix.k
        f = self.f
        k = self.k
        if size is None:
            self.size = get_global_vector_size()
        else:
            self.size = size

        if isinstance(v, cint):
            self.v = cint(v, size=self.size)
        elif isinstance(v, cfix.scalars):
            v = v * (2 ** f)
            try:
                v = int(round(v))
            except TypeError:
                pass
            self.v = cint(v, size=self.size)
        elif isinstance(v, cfix):
            self.v = v.v
        elif isinstance(v, MemValue):
            self.v = v
        else:
            raise NotImplementedError
        program.curr_tape.require_bit_length(2*self.k+1)

    @vectorize
    def load_int(self, v):
        r"""Converts the integer v to a cfix value, basically multiplies v by 2^f """
        self.v = cint(v) * (2 ** self.f)

    @classmethod
    def conv(cls, other):
        if isinstance(other, MemValue):
            other = other.read()
        if isinstance(other, cls):
            return other
        else:
            try:
                res = cfix()
                res.load_int(other)
                return res
            except (TypeError, CompilerError):
                pass
        return cls(other)

    def store_in_mem(self, address):
        r"""Stores the cfix value x to the cint memory with address
              x.store_in_mem(address)
        """
        self.v.store_in_mem(address)

    def sizeof(self):
        return self.size * 1

    @vectorize
    def add(self, other):
        if isinstance(other, sfloat):
            return other + self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return cfix(self.v + other.v)
            elif isinstance(other, sfix):
                return sfix(self.v + other.v)
            else:
                raise CompilerError('Invalid type %s for cfix.__add__' % type(other))

    @vectorize
    def mul(self, other):
        if isinstance(other, sfloat):
            return other * self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                sgn = cint(1 - 2 * (self.v * other.v < 0))
                absolute = self.v * other.v * sgn
                val = sgn * (absolute >> self.f)
                return cfix(val)
            elif isinstance(other, sfix):
                res = sfix((self.v * other.v) >> self.f)
                return res
            else:
                raise CompilerError('Invalid type %s for cfix.__mul__' % type(other))

    @vectorize
    def __sub__(self, other):
        if isinstance(other, sfloat):
            return -other + self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return cfix(self.v - other.v)
            elif isinstance(other, sfix):
                return sfix(self.v - other.v)
            else:
                raise NotImplementedError

    @vectorize
    def __neg__(self):
        # cfix type always has .v
        return cfix(-self.v)

    def __rsub__(self, other):
        return -self + other

    @vectorize
    def __eq__(self, other):
        """ parses all types to  fix registers and performs test.
        in case is performed against a sfloat use equality test from sfloat
        """
        if isinstance(other, sfloat):
            return other == self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v == other.v
            elif isinstance(other, sfix):
                return other.v.equal(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __lt__(self, other):
        """ parses all types to  fix registers and performs test.
        in case is performed against a sfloat use inequality test from sfloat
        """

        if isinstance(other, sfloat):
            return other > self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v < other.v
            elif isinstance(other, sfix):
                if (self.k != other.k or self.f != other.f):
                    raise TypeError('Incompatible fixed point types in comparison')
                return other.v.greater_than(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __le__(self, other):
        """ parses all types to  fix registers and performs test.
        in case is performed against a sfloat use inequality test from sfloat
        """

        if isinstance(other, sfloat):
            return other >= self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v <= other.v
            elif isinstance(other, sfix):
                return other.v.greater_equal(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __gt__(self, other):
        """ parses all types to  fix registers and performs test.
        in case is performed against a sfloat use inequality test from sfloat
        """
        if isinstance(other, sfloat):
            return other <= self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v > other.v
            elif isinstance(other, sfix):
                return other.v.less_than(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __ge__(self, other):
        """ parses all types to  fix registers and performs test.
        in case is performed against a sfloat use inequality test from sfloat
        """
        if isinstance(other, sfloat):
            return other < self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v >= other.v
            elif isinstance(other, sfix):
                return other.v.less_equal(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __ne__(self, other):
        if isinstance(other, sfloat):
            return other != self
        else:
            other = parse_type(other)
            if isinstance(other, cfix):
                return self.v != other.v
            elif isinstance(other, sfix):
                return other.v.not_equal(self.v, self.k, other.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __div__(self, other):
        other = parse_type(other)
        if isinstance(other, cfix):
            return cfix(library.cint_cint_division(self.v, other.v, self.k, self.f))
        elif isinstance(other, sfix):
            return sfix(library.FPDiv(self.v, other.v, self.k, self.f, other.kappa))
        else:
            raise TypeError('Incompatible fixed point types in division')

    @vectorize
    def print_fix(self):
        r"""Prints the cfix in as a vector (y,f,k)"""
        print_fix(self.v, self.f, self.k)


##
# secret fixed point class based on mod p sint.
class sfix(_number):
    """ Shared fixed point type. """
    __slots__ = ['v', 'f', 'k', 'size']
    reg_type = 's'
    kappa = 40

    @classmethod
    def set_precision(cls, f, k=None):
        cls.f = f
        # default bitlength = 2*precision
        if k is None:
            cls.k = 2 * f
        else:
            cls.k = k

    @classmethod
    def conv(cls, other):
        if isinstance(other, MemValue):
            other = other.read()
        if isinstance(other, cls):
            return other
        else:
            try:
                res = sfix()
                sfix.load_int(other)
                return res
            except (TypeError, CompilerError):
                pass
        return cls(other)

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address of the sint memory to the sfix x
              x.load_mem(address)
        """
        res = []
        res.append(sint.load_mem(address))
        return sfix(*res)

    @classmethod
    def load_sint(cls, v):
        res = cls()
        res.load_int(v)
        return res

    @vectorize_init
    def __init__(self, _v=None, size=None):
        if size is None:
            self.size = get_global_vector_size()
        else:
            self.size = size

        f = self.f
        k = self.k
        # warning: don't initialize a sfix from a sint, this is only used in internal methods;
        # for external initialization use load_int.
        if isinstance(_v, sint):
            self.v = _v
        elif isinstance(_v, cfix.scalars):
            v = _v * (2 ** f)
            try:
                v = int(round(v))
            except TypeError:
                pass
            self.v = sint(v, size=self.size)
        elif isinstance(_v, sfloat):
            p = (f + _v.p)
            b = (p >= 0)
            a = b * (_v.v << (p)) + (1 - b) * (_v.v >> (-p))
            self.v = (1 - 2 * _v.s) * a
        elif isinstance(_v, sfix):
            self.v = _v.v
        elif isinstance(_v, MemFix):
            # this is a memvalue object
            self.v = _v.v
        elif isinstance(_v, regint):
            self.v = sint(_v, size=self.size) * 2 ** f
        self.kappa = sfix.kappa
        program.curr_tape.require_bit_length(2*self.k+self.kappa+1)

    @vectorize
    def load_int(self, v):
        self.v = sint(v) * (2 ** self.f)

    def store_in_mem(self, address):
        r"""Stores the sfix value x to the sint memory with address
              x.store_in_mem(address)
        """
        self.v.store_in_mem(address)

    ##
    # returns the number of registers being stored.
    # given that only parameter v is stored
    # the function returns the vector size.
    # @return number of registers  engaged in memory.
    def sizeof(self):

        return self.size * 1

    @vectorize
    def add(self, other):
        if isinstance(other, sfloat):
            return other + self
        else:
            other = parse_type(other)
            if isinstance(other, (sfix, cfix)):
                return sfix(self.v + other.v)
            elif isinstance(other, cfix.scalars):
                tmp = cfix(other)
                return self + tmp
            else:
                raise CompilerError('Invalid type %s for sfix.__add__' % type(other))

    @vectorize
    def mul(self, other):
        if isinstance(other, sfloat):
            return other * self
        else:
            other = parse_type(other)
            if isinstance(other, (sfix, cfix)):
                val = floatingpoint.TruncPr(self.v * other.v, self.k * 2, self.f, self.kappa)
                return sfix(val)
            elif isinstance(other, cfix.scalars):
                scalar_fix = cfix(other)
                return self * scalar_fix
            else:
                raise CompilerError('Invalid type %s for sfix.__mul__' % type(other))

    @vectorize
    def __sub__(self, other):
        if isinstance(other, sfloat):
            return (-other) + self
        else:
            other = parse_type(other)
            return self + (-other)

    @vectorize
    def __neg__(self):
        return sfix(-self.v)

    def __rsub__(self, other):
        return -(self) + other

    @vectorize
    def __eq__(self, other):
        if isinstance(other, sfloat):
            return other == self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.equal(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __le__(self, other):
        if isinstance(other, sfloat):
            return other >= self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.less_equal(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __lt__(self, other):
        if isinstance(other, sfloat):
            return other < self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.less_than(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __ge__(self, other):
        if isinstance(other, sfloat):
            return other <= self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.greater_equal(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __gt__(self, other):
        if isinstance(other, sfloat):
            return other < self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.greater_than(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __ne__(self, other):
        if isinstance(other, sfloat):
            return other != self
        else:
            other = parse_type(other)
            if isinstance(other, (cfix, sfix)):
                return self.v.not_equal(other.v, self.k, self.kappa)
            else:
                raise NotImplementedError

    @vectorize
    def __div__(self, other):
        other = parse_type(other)
        if isinstance(other, sfix):
            return sfix(library.FPDiv(self.v, other.v, self.k, self.f, self.kappa))
        elif isinstance(other, cfix):
            return sfix(library.sint_cint_division(self.v, other.v, self.k, self.f, self.kappa))
        else:
            raise TypeError('Incompatible fixed point types in division')

    @vectorize
    def compute_reciprocal(self):
        return sfix(library.FPDiv(cint(2) ** self.f, self.v, self.k, self.f, self.kappa, True))

    def reveal(self):
        val = self.v.reveal()
        return cfix(val, size=self.size)


# this is for 20 bit decimal precision
# with 40 bitlength of entire number
# these constants have been chosen for multiplications to fit in 128 bit prime field
# (precision n1) 41 + (precision n2) 41 + (stat_sec) 40 = 82 + 40 = 122 <= 128
# with statistical security of 40
fixed_lower = 20
fixed_upper = 41
sfix.set_precision(fixed_lower, fixed_upper)
cfix.set_precision(fixed_lower, fixed_upper)


##
# clear floating point class based on mod p sint.
class sfloat(_number):
    """ Shared floating point data type, representing (1 - 2s)*(1 - z)*v*2^p.
        v: significand
        p: exponent
        z: zero flag
        s: sign bit
        err: error flag
        """
    __slots__ = ['v', 'p', 'z', 's', 'err', 'size']
    # single precision
    vlen = 24
    plen = 8
    kappa = 40
    round_nearest = False
    error = 0

    @vectorized_classmethod
    def load_mem(cls, address, mem_type=None):
        r"""Loads the value stored in address (and the following three locations) of the sint memory to the sfloat x
              x.load_mem(address)
        """
        res = []
        for i in range(5):
            res.append(sint.load_mem(address + i * get_global_vector_size()))
        return sfloat(*res)

    @classmethod
    def set_error(cls, error):
        cls.error += error

    @classmethod
    def add_err(self, flag):
        self.err += flag

    @staticmethod
    def convert_float(v, vlen, plen):
        v, p, z, s = floatingpoint.convert_float(v, vlen, plen)
        err = sint(0)
        return v, p, z, s, err

    ##
    # Constructor that receives the basic 5 slots.
    # If it only receives v, it casts v to the float representation in use,
    # otherwise it uses the parametrization to instanciate a cfloat register.
    # @param v basic input or significand of IEEE floating point representation
    # @param p exponent. Optional parameter
    # @param z zero flag. Optional parameter
    # @param s sign. Optional parameter
    # @param size number of vectorized instances. Optional parameter
    @vectorize_init
    def __init__(self, v, p=None, z=None, s=None, err=None, size=None):
        self.size = get_global_vector_size()
        if p is None:
            if isinstance(v, sfloat):
                err = v.err
                p = v.p
                z = v.z
                s = v.s
                v = v.v
            elif isinstance(v, sint):
                v, p, z, s = floatingpoint.Int2FL(v, program.bit_length,
                                                  self.vlen, self.kappa)

                err = self.__flow_detect__(p)

            elif isinstance(v, sfix):
                f = v.f
                v, p, z, s = floatingpoint.Int2FL(v.v, v.k,
                                                  self.vlen, self.kappa)
                p = p - f
                err = self.__flow_detect__(p)

            else:
                v, p, z, s, err = self.convert_float(v, self.vlen, self.plen)

        if isinstance(v, (int,int)):
            if not ((v >= 2 ** (self.vlen - 1) and v < 2 ** (self.vlen)) or v == 0):
                raise CompilerError('Floating point number malformed: significand')
            self.v = library.load_int_to_secret(v)
        else:
            self.v = v
        if isinstance(p, int):
            if not (p >= -2 ** (self.plen - 1) and p < 2 ** (self.plen - 1)):
                raise CompilerError(
                    'Floating point number malformed: exponent %d not unsigned %d-bit integer' % (p, self.plen))
            self.p = library.load_int_to_secret(p)
        else:
            self.p = p
        if isinstance(z, int):
            if not (z == 0 or z == 1):
                raise CompilerError('Floating point number malformed: zero bit')
            self.z = sint()
            ldsi(self.z, z)
        else:
            self.z = z
        if isinstance(s, int):
            if not (s == 0 or s == 1):
                raise CompilerError('Floating point number malformed: sign')
            self.s = sint()
            ldsi(self.s, s)
        else:
            self.s = s
        if isinstance(err, int):
            if not (err >= 0):
                raise CompilerError('Floating point number malformed: err')
            self.err = library.load_int_to_secret(err)
        else:
            self.err = err
        program.curr_tape.require_bit_length(2*self.vlen+self.kappa+1)

    def __iter__(self):
        yield self.v
        yield self.p
        yield self.z
        yield self.s
        yield self.err

    def store_in_mem(self, address):
        r"""Stores the sfloat value x to the sint memory with address (and the following three locations)
              x.store_in_mem(address)
        """
        for i, x in enumerate((self.v, self.p, self.z, self.s)):
            x.store_in_mem(address + i * get_global_vector_size())

    ##
    # returns the number of registers being stored.
    # given that the parameters v, p, s, z, err are stored
    # in memory, the function returns the vector size times 5.
    # @return number of registers engaged in memory.
    def sizeof(self):
        return self.size * 5

    ##
    # realizes the addition protocol for several different types.
    # @param other: value adding to self, could be any type
    # @return sloat: new sfloat instance
    @vectorize
    def add(self, other):

        if isinstance(other, (cfloat, sfloat)):
            a, c, d, e = [sint() for i in range(4)]
            t = sint()
            t2 = sint()
            v1 = self.v
            v2 = other.v
            p1 = self.p
            p2 = other.p
            s1 = self.s
            s2 = other.s
            z1 = self.z
            z2 = other.z
            a = p1.less_than(p2, self.plen, self.kappa)
            b = floatingpoint.EQZ(p1 - p2, self.plen, self.kappa)
            c = v1.less_than(v2, self.vlen, self.kappa)
            ap1 = a * p1
            ap2 = a * p2
            aneg = 1 - a
            bneg = 1 - b
            cneg = 1 - c
            av1 = a * v1
            av2 = a * v2
            cv1 = c * v1
            cv2 = c * v2
            pmax = ap2 + p1 - ap1
            pmin = p2 - ap2 + ap1
            vmax = bneg * (av2 + v1 - av1) + b * (cv2 + v1 - cv1)
            vmin = bneg * (av1 + v2 - av2) + b * (cv1 + v2 - cv2)
            s3 = s1 + s2 - 2 * s1 * s2
            comparison.LTZ(d, self.vlen + pmin - pmax + sfloat.round_nearest,
                           self.plen, self.kappa)
            pow_delta = floatingpoint.Pow2((1 - d) * (pmax - pmin),
                                           self.vlen + 1 + sfloat.round_nearest,
                                           self.kappa)
            # deviate from paper for more precision
            # v3 = 2 * (vmax - s3) + 1
            v3 = vmax
            v4 = vmax * pow_delta + (1 - 2 * s3) * vmin
            v = (d * v3 + (1 - d) * v4) * two_power(self.vlen + sfloat.round_nearest) \
                * floatingpoint.Inv(pow_delta)
            comparison.Trunc(t, v, 2 * self.vlen + 1 + sfloat.round_nearest,
                             self.vlen - 1, self.kappa, False)
            v = t
            u = floatingpoint.BitDec(v, self.vlen + 2 + sfloat.round_nearest,
                                     self.vlen + 2 + sfloat.round_nearest, self.kappa,
                                     list(range(1 + sfloat.round_nearest,
                                           self.vlen + 2 + sfloat.round_nearest)))
            # using u[0] doesn't seem necessary
            h = floatingpoint.PreOR(u[:sfloat.round_nearest:-1], self.kappa)
            p0 = self.vlen + 1 - sum(h)
            pow_p0 = 1 + sum([two_power(i) * (1 - h[i]) for i in range(len(h))])
            if self.round_nearest:
                t2, overflow = \
                    floatingpoint.TruncRoundNearestAdjustOverflow(pow_p0 * v,
                                                                  self.vlen + 3,
                                                                  self.vlen,
                                                                  self.kappa)
                p0 = p0 - overflow
            else:
                comparison.Trunc(t2, pow_p0 * v, self.vlen + 2, 2, self.kappa, False)
            v = t2
            # deviate for more precision
            # p = pmax - p0 + 1 - d
            p = pmax - p0 + 1
            zz = self.z * other.z
            zprod = 1 - self.z - other.z + zz
            v = zprod * t2 + self.z * v2 + other.z * v1
            z = floatingpoint.EQZ(v, self.vlen, self.kappa)
            p = (zprod * p + self.z * p2 + other.z * p1) * (1 - z)
            s = (1 - b) * (a * other.s + aneg * self.s) + b * (c * other.s + cneg * self.s)
            s = zprod * s + (other.z - zz) * self.s + (self.z - zz) * other.s
            err = sint(0)
            if (isinstance(other, sfloat)):
                err = err + other.err
            err = err + self.err
            err = err + self.__flow_detect__(p)
            return sfloat(v, p, z, s, err)
        # in case is not a register
        else:
            other_parse = parse_float(other)
            return self + other_parse

    ##
    # realizes the multiplication protocol for several different types.
    # @param other: value multiplying self, could be any type
    # @return sloat: new sfloat instance
    @vectorize
    def mul(self, other):

        if isinstance(other, (cfloat, sfloat)):
            # return sint(-1)
            v1 = sint()
            v2 = sint()
            b = sint()
            c2expl = cint()
            comparison.ld2i(c2expl, self.vlen)
            if sfloat.round_nearest:
                v1 = comparison.TruncRoundNearest(self.v * other.v, 2 * self.vlen,
                                                  self.vlen - 1, self.kappa)
            else:
                comparison.Trunc(v1, self.v * other.v, 2 * self.vlen, self.vlen - 1, self.kappa, False)
            t = v1 - c2expl
            comparison.LTZ(b, t, self.vlen + 1, self.kappa)
            comparison.Trunc(v2, b * v1 + v1, self.vlen + 1, 1, self.kappa, False)
            z = self.z + other.z - self.z * other.z  # = OR(z1, z2)
            s = self.s + other.s - 2 * self.s * other.s  # = XOR(s1,s2)
            p = (self.p + other.p - b + self.vlen) * (1 - z)
            err = sint(0)
            if isinstance(other, sfloat):
                err = other.err
            err = err + self.err
            err = err + self.__flow_detect__(p)
            return sfloat(v2, p, z, s, err)

        # in case is not a register
        else:
            other_parse = parse_float(other)
            return self * other_parse  # self.mul(scalar_float)

    ##
    # float division (FLDiv) as described in ABZS12.
    # Additional conditions for err added in algiment with
    # the way we work with floating point numbers.
    def local_division_ABZS12(self, other):
        if isinstance(other, (cfloat, sfloat)):
            l = self.vlen
            v = floatingpoint.SDiv_ABZS12(self.v, other.v + other.z, l, self.kappa)
            b = v.less_than(2 ** l, l + 1, self.kappa)
            v = floatingpoint.Trunc(v * b + v, l + 1, 1, self.kappa)
            p = (1 - self.z) * (self.p - other.p - l + 1 - b)
            z = self.z
            
            #simple xor of sign
            s = self.s + other.s - 2*self.s * other.s
            if isinstance(other, sfloat):
                err = other.err
            err = err + self.err
            err = err + self.__flow_detect__(p)
            err = err + other.z
            return sfloat(v, p, z, s, err)
        else:

            other_parse = parse_float(other)
            return old_div(self, other_parse)

    ##
    # realizes the division protocol for several different types.
    # @param other: value dividing self, could be any type
    # @return sloat: new sfloat instance   
    def local_division(self, other):
        if isinstance(other, (cfloat, sfloat)):
            v = floatingpoint.SDiv(self.v, other.v + other.z * (2 ** self.vlen - 1),
                                   self.vlen, self.kappa)
            b = v.less_than(two_power(self.vlen - 1), self.vlen + 1, self.kappa)
            overflow = v.greater_equal(two_power(self.vlen), self.vlen + 1, self.kappa)
            underflow = v.less_than(two_power(self.vlen - 2), self.vlen + 1, self.kappa)
            v = (v + b * v) * (1 - overflow) * (1 - underflow) + \
                overflow * (2 ** self.vlen - 1) + \
                underflow * (2 ** (self.vlen - 1)) * (1 - self.z)
            p = (1 - self.z) * (self.p - other.p - self.vlen - b + 1)
            z = self.z
            s = self.s + other.s - 2 * self.s * other.s
            # self.add_err(other.z)

            # error management
            if isinstance(other, sfloat):
                err = other.err
            err = err + self.err
            err = err + self.__flow_detect__(p)
            err = err + other.z
            return sfloat(v, p, z, s, err)

        else:

            other_parse = parse_float(other)
            return old_div(self, other_parse)


    def __sub__(self, other):
        return (self + -other)

    def __rsub__(self, other):
        return -1 * self + other

    def __div__(self, other):
        return self.local_division_ABZS12(other)

    @vectorize
    def __neg__(self):
        return sfloat(self.v, self.p, self.z, (1 - self.s) * (1 - self.z), self.err)

    ##
    # realizes the less than thest protocol for several different types.
    # Basic Method, should resolve directly < operator.
    # this is the base for all comparison operations
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    @vectorize
    def __lt__(self, other):
        if isinstance(other, (cfloat, sfloat)):
            return floatingpoint.FLLT(self, other)
        else:
            other_parse = parse_float(other)
            return self < other_parse

    ##
    # realizes the greater than  protocol for several different types.
    # Basic Method, should resolve directly > operator.
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    def __gt__(self, other):
        # return floatingpoint.FLLT(other, self)
        return (other - self) < 0  # floatingpoint.FLLTZ(other - self)

    ##
    # realizes the less equal  protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    def __le__(self, other):
        return 1 - (self > other)

    ##
    # realizes the great than  protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    def __ge__(self, other):
        return 1 - (self < other)

    ##
    # realizes the equality test protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    @vectorize
    def __eq__(self, other):
        if isinstance(other, (cfloat, sfloat)):
            t = self.err
            if isinstance(other, sfloat):
                t = t + other.err
            t = t == 0
            # the sign can be both ways for zeroes
            both_zero = self.z * other.z
            return (floatingpoint.EQZ(self.v - other.v, self.vlen, self.kappa) * \
                    floatingpoint.EQZ(self.p - other.p, self.plen, self.kappa) * \
                    (1 - self.s - other.s + 2 * self.s * other.s) * \
                    (1 - both_zero) + both_zero) * t
        else:
            other_parse = parse_float(other)
            return self == other_parse

    ##
    # realizes the  not equal  protocol for several different types.
    # Basic Method, should resolve directly > operator.
    # @param other: value comparing self, could be any type
    # @return sint: new sint bitwise instance
    def __ne__(self, other):
        return 1 - (self == other)

    def value(self):
        """ Gets actual floating point value, if emulation is enabled. """
        return (1 - 2 * self.s.value) * (1 - self.z.value) * self.v.value / float(2 ** self.p.value)

    ##
    # reveals instance as a floating point number, by creating an instance of cfloat.
    # in case there was an error during the circuit execution, it returns 0 in all
    # state elements of the instance.
    # @return a cfloat value with the corresponding state elements in plain text.
    def reveal(self):
        """ Reveals instance as a floating point number, by creating an instance of cfloat.
        in case there was an error during the circuit execution, it returns 0 in all
    	state elements of the instance.
    	"""
        signal = self.err == 0
        return cfloat((self.v * signal).reveal(), (self.p * signal).reveal(), (self.z * signal).reveal(),
                      (self.s * signal).reveal())

    ##
    # detects overflow by or underflow. Implementation saves one multiplication,
    # by simply  calculating the |p|> 2^{k-1}
    # @param p  pre-calculated secret shared exponent p
    # @return secret shared \{0,1\} flag. Returns 1 if p under or over flows, 0 otherwise
    def __flow_detect__(self, p):
        """ detects overflow by or underflow. Implementation saves one multiplication,
            by simply  calculating the |p|> 2^{k-1}"""
        if (program.fdflag):
            s = (p < 0) * (-2) + 1
            return (s * p) >= (2 ** (self.plen - 1))
        else:
            return sint(0)


##
# clear fixed point class based on mod p sint.
class cfloat(object):
    """
    Helper class used for printing sfloats
    and to perform operations with clear and secret registers
    open register floating point data type, representing (1 - 2s)*(1 - z)*v*2^p.

        v: significand
        p: exponent
        z: zero flag
        s: sign bit
    """
    __slots__ = ['v', 'p', 'z', 's', 'size']

    scalars = (int, long, float)
    clears = (cfix, cint)
    secrets = (sfix, sint)

    # single precision
    vlen = 24
    plen = 8
    kappa = 40
    round_nearest = False
    error = 0

    ##
    # Constructor that receives the basic 4 slots.
    # If it only receives v, it casts v to the float representation in use,
    # otherwise it uses the parametrization to instanciate a cfloat register.
    # @param v basic input or significand of IEEE floating point representation
    # @param p exponent. Optional parameter
    # @param z zero flag. Optional parameter
    # @param s sign. Optional parameter
    # @param size number of vectorized instances. Optional parameter
    @vectorize_init
    def __init__(self, v, p=None, z=None, s=None, size=None):
        self.size = get_global_vector_size()
        if p is None:
            # copy instance
            if isinstance(v, cfloat):
                p = v.p
                z = v.z
                s = v.s
                v = v.v
            elif isinstance(v, cfloat.clears):
                # something like this should be done for fix:
                # raise CompilerError('Unsupported operation for clear registries')
                v_clear = parse_type(v)
                f = v_clear.f
                v, p, z, s = library.int2FL_plain(v_clear.v, program.bit_length, self.vlen, self.kappa)
                p = p - f
                # v, p, z, s = library.int2FL_plain(v, program.bit_length, self.vlen, self.kappa)

            # instantiate v, p z, s as int
            elif isinstance(v, cfloat.scalars):
                # convert float verfies p < 2** vlen, which is then done again here.
                # this is for legacy reasons, the method is a geacy method embedded in sfloat
                v, p, z, s = floatingpoint.convert_float(v, self.vlen, self.plen)

            else:  # missmatch of types validation
                raise CompilerError('Missmatching input type')

        # validation of v
        if isinstance(v, int):
            if not ((v >= 2 ** (self.vlen - 1) and v < 2 ** (self.vlen)) or v == 0):
                raise CompilerError('Floating point number malformed: significand')
            self.v = cint(v)
        elif isinstance(v, cint):
            self.v = v
        else:  # missmatch of types validation
            raise CompilerError('Missmatching input type ')
        program.curr_tape.require_bit_length(2*self.vlen+1)

        # validation of p
        if isinstance(p, int):
            if not (p >= -2 ** (self.plen - 1) and p < 2 ** (self.plen - 1)):
                raise CompilerError(
                    'Floating point number malformed: exponent %d not unsigned %d-bit integer' % (p, self.plen))
            self.p = cint(p)
        elif isinstance(p, cint):
            self.p = p
        else:  # missmatch of types validation
            raise CompilerError('Missmatching input type')

        # validation of z
        if isinstance(z, int):
            if not (z == 0 or z == 1):
                raise CompilerError('Floating point number malformed: zero bit')
            self.z = cint(0)
            if (z == 1):
                self.set_zero(z)
                self.z = cint(1)
        elif isinstance(z, cint):
            self.z = z
        else:  # missmatch of types validation
            raise CompilerError('Missmatching input type')

        # validation of s
        if isinstance(s, int):
            if not (s == 0 or s == 1):
                self.set_zero(1)
                raise CompilerError('Floating point number malformed: sign')
            self.s = cint(s)
        elif isinstance(s, cint):
            self.s = s
        else:  # missmatch of types validation
            raise CompilerError('Missmatching input type')

    ##
    # @private
    # sets records to zero
    # @param flag: whether or not it has to set up records to 0
    def set_zero(self, flag):
        if (flag == 1):
            self.v = cint(0)
            self.p = cint(0)
            self.s = cint(0)
            self.z = cint(0)

    ##
    # facade method that evokes low level instructions
    # to print float number.
    # No params, uses instance records.
    def print_float(self):
        print_float(self.v, self.p, self.z, self.s)

    ##
    # computes the product times -1 of the cfloat
    # @return:  new cfloat instance of the negation of input
    @vectorize
    def __neg__(self):
        return cfloat(self.v, self.p, self.z, (1 - self.s) * (1 - self.z))

    ##
    # returns the number of registers being stored.
    # given that the parameters v, p, s, z are stored
    # in memory, the function returns the vector size times 4.
    # @return number of registers engaged in memory.
    def sizeof(self):
        return self.size * 4

    ##
    # realizes the less than protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new cint bitwise instance
    def __lt__(self, other):
        if (isinstance(other, sfloat)):
            return other > self
        raise NotImplemented

    ##
    # realizes the great than protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new cint bitwise instance
    def __gt__(self, other):
        if (isinstance(other, sfloat)):
            return other < self
        raise NotImplemented

    ##
    # realizes the less equal protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new cint bitwise instance
    def __le__(self, other):
        if (isinstance(other, sfloat)):
            return other >= self
        raise NotImplemented

    ##
    # realizes the great equal protocol for several different types.
    # @param other: value comparing self, could be any type
    # @return sint: new cint bitwise instance
    def __ge__(self, other):
        if (isinstance(other, sfloat)):
            return other <= self
        raise NotImplemented


_types = {
    'c': cint,
    's': sint,
    'r': regint,
    'sr': sregint,
    'sb': sbit
}


class Array(object):
    """ Array objects """

    def __init__(self, length, value_type, address=None):
        if value_type in _types:
            value_type = _types[value_type]
        self.address = address
        self.length = length
        self.value_type = value_type
        if address is None:
            self.address = self._malloc()

    def _malloc(self):
        return program.malloc(self.length, self.value_type)

    def delete(self):
        if program:
            program.free(self.address, self.value_type.reg_type)

    def get_address(self, index):
        if isinstance(index, int) and self.length is not None:
            index += self.length * (index < 0)
            if index >= self.length or index < 0:
                raise IndexError('index %s, length %s' % \
                                 (str(index), str(self.length)))
        return self.address + index

    def get_slice(self, index):
        if index.stop is None and self.length is None:
            raise CompilerError('Cannot slice array of unknown length')
        return index.start or 0, index.stop or self.length, index.step or 1

    def __getitem__(self, index):
        if isinstance(index, slice):
            start, stop, step = self.get_slice(index)
            res_length = old_div((stop - start - 1), step) + 1
            res = Array(res_length, self.value_type)

            @library.for_range(res_length)
            def f(i):
                res[i] = self[start + i * step]

            return res
        return self._load(self.get_address(index))

    def __setitem__(self, index, value):
        if isinstance(index, slice):
            start, stop, step = self.get_slice(index)
            source_index = MemValue(0)

            @library.for_range(start, stop, step)
            def f(i):
                self[i] = value[source_index]
                source_index.iadd(1)

            return
        self._store(self.value_type.conv(value), self.get_address(index))

    def _load(self, address):
        return self.value_type.load_mem(address)

    def _store(self, value, address):
        value.store_in_mem(address)

    def __len__(self):
        return self.length

    def __iter__(self):
        for i in range(self.length):
            yield self[i]

    def assign(self, other):
        if isinstance(other, Array):
            def loop(i):
                self[i] = other[i]

            library.range_loop(loop, len(self))
        elif isinstance(other, Tape.Register):
            if len(other) == self.length:
                self[0] = other
            else:
                raise CompilerError('Length mismatch between array and vector')
        else:
            for i, j in enumerate(other):
                self[i] = j
        return self

    def assign_all(self, value):
        mem_value = MemValue(value)
        n_loops = 8 if len(self) > 2 ** 20 else 1

        @library.for_range_multithread(n_loops, 1024, len(self))
        def f(i):
            self[i] = mem_value

        return self


sint.dynamic_array = Array


class Matrix(object):
    def __init__(self, rows, columns, value_type, address=None):
        self.rows = rows
        self.columns = columns
        if value_type in _types:
            value_type = _types[value_type]
        self.value_type = value_type
        if address is None:
            self.address = Array(rows * columns, value_type).address
        else:
            self.address = Array(rows * columns, value_type, address).address

    def __getitem__(self, index):
        return Array(self.columns, self.value_type, \
                     self.address + index * self.columns)

    def __len__(self):
        return self.rows

    def assign_all(self, value):
        @library.for_range(len(self))
        def f(i):
            self[i].assign_all(value)

        return self


class SubMultiArray(object):
    def __init__(self, sizes, value_type, address, index):
        self.sizes = sizes
        self.value_type = value_type
        self.address = address + index * reduce(operator.mul, self.sizes)

    def __getitem__(self, index):
        if len(self.sizes) == 2:
            return self.value_type.Array(self.sizes[1], \
                                         self.address + index * self.sizes[1])
        else:
            return SubMultiArray(self.sizes[1:], self.value_type, \
                                 self.address, index)


class MultiArray(object):
    def __init__(self, sizes, value_type):
        self.sizes = sizes
        self.value_type = value_type
        self.array = value_type.Array(reduce(operator.mul, sizes))
        if len(sizes) < 2:
            raise CompilerError('Use Array')

    def __getitem__(self, index):
        return SubMultiArray(self.sizes[1:], self.value_type, \
                             self.array.address, index)


class VectorArray(object):
    def __init__(self, length, value_type, vector_size, address=None):
        self.array = Array(length * vector_size, value_type, address)
        self.vector_size = vector_size
        self.value_type = value_type

    def __getitem__(self, index):
        return self.value_type.load_mem(self.array.address + \
                                        index * self.vector_size,
                                        size=self.vector_size)

    def __setitem__(self, index, value):
        if value.size != self.vector_size:
            raise CompilerError('vector size mismatch')
        value.store_in_mem(self.array.address + index * self.vector_size)


class sfloatArray(Array):
    def __init__(self, length, address=None):
        print(length, address)
        self.matrix = Matrix(length, 5, sint, address)
        self.length = length
        self.value_type = sfloat

    def __getitem__(self, index):
        if isinstance(index, slice):
            return Array.__getitem__(self, index)
        return sfloat(*self.matrix[index])

    def __setitem__(self, index, value):
        if isinstance(index, slice):
            return Array.__setitem__(self, index, value)
        self.matrix[index].assign(iter(sfloat(value)))


class sfloatMatrix(Matrix):
    def __init__(self, rows, columns):
        self.rows = rows
        self.columns = columns
        self.multi_array = MultiArray([rows, columns, 5], sint)

    def __getitem__(self, index):
        return sfloatArray(self.columns, self.multi_array[index].address)


class _fixArray(Array):
    def __init__(self, length, address=None):
        self.array = Array(length, self.int_type, address)
        self.address = self.array.address
        self.length = length

    def __getitem__(self, index):
        if isinstance(index, slice):
            return Array.__getitem__(self, index)
        return self.value_type(self.array[index])

    def __setitem__(self, index, value):
        if isinstance(index, slice):
            return Array.__setitem__(self, index, value)
        self.array[index] = self.value_type.conv(value).v

    def get_address(self, index):
        return self.array.get_address(index)

class cfixArray(_fixArray):
    int_type = cint
    value_type = cfix

class sfixArray(_fixArray):
    int_type = sint
    value_type = sfix

class _fixMatrix(Matrix):
    def __init__(self, rows, columns, address=None):
        self.rows = rows
        self.columns = columns
        self.multi_array = Matrix(rows, columns, self.int_type, address)

    def __getitem__(self, index):
        return self.value_type.Array(self.columns, self.multi_array[index].address)

class cfixMatrix(_fixMatrix):
    int_type = cint
    value_type = cfix

class sfixMatrix(_fixMatrix):
    int_type = sint
    value_type = sfix

class _mem(_number):
    __add__ = lambda self, other: self.read() + other
    __sub__ = lambda self, other: self.read() - other
    __mul__ = lambda self, other: self.read() * other
    __div__ = lambda self, other: old_div(self.read(), other)
    __mod__ = lambda self, other: self.read() % other
    __pow__ = lambda self, other: self.read() ** other
    __neg__ = lambda self, other: -self.read()
    __lt__ = lambda self, other: self.read() < other
    __gt__ = lambda self, other: self.read() > other
    __le__ = lambda self, other: self.read() <= other
    __ge__ = lambda self, other: self.read() >= other
    __eq__ = lambda self, other: self.read() == other
    __ne__ = lambda self, other: self.read() != other
    __and__ = lambda self, other: self.read() & other
    __xor__ = lambda self, other: self.read() ^ other
    __or__ = lambda self, other: self.read() | other
    __lshift__ = lambda self, other: self.read() << other
    __rshift__ = lambda self, other: self.read() >> other

    __radd__ = lambda self, other: other + self.read()
    __rsub__ = lambda self, other: other - self.read()
    __rmul__ = lambda self, other: other * self.read()
    __rdiv__ = lambda self, other: old_div(other, self.read())
    __rmod__ = lambda self, other: other % self.read()
    __rand__ = lambda self, other: other & self.read()
    __rxor__ = lambda self, other: other ^ self.read()
    __ror__ = lambda self, other: other | self.read()

    __iadd__ = lambda self, other: self.write(self.read() + other)
    __isub__ = lambda self, other: self.write(self.read() - other)
    __imul__ = lambda self, other: self.write(self.read() * other)
    __idiv__ = lambda self, other: self.write(old_div(self.read(), other))
    __imod__ = lambda self, other: self.write(self.read() % other)
    __ipow__ = lambda self, other: self.write(self.read() ** other)
    __iand__ = lambda self, other: self.write(self.read() & other)
    __ixor__ = lambda self, other: self.write(self.read() ^ other)
    __ior__ = lambda self, other: self.write(self.read() | other)
    __ilshift__ = lambda self, other: self.write(self.read() << other)
    __irshift__ = lambda self, other: self.write(self.read() >> other)

    iadd = __iadd__
    isub = __isub__
    imul = __imul__
    idiv = __idiv__
    imod = __imod__
    ipow = __ipow__
    iand = __iand__
    ixor = __ixor__
    ior = __ior__
    ilshift = __ilshift__
    irshift = __irshift__

    store_in_mem = lambda self, address: self.read().store_in_mem(address)


class MemValue(_mem):
    __slots__ = ['last_write_block', 'reg_type', 'register', 'address', 'deleted']

    def __init__(self, value):
        self.last_write_block = None
        if isinstance(value, int):
            self.value_type = regint
            value = regint(value)
        elif isinstance(value, MemValue):
            self.value_type = value.value_type
        else:
            self.value_type = type(value)
        self.reg_type = self.value_type.reg_type
        self.address = program.malloc(1, self.value_type)
        self.deleted = False
        self.write(value)

    def delete(self):
        program.free(self.address, self.reg_type)
        self.deleted = True

    def check(self):
        if self.deleted:
            raise CompilerError('MemValue deleted')

    def read(self):
        self.check()
        if program.curr_block != self.last_write_block:
            self.register = library.load_mem(self.address, self.value_type)
            self.last_write_block = program.curr_block
        return self.register

    def write(self, value):
        self.check()
        if isinstance(value, MemValue):
            self.register = value.read()
        elif isinstance(value, (int, long)):
            self.register = self.value_type(value)
        else:
            self.register = value
        if not isinstance(self.register, self.value_type):
            raise CompilerError('Mismatch in register type, cannot write \
                %s to %s' % (type(self.register), self.value_type))
        self.register.store_in_mem(self.address)
        self.last_write_block = program.curr_block
        return self

    def reveal(self):
        if self.register.is_clear:
            return self.read()
        else:
            return self.read().reveal()

    less_than = lambda self, other, bit_length=None, security=None: \
        self.read().less_than(other, bit_length, security)
    greater_than = lambda self, other, bit_length=None, security=None: \
        self.read().greater_than(other, bit_length, security)
    less_equal = lambda self, other, bit_length=None, security=None: \
        self.read().less_equal(other, bit_length, security)
    greater_equal = lambda self, other, bit_length=None, security=None: \
        self.read().greater_equal(other, bit_length, security)
    equal = lambda self, other, bit_length=None, security=None: \
        self.read().equal(other, bit_length, security)
    not_equal = lambda self, other, bit_length=None, security=None: \
        self.read().not_equal(other, bit_length, security)

    pow2 = lambda self, *args, **kwargs: self.read().pow2(*args, **kwargs)
    mod2m = lambda self, *args, **kwargs: self.read().mod2m(*args, **kwargs)
    right_shift = lambda self, *args, **kwargs: self.read().right_shift(*args, **kwargs)

    bit_decompose = lambda self, *args, **kwargs: self.read().bit_decompose(*args, **kwargs)

    if_else = lambda self, *args, **kwargs: self.read().if_else(*args, **kwargs)

    def __repr__(self):
        return 'MemValue(%s,%d)' % (self.value_type, self.address)


class MemFloat(_mem):
    def __init__(self, *args):
        value = sfloat(*args)
        self.err = MemValue(value.err)
        self.v = MemValue(value.v)
        self.p = MemValue(value.p)
        self.z = MemValue(value.z)
        self.s = MemValue(value.s)

    def write(self, *args):
        value = sfloat(*args)
        self.v.write(value.v)
        self.p.write(value.p)
        self.z.write(value.z)
        self.s.write(value.s)
        self.err.write(value.err)

    def read(self):
        return sfloat(self.v, self.p, self.z, self.s, self.err)


class MemFix(_mem):
    def __init__(self, value):
        self.v = MemValue(value.v)

    def write(self, value):
        self.v.write(value.v)

    def reveal(self):
        return cfix(self.v.reveal())

    def read(self):
        val = self.v.read()
        if isinstance(val, sint):
            return sfix(val)
        else:
            return cfix(val)


def getNamedTupleType(*names):
    class NamedTuple(object):
        class NamedTupleArray(object):
            def __init__(self, size, t):
                import types
                self.arrays = [types.Array(size, t) for i in range(len(names))]

            def __getitem__(self, index):
                return NamedTuple(array[index] for array in self.arrays)

            def __setitem__(self, index, item):
                for array, value in zip(self.arrays, item):
                    array[index] = value

        @classmethod
        def get_array(cls, size, t):
            return cls.NamedTupleArray(size, t)

        def __init__(self, *args):
            if len(args) == 1:
                args = args[0]
            for name, value in zip(names, args):
                self.__dict__[name] = value

        def __iter__(self):
            for name in names:
                yield self.__dict__[name]

        def __add__(self, other):
            return NamedTuple(i + j for i, j in zip(self, other))

        def __sub__(self, other):
            return NamedTuple(i - j for i, j in zip(self, other))

        def __xor__(self, other):
            return NamedTuple(i ^ j for i, j in zip(self, other))

        def __mul__(self, other):
            return NamedTuple(other * i for i in self)

        __rmul__ = __mul__
        __rxor__ = __xor__

        def reveal(self):
            return self.__type__(x.reveal() for x in self)

    return NamedTuple


import library


class cintArray(Array):
    def __init__(self, n, address=None):
        Array.__init__(self, n, cint, address)


class cintMatrix(Matrix):
    def __init__(self, n, m, address=None):
        Matrix.__init__(self, n, m, cint, address)


class sintArray(Array):
    def __init__(self, n, address=None):
        Array.__init__(self, n, sint, address)


class sintMatrix(Matrix):
    def __init__(self, n, m, address=None):
        Matrix.__init__(self, n, m, sint, address)


cint.MemValue = MemValue
cint.Array = cintArray
cint.Matrix = cintMatrix

cfix.Array = cfixArray
cfix.Matrix = cfixMatrix
cfix.MemValue = MemFix

sint.MemValue = MemValue
sint.Array = sintArray
sint.Matrix = sintMatrix

sfloat.Array = sfloatArray
sfloat.Matrix = sfloatMatrix
sfloat.MemValue = MemFloat

sfix.Array = sfixArray
sfix.Matrix = sfixMatrix
sfix.MemValue = MemFix


def get_generic_array(value_type):
    class CustomMultiArray(MultiArray):
        def __init__(self, sizes):
            MultiArray.__init__(self, sizes, value_type)

    return CustomMultiArray


# generate MultiArray for every type
for value_type in [cint, cfix, sint, sfloat, sfix]:
    value_type.MultiArray = get_generic_array(value_type)
