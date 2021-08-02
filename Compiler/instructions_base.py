
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

import itertools
from random import randint
import time
import inspect
import functools
from Compiler.exceptions import *
from Compiler.config import *
from Compiler import util


###
### Opcode constants
###
### Whenever these are changed the corresponding enums in Processor/instruction.h
### MUST also be changed. (+ the documentation)
###
opcodes = dict(
  # Load/store
  LDI= 0x1,
  LDSI= 0x2,
  LDMC= 0x3,
  LDMS= 0x4,
  STMC= 0x5,
  STMS= 0x6,
  LDMCI= 0x7,
  LDMSI= 0x8,
  STMCI= 0x9,
  STMSI= 0xA,
  MOVC= 0xB,
  MOVS= 0xC,
  MOVINT= 0xD,
  MOVSB= 0xE,
  LDMINT= 0xCA,
  STMINT= 0xCB,
  LDMINTI= 0xCC,
  STMINTI= 0xCD,

  # Machine
  LDTN= 0x10,
  LDARG= 0x11,
  REQBL= 0x12,
  STARG= 0x13,
  CALL= 0x14,
  RETURN=0x15,
  CALLR= 0x16,
  JMPR= 0x17,
  RUN_TAPE= 0x19,
  JOIN_TAPE= 0x1A,
  CRASH= 0x1B,
  RESTART= 0x1C,
  CLEAR_MEMORY= 0x1D,
  CLEAR_REGISTERS= 0x1E,

  # Addition
  ADDC= 0x20,
  ADDS= 0x21,
  ADDM= 0x22,
  ADDCI= 0x23,
  ADDSI= 0x24,
  SUBC= 0x25,
  SUBS= 0x26,
  SUBML= 0x27,
  SUBMR= 0x28,
  SUBCI= 0x29,
  SUBSI= 0x2A,
  SUBCFI= 0x2B,
  SUBSFI= 0x2C,

  # Multiplication/division/other arithmetic
  MULC= 0x30,
  MULM= 0x31,
  MULCI= 0x32,
  MULSI= 0x33,
  DIVC= 0x34,
  DIVCI= 0x35,
  MODC= 0x36,
  MODCI= 0x37,
  LEGENDREC= 0x38,
  DIGESTC= 0x39,

  # IO
  OUTPUT_CLEAR= 0x40,
  INPUT_CLEAR= 0x41,
  OUTPUT_SHARES= 0x42,
  INPUT_SHARES= 0x43,
  PRIVATE_INPUT= 0x44,
  PRIVATE_OUTPUT= 0x46,
  OUTPUT_INT= 0x48,
  INPUT_INT= 0x49,
  OPEN_CHANNEL= 0x4A,
  CLOSE_CHANNEL= 0x4B,
  MPRIVATE_INPUT = 0x4C,
  MPRIVATE_OUTPUT = 0x4D,

  # Open
  STARTOPEN= 0xA0,
  STOPOPEN= 0xA1,
  OPENSINT= 0xA2,
  OPENSBIT= 0xA3,

  # Memory Management
  NEWC = 0xA4,
  NEWS = 0xA5,
  NEWINT = 0xA6,
  NEWSINT = 0xA7,
  DELETEC = 0xA8,
  DELETES = 0xA9,
  DELETEINT = 0xAA,
  DELETESINT = 0xAB,

  # Data access
  TRIPLE= 0x50,
  BIT= 0x51,
  SQUARE= 0x52,
  DABIT= 0x53,

  # Bitwise logical operations on regints
  ANDINT= 0x5A,
  ORINT= 0x5B,
  XORINT= 0x5C,
  INVINT= 0x5D,
  SHLINT= 0x5E,
  SHRINT= 0x5F,

  # sregint/sbit instructions
  LDMSINT= 0x60,
  LDMSINTI= 0x61,
  STMSINT= 0x62,
  STMSINTI= 0x63,
  MOVSINT= 0x64,
  LDSINT= 0x65,
  ADDSINT= 0x66,
  ADDSINTC= 0x67,
  SUBSINT= 0x68,
  SUBSINTC= 0x69,
  SUBCINTS= 0x6A,
  MULSINT= 0x6B,
  MULSINTC= 0x6C,
  DIVSINT= 0x6D,
  SHLSINT= 0x6E,
  SHRSINT= 0x6F,

  # Bitwise logic
  ANDC= 0x70,
  XORC= 0x71,
  ORC= 0x72,
  ANDCI= 0x73,
  XORCI= 0x74,
  ORCI= 0x75,
  NOTC= 0x76,
  NEG= 0x77,
  SAND= 0x78,
  XORSB= 0x79,
  ANDSB= 0x7A,
  ORSB= 0x7B,
  NEGB= 0x7C,
  LDSBIT = 0x7D,

  # Bitwise operations mod p
  SHLC= 0x80,
  SHRC= 0x81,
  SHLCI= 0x82,
  SHRCI= 0x83,
  EQZC = 0x84,
  LTZC = 0x85,

  # Branching and comparison
  JMP= 0x90,
  JMPNE= 0x91,
  JMPEQ= 0x92,
  EQZINT= 0x93,
  LTZINT= 0x94,
  LTINT= 0x95,
  GTINT= 0x96,
  EQINT= 0x97,

  # Integers
  LDINT= 0x9A,
  ADDINT= 0x9B,
  SUBINT= 0x9C,
  MULINT= 0x9D,
  DIVINT= 0x9E,
  MODINT= 0x9F,

  # Conversion
  CONVINT= 0xC0,
  CONVMODP= 0xC1,
  CONVSINTSREG= 0xC2,
  CONVREGSREG= 0xC3,
  CONVSREGSINT= 0xC4,
  CONVSUREGSINT= 0xC5,
  CONVSINTSBIT= 0xC6,
  CONVSBITSINT= 0xC7,

  # Debug Printing
  PRINT_MEM= 0xB0,
  PRINT_REG= 0xB2,
  PRINT_CHAR= 0xB3,
  PRINT_CHAR4= 0xB4,
  PRINT_CHAR_REGINT= 0xB5,
  PRINT_CHAR4_REGINT= 0xB6,
  PRINT_FLOAT= 0xB7,
  PRINT_FIX= 0xB8,
  PRINT_INT= 0xB9,
  PRINT_IEEE_FLOAT= 0xBA,

  # Comparison of sregints
  EQZSINT = 0xD0,
  LTZSINT = 0xD1,

  # Bitwise logical operations on sregints
  ANDSINT = 0xD3,
  ANDSINTC = 0xD4,
  ORSINT = 0xD5,
  ORSINTC = 0xD6,
  XORSINT = 0xD7,
  XORSINTC = 0xD8,
  INVSINT = 0xD9,

  # 64x64 -> 128 multiplier
  MUL2SINT = 0xDA,

  # GC/Local Function (and allied) commands
  GC = 0xDB,
  BITSINT = 0xDC,
  SINTBIT = 0xDD,
  LF      = 0xDE,
  SETBIT  = 0xDF,

  # Others
  RAND= 0xE0,
  START_CLOCK= 0xE1,
  STOP_CLOCK= 0xE2,

  RANDC = 0xE3,
  RANDINT = 0xE4,
  RANDSINT = 0xE5,
  RANDFLOAT = 0xE6,
  RANDSBIT = 0xE7,

  # Stack operations
  PUSHINT= 0x100,
  POPINT= 0x101,
  PEEKINT= 0x102,
  POKEINT= 0x103,
  GETSPINT= 0x104,

  PUSHSINT= 0x105,
  POPSINT= 0x106,
  PEEKSINT= 0x107,
  POKESINT= 0x108,
  GETSPSINT= 0x109,

  PUSHSBIT= 0x10A,
  POPSBIT= 0x10B,
  PEEKSBIT= 0x10C,
  POKESBIT= 0x10D,
  GETSPSBIT= 0x10E,

  PUSHC= 0x110,
  POPC= 0x111,
  PEEKC= 0x112,
  POKEC= 0x113,
  GETSPC= 0x114,

  PUSHS= 0x115,
  POPS= 0x116,
  PEEKS= 0x117,
  POKES= 0x118,
  GETSPS= 0x119,

  # Memory based vector addition/subtraction
  MADDC= 0x120,
  MADDS= 0x121,
  MADDM= 0x122,
  MSUBC= 0x125,
  MSUBS= 0x126,
  MSUBML= 0x127,
  MSUBMR= 0x128,

  # Memory based multiplication/division arithmetic
  MMULC= 0x130,
  MMULM= 0x131,
  MDIVC= 0x134,
  MMODC= 0x136,

  # Vector manipulation operations
  MREVC   = 0x138,
  MREVS   = 0x139,
  MEVALCC = 0x13A,
  MEVALSC = 0x13B,
  MBITDECC = 0x13C,
  MBITDECINT = 0x13D,
  MBITDECCS = 0x13E,

  # Relative peek and poke
  RPEEKINT= 0x1F0,
  RPOKEINT= 0x1F1,
  RPEEKSINT= 0x1F2,
  RPOKESINT= 0x1F3,
  RPEEKSBIT= 0x1F4,
  RPOKESBIT= 0x1F5,
  RPEEKC= 0x1F6,
  RPOKEC= 0x1F7,
  RPEEKS= 0x1F8,
  RPOKES= 0x1F9,
)


def int_to_bytes(x):
    """ 32 bit int to big-endian 4 byte conversion. """
    return [(x >> 8*i) % 256 for i in (3,2,1,0)]


global_vector_size = 1
global_vector_size_depth = 0
global_instruction_type_stack = ['modp', 'mod2n']

def set_global_vector_size(size):
    global global_vector_size, global_vector_size_depth
    if size == 1:
        return
    if global_vector_size == 1 or global_vector_size == size:
        global_vector_size = size
        global_vector_size_depth += 1
    else:
        raise CompilerError('Cannot set global vector size when already set')

def set_global_instruction_type(t):
    if t == 'modp' or t == 'mod2n':
        global_instruction_type_stack.append(t)
    else:
        raise CompilerError('Invalid type %s for setting global instruction type')

def reset_global_vector_size():
    global global_vector_size, global_vector_size_depth
    if global_vector_size_depth > 0:
        global_vector_size_depth -= 1
        if global_vector_size_depth == 0:
            global_vector_size = 1

def reset_global_instruction_type():
    global_instruction_type_stack.pop()

def get_global_vector_size():
    return global_vector_size

def get_global_instruction_type():
    return global_instruction_type_stack[-1]


def vectorize(instruction, global_dict=None):
    """ Decorator to vectorize instructions. """

    if global_dict is None:
        global_dict = inspect.getmodule(instruction).__dict__

    class Vectorized_Instruction(instruction):
        __slots__ = ['size']
        def __init__(self, size, *args, **kwargs):
            self.size = size
            super(Vectorized_Instruction, self).__init__(*args, **kwargs)
            for arg,f in zip(self.args, self.arg_format):
                if issubclass(ArgFormats[f], RegisterArgFormat):
                    arg.set_size(size)
        def get_code(self):
            return (self.size << 9) + self.code
        def get_pre_arg(self):
            return "%d, " % self.size
        def is_vec(self):
            return self.size > 1
        def get_size(self):
            return self.size
        def expand(self):
            set_global_vector_size(self.size)
            super(Vectorized_Instruction, self).expand()
            reset_global_vector_size()

    @functools.wraps(instruction)
    def maybe_vectorized_instruction(*args, **kwargs):
        if global_vector_size == 1:
            return instruction(*args, **kwargs)
        else:
            return Vectorized_Instruction(global_vector_size, *args, **kwargs)
    maybe_vectorized_instruction.vec_ins = Vectorized_Instruction
    maybe_vectorized_instruction.std_ins = instruction
    
    vectorized_name = 'v' + instruction.__name__
    Vectorized_Instruction.__name__ = vectorized_name
    global_dict[vectorized_name] = Vectorized_Instruction
    global_dict[instruction.__name__ + '_class'] = instruction
    return maybe_vectorized_instruction


class RegType(object):
    """ enum-like static class for Register types """
    ClearModp = 'c'
    SecretModp = 's'
    ClearInt = 'r'
    SecretInt = 'sr'
    SecretBit = 'sb'
    Types = [ClearModp, SecretModp, ClearInt, SecretInt, SecretBit]

    @staticmethod
    def create_dict(init_value_fn):
        """ Create a dictionary with all the RegTypes as keys """
        res = defaultdict(init_value_fn)
        # initialization for legacy
        for t in RegType.Types:
            res[t]
        return res

class ArgFormat(object):
    @classmethod
    def check(cls, arg):
        return NotImplemented

    @classmethod
    def encode(cls, arg):
        return NotImplemented

class RegisterArgFormat(ArgFormat):
    @classmethod
    def check(cls, arg):
        if not isinstance(arg, program.curr_tape.Register):
            raise ArgumentError(arg, 'Invalid register argument')
        if arg.i > REG_MAX:
            raise ArgumentError(arg, 'Register index too large')
        if arg.program != program.curr_tape:
            raise ArgumentError(arg, 'Register from other tape, trace: %s' % \
                                    util.format_trace(arg.caller))
        if arg.reg_type != cls.reg_type:
            raise ArgumentError(arg, "Wrong register type '%s', expected '%s'" % \
                                    (arg.reg_type, cls.reg_type))

    @classmethod
    def encode(cls, arg):
        return int_to_bytes(arg.i)

class ClearModpAF(RegisterArgFormat):
    reg_type = RegType.ClearModp


#Classes for Secret Registers
class SecretRegB64AF(RegisterArgFormat):
    reg_type = RegType.SecretInt


class SecretRegBAF(RegisterArgFormat):
    reg_type = RegType.SecretBit



## Added to make it srgeint friendly
class SecretModpAF(RegisterArgFormat):
        reg_type = RegType.SecretModp

class ClearIntAF(RegisterArgFormat):
    reg_type = RegType.ClearInt

class IntArgFormat(ArgFormat):
    @classmethod
    def check(cls, arg):
        if not isinstance(arg, (int, long)):
            raise ArgumentError(arg, 'Expected an integer-valued argument')

    @classmethod
    def encode(cls, arg):
        return int_to_bytes(arg)

class ImmediateModpAF(IntArgFormat):
    @classmethod
    def check(cls, arg):
        super(ImmediateModpAF, cls).check(arg)
        if arg >= 2**31 or arg < -2**31:
            raise ArgumentError(arg, 'Immediate value outside of 32-bit range')

class PlayerNoAF(IntArgFormat):
    @classmethod
    def check(cls, arg):
        super(PlayerNoAF, cls).check(arg)
        if arg > 256:
            raise ArgumentError(arg, 'Player number > 256')

class String(ArgFormat):
    length = 16

    @classmethod
    def check(cls, arg):
        if not isinstance(arg, str):
            raise ArgumentError(arg, 'Argument is not string')
        if len(arg) > cls.length:
            raise ArgumentError(arg, 'String longer than ' + cls.length)
        if '\0' in arg:
            raise ArgumentError(arg, 'String contains zero-byte')

    @classmethod
    def encode(cls, arg):
        return arg + '\0' * (cls.length - len(arg))

##should we add SecretRegModp
ArgFormats = {
    'c': ClearModpAF,
    's': SecretModpAF,
    'cw': ClearModpAF,
    'sw': SecretModpAF,
    'r': ClearIntAF,
    'rw': ClearIntAF,
    'i': ImmediateModpAF,
    'int': IntArgFormat,
    'p': PlayerNoAF,
    'str': String,
    'srw': SecretRegB64AF,
    'sr': SecretRegB64AF,
    'sbw' : SecretRegBAF,
    'sb' : SecretRegBAF
}

def format_str_is_reg(format_str):
    return issubclass(ArgFormats[format_str], RegisterArgFormat)

def format_str_is_writeable(format_str):
    return format_str_is_reg(format_str) and format_str[-1] == 'w'


class Instruction(object):
    """
    Base class for a RISC-type instruction. Has methods for checking arguments,
    getting byte encoding, etc.
    """
    __slots__ = ['args', 'arg_format', 'code', 'caller']
    count = 0


    def __init__(self, *args, **kwargs):
        """ Create an instruction and append it to the program list. """
        self.args = list(args)
        self.check_args()
        if not program.FIRST_PASS:
            if kwargs.get('add_to_prog', True):
                program.curr_block.instructions.append(self)
            if program.DEBUG:
                self.caller = [frame[1:] for frame in inspect.stack()[1:]]
            else:
                self.caller = None
        
        Instruction.count += 1
        if Instruction.count % 100000 == 0:
            print "Compiled %d lines at" % self.__class__.count, time.asctime()

    def get_code(self):
        return self.code
    
    def get_encoding(self):
        enc = int_to_bytes(self.get_code())
        # add the number of registers if instruction flagged as has var args
        if self.has_var_args():
            enc += int_to_bytes(len(self.args))
        for arg,format in zip(self.args, self.arg_format):
            enc += ArgFormats[format].encode(arg)

        return enc
    
    def get_bytes(self):
        return bytearray(self.get_encoding())
    
    
    def check_args(self):
        """ Check the args match up with that specified in arg_format """
        for n,(arg,f) in enumerate(itertools.izip_longest(self.args, self.arg_format)):
            if arg is None:
                if not isinstance(self.arg_format, (list, tuple)):
                    break # end of optional arguments
                else:
                    raise CompilerError('Incorrect number of arguments for instruction %s' % (self))
            try:
                ArgFormats[f].check(arg)
            except ArgumentError as e:
                raise CompilerError('Invalid argument "%s" to instruction: %s'
                    % (e.arg, self) + '\n' + e.msg)
            except KeyError as e:
                raise CompilerError('Incorrect number of arguments for instruction %s' % (self))
    
    def get_used(self):
        """ Return the set of registers that are read in this instruction. """
        return set(arg for arg,w in zip(self.args, self.arg_format) if \
            format_str_is_reg(w) and not format_str_is_writeable(w))
    
    def get_def(self):
        """ Return the set of registers that are written to in this instruction. """
        return set(arg for arg,w in zip(self.args, self.arg_format) if \
            format_str_is_writeable(w))
    
    def get_pre_arg(self):
        return ""

    def has_var_args(self):
        return False

    def is_vec(self):
        return False

    def get_size(self):
        return 1

    def add_usage(self, req_node):
        pass

    # String version of instruction attempting to replicate encoded version
    def __str__(self):
        
        if self.has_var_args():
            varargCount = str(len(self.args)) + ', '
        else:
            varargCount = ''

        return self.__class__.__name__ + ' ' + self.get_pre_arg() + varargCount + ', '.join(str(a) for a in self.args)

    def __repr__(self):
        return self.__class__.__name__ + '(' + self.get_pre_arg() + ','.join(str(a) for a in self.args) + ')'




###
### Memory access instructions
###

class DirectMemoryInstruction(Instruction):
    __slots__ = []
    def __init__(self, *args, **kwargs):
        super(DirectMemoryInstruction, self).__init__(*args, **kwargs)

class ReadMemoryInstruction(Instruction):
    __slots__ = []

class WriteMemoryInstruction(Instruction):
    __slots__ = []

class DirectMemoryWriteInstruction(DirectMemoryInstruction, \
                                       WriteMemoryInstruction):
    __slots__ = []
    def __init__(self, *args, **kwargs):
        if program.curr_tape.prevent_direct_memory_write:
            raise CompilerError('Direct memory writing prevented')
        super(DirectMemoryWriteInstruction, self).__init__(*args, **kwargs)

###
### I/O instructions
###

class DoNotEliminateInstruction(Instruction):
    """ What do you think? """
    __slots__ = []

class IOInstruction(DoNotEliminateInstruction):
    """ Instruction that uses stdin/stdout/etc during runtime. 
        These are linked to prevent instruction reordering during optimization. """
    __slots__ = []

    @classmethod
    def str_to_int(cls, s):
        """ Convert a 4 character string to an integer. """
        if len(s) > 4:
            raise CompilerError('String longer than 4 characters')
        n = 0
        for c in reversed(s.ljust(4)):
            n <<= 8
            n += ord(c)
        return n

###
### Data access instructions
###

class DataInstruction(Instruction):
    __slots__ = []
    field_type = 'modp'

    def add_usage(self, req_node):
        req_node.increment((self.field_type, self.data_type), self.get_size())

###
### Integer operations
### 

class StackInstruction(Instruction):
    """ Base class for thread-local stack instructions. """
    __slots__ = []


### 
### Clear shift instructions
###   - NPS Where is this used?
### 

class ClearShiftInstruction(Instruction):
    __slots__ = []
    arg_format = ['cw', 'c', 'i']

    def check_args(self):
        super(ClearShiftInstruction, self).check_args()
        bits = 63
        if self.args[2] > bits:
            raise CompilerError('Shifting by more than %d bits '
                                'not implemented' % bits)

###
### Jumps etc
###

class dummywrite(Instruction):
    """ Dummy instruction to create source node in the dependency graph,
        preventing read-before-write warnings. """
    __slots__ = []
    
    def __init__(self, *args, **kwargs):
        self.arg_format = [arg.reg_type + 'w' for arg in args]
        super(dummywrite, self).__init__(*args, **kwargs)
    
    def get_encoding(self):
        return []

class JumpInstruction(Instruction):
    __slots__ = ['jump_arg']

    def set_relative_jump(self, value):
        if value == -1:
            raise CompilerException('Jump by -1 would cause infinite loop')
        self.args[self.jump_arg] = value

    def get_relative_jump(self):
        if self.jump_arg == -1:
	  return -1
        return self.args[self.jump_arg]


class VarArgsInstruction(Instruction):
    def has_var_args(self):
        return True


class CISC(Instruction):
    """
    Base class for a CISC instruction.
    
    Children must implement expand(self) to process the instruction.
    """
    __slots__ = []
    code = None

    def __init__(self, *args):
        self.args = args
        self.check_args()
        if not program.FIRST_PASS:
            self.expand()
    
    def expand(self):
        """ Expand this into a sequence of RISC instructions. """
        raise NotImplementedError('expand method must be implemented')
