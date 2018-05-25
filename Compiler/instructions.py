""" This module is for classes of actual assembly instructions.
        Details:
        -Each Class name is the same as its opcode, except when indicated otherwise.
        -I/O Classes names differ in some cases from their opcode in order to facilitate readibility.
        -I/O SHARE instructions are used to implement Classes I/O SHARES, hence the variation on their class name,
    Remark: In comments C: stands for Class Name.

    # Load/store
    LDI = 0x1,
    LDSI = 0x2,
    LDMC = 0x3,
    LDMS = 0x4,
    STMC = 0x5,
    STMS = 0x6,
    LDMCI = 0x7,
    LDMSI = 0x8,
    STMCI = 0x9,
    STMSI = 0xA,
    MOVC = 0xB,
    MOVS = 0xC,
    LDMINT = 0xCA,
    STMINT = 0xCB,
    LDMINTI = 0xCC,
    STMINTI = 0xCD,
    PUSHINT = 0xCE,
    POPINT = 0xCF,
    MOVINT = 0xD0,

    # Machine
    LDTN = 0x10,
    LDARG = 0x11,
    STARG = 0x13,
    REQBL = 0x12,
    RUN_TAPE = 0x19,
    JOIN_TAPE = 0x1A,
    CRASH = 0x1B,
    RESTART = 0x1C,

    # Addition
    ADDC = 0x20,
    ADDS = 0x21,
    ADDM = 0x22,
    ADDCI = 0x23,
    ADDSI = 0x24,
    SUBC = 0x25,
    SUBS = 0x26,
    SUBML = 0x27,
    SUBMR = 0x28,
    SUBCI = 0x29,
    SUBSI = 0x2A,
    SUBCFI = 0x2B,
    SUBSFI = 0x2C,

    # Multiplication/division
    MULC = 0x30,
    MULM = 0x31,
    MULCI = 0x32,
    MULSI = 0x33,
    DIVC = 0x34,
    DIVCI = 0x35,
    MODC = 0x36,
    MODCI = 0x37,
    LEGENDREC = 0x38,
    DIGESTC = 0x39,

    # Open Instructions 
    STARTOPEN = 0xA0,
    STOPOPEN = 0xA1,

    # Data access instructions
    TRIPLE = 0x50,
    BIT = 0x51,
    SQUARE = 0x52,

    # Bitwise logic (Bitwise Operations)
    ANDC = 0x70,
    XORC = 0x71,
    ORC = 0x72,
    ANDCI = 0x73,
    XORCI = 0x74,
    ORCI = 0x75,
    NOTC = 0x76,

    # Bitwise shifts(Shift instructions)
    SHLC = 0x80,
    SHRC = 0x81,
    SHLCI = 0x82,
    SHRCI = 0x83,

    # Branching and comparison
    JMP= 0x90,
    JMPNZ= 0x91,
    JMPEQZ= 0x92,
    EQZC= 0x93,
    LTZC= 0x94,
    LTC= 0x95,
    GTC= 0x96,
    EQC= 0x97,
    JMPI= 0x98,

    # Integer Operations
    LDINT = 0x9A,
    ADDINT = 0x9B,
    SUBINT = 0x9C,
    MULINT = 0x9D,
    DIVINT = 0x9E,
    PRINTINT = 0x9F (C: print_int, base.opcode "PRINTINT")

    # Conversion
    CONVINT = 0xC0,
    CONVMODP = 0xC1,

    # IO
    PRINTMEM = 0xB0, (C: print_mem, base.opcodes "PRINTMEM")
    PRINTREG = 0XB1, (C: print_reg, base.opcode "PRINTREG")
    PRINTREGPLAIN = 0xB2, (C: print_reg_plain, base.opcode "PRINTREGPLAIN")
    PRINTCHR = 0xB3, (C: print_char, base.opcode "PRINTCHR")
    PRINTSTR = 0xB4 (C: print_char4 , base.opcode "PRINTSTR"
    PRINTCHRINT = 0xB5, (C: print_char_int, base.opcode "PRINTCHRINT")
    PRINTSTRINT = 0xB6, (C: print_char4_reg_int, base.opcode "PRINTSTRINT")
    PRINTFLOATPLAIN = 0xB7, (C: print_float_plain, base.opcode "PRINTFLOATPLAIN")
    PRINTFIXPLAIN = 0xC7, (C: print_fix_plain, base.opcode "PRINTFIXPLAIN")
    OUTPUT_CLEAR = 0xB8,
    INPUT_CLEAR = 0xB9,
    OUTPUT_SHARE = 0xBA, (Class is OUTPUT_SHARES)
    INPUT_SHARE = 0xBB, (Class is INPUT_SHARES)
    START_PRIVATE_INPUT = 0xBC,
    STOP_PRIVATE_INPUT = 0xBD,
    START_PRIVATE_OUTPUT = 0xBE,
    STOP_PRIVATE_OUTPUT = 0xBF,

    # Others
    RAND = 0xE0,
    START_TIMER= 0xE1,
    STOP_TIMER= 0xE2


Many instructions can be vectorized, this is done by taking the opcode
being a 32 bit value. The last nine bits being the base opcode and previous
23 bits being how many times the instruction should be executed in
parallel. If this first 23 bits are zero, then this is interpreted as
one. The arguments of a vectorized instruction given the first
of the consequtive registers which should be accessed in turn.
"""

"""

All base classes, utility functions etc. should go in instructions_base.py instead. 

This is for two reasons:
1) Easier generation of documentation
2) Ensures that 'from instructions import *' will only import assembly instructions and nothing else.

Note: every instruction should have a suitable docstring for auto-generation of
documentation

The opcodes are copied here from instruction_base.py to here for ease of use in documentation)

"""

##@instruction.py

import itertools
import tools
from random import randint
from Compiler.config import *
from Compiler.exceptions import *
import Compiler.instructions_base as base
# avoid naming collision with input instruction

#
# Load and store instructions
#
_python_input = input


@base.vectorize
class ldi(base.Instruction):
    r""" Assigns (loads) register c_i the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDI']
    arg_format = ['cw', 'i']

    def execute(self):
        self.args[0].value = self.args[1]


@base.vectorize
class ldsi(base.Instruction):
    r""" Assigns register s_i a share of the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDSI']
    arg_format = ['sw', 'i']

    def execute(self):
        self.args[0].value = self.args[1]


@base.vectorize
class ldmc(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" Assigns register c_i the value in memory \verb+C[n]+.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMC']
    arg_format = ['cw', 'int']

    def execute(self):
        self.args[0].value = program.mem_c[self.args[1]]


@base.vectorize
class ldms(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" Assigns register s_i the value in memory \verb+S[n]+.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMS']
    arg_format = ['sw', 'int']

    def execute(self):
        self.args[0].value = program.mem_s[self.args[1]]


@base.vectorize
class stmc(base.DirectMemoryWriteInstruction):
    r""" Sets \verb+C[n]+ to be the value c_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMC']
    arg_format = ['c', 'int']

    def execute(self):
        program.mem_c[self.args[1]] = self.args[0].value


@base.vectorize
class stms(base.DirectMemoryWriteInstruction):
    r""" Sets \verb+S[n]+ to be the value s_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMS']
    arg_format = ['s', 'int']

    def execute(self):
        program.mem_s[self.args[1]] = self.args[0].value


@base.vectorize
class ldmint(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" Assigns register ci_i the value in memory \verb+Ci[n]+.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMINT']
    arg_format = ['ciw', 'int']

    def execute(self):
        self.args[0].value = program.mem_i[self.args[1]]


@base.vectorize
class stmint(base.DirectMemoryWriteInstruction):
    r""" Sets \verb+Ci[n]+ to be the value ci_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMINT']
    arg_format = ['ci', 'int']

    def execute(self):
        program.mem_i[self.args[1]] = self.args[0].value


# must have seperate instructions because address is always modp
@base.vectorize
class ldmci(base.ReadMemoryInstruction):
    r""" Assigns register c_i the value in memory \verb+C[cj]+.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMCI']
    arg_format = ['cw', 'ci']

    def execute(self):
        self.args[0].value = program.mem_c[self.args[1].value]


@base.vectorize
class ldmsi(base.ReadMemoryInstruction):
    r""" Assigns register s_i the value in memory \verb+S[cj]+.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMSI']
    arg_format = ['sw', 'ci']

    def execute(self):
        self.args[0].value = program.mem_s[self.args[1].value]


@base.vectorize
class stmci(base.WriteMemoryInstruction):
    r""" Sets \verb+C[cj]+ to be the value c_i.
         This instruction is vectorizable
     """
    code = base.opcodes['STMCI']
    arg_format = ['c', 'ci']

    def execute(self):
        program.mem_c[self.args[1].value] = self.args[0].value


@base.vectorize
class stmsi(base.WriteMemoryInstruction):
    r""" Sets \verb+S[cj]+ to be the value s_i.
         This instruction is vectorizable
     """
    code = base.opcodes['STMSI']
    arg_format = ['s', 'ci']

    def execute(self):
        program.mem_s[self.args[1].value] = self.args[0].value


@base.vectorize
class ldminti(base.ReadMemoryInstruction):
    r""" Assigns register ci_i the value in memory \verb+Ci[cj]+.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMINTI']
    arg_format = ['ciw', 'ci']

    def execute(self):
        self.args[0].value = program.mem_i[self.args[1].value]


@base.vectorize
class stminti(base.WriteMemoryInstruction):
    r""" Sets \verb+Ci[cj]+ to be the value ci_i.
         This instruction is vectorizable
     """
    code = base.opcodes['STMINTI']
    arg_format = ['ci', 'ci']

    def execute(self):
        program.mem_i[self.args[1].value] = self.args[0].value


@base.vectorize
class movc(base.Instruction):
    r""" Assigns register c_i the value in the register c_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVC']
    arg_format = ['cw', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value


@base.vectorize
class movs(base.Instruction):
    r""" Assigns register s_i the value in the register s_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVS']
    arg_format = ['sw', 's']

    def execute(self):
        self.args[0].value = self.args[1].value


@base.vectorize
class movint(base.Instruction):
    r""" Assigns register ci_i the value in the register ci_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVINT']
    arg_format = ['ciw', 'ci']


@base.vectorize
class pushint(base.StackInstruction):
    r""" Pushes register ci_i to the thread-local stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHINT']
    arg_format = ['ci']


@base.vectorize
class popint(base.StackInstruction):
    r""" Pops from the thread-local stack to register ci_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPINT']
    arg_format = ['ciw']


#
# Machine
#

@base.vectorize
class ldtn(base.Instruction):
    r""" Assigns register c_i the number of the current thread.
         This instruction is vectorizable
     """
    code = base.opcodes['LDTN']
    arg_format = ['ciw']


@base.vectorize
class ldarg(base.Instruction):
    r""" Assigns the argument passed to the current thread to the register c_i.
         This instruction is vectorizable
     """
    code = base.opcodes['LDARG']
    arg_format = ['ciw']


@base.vectorize
class starg(base.Instruction):
    r""" Assigns register c_i to variable in the argument.
         This instruction is vectorizable
     """
    code = base.opcodes['STARG']
    arg_format = ['ci']


class reqbl(base.Instruction):
    r""" Built to indicate that it requires bit length n". """
    code = base.opcodes['REQBL']
    arg_format = ['int']



class run_tape(base.Instruction):
    r""" Start tape n in thread c_i with argument c_j. """
    code = base.opcodes['RUN_TAPE']
    arg_format = ['int', 'int', 'int']


class join_tape(base.Instruction):
    r""" Join thread c_i. """
    code = base.opcodes['JOIN_TAPE']
    arg_format = ['int']


class crash(base.IOInstruction):
    r""" Crash runtime. """
    code = base.opcodes['CRASH']
    arg_format = []

class restart(base.IOInstruction):
    r""" Restart the runtime by reloading the schedule file. 
         Note, the schedule file may have changed since it has
         last been loaded. See the main documentation for how
         this instruction is intended to be used.
     """
    code = base.opcodes['RESTART']
    arg_format = []


#
# Basic arithmetic
#

@base.vectorize
class addc(base.AddBase):
    r""" Clear addition c_i=c_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class adds(base.AddBase):
    r""" Secret addition s_i=s_j+s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class addm(base.AddBase):
    r""" Mixed addition s_i=s_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDM']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class subc(base.SubBase):
    r""" Clear subtraction c_i=c_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class subs(base.SubBase):
    r""" Secret subtraction s_i=s_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class subml(base.SubBase):
    r""" Mixed (secret and clear) subtraction s_i=s_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBML']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class submr(base.SubBase):
    r""" Mixed (clear and secret) subtraction s_i=c_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBMR']
    arg_format = ['sw', 'c', 's']

# Multiplication/division

@base.vectorize
class mulc(base.MulBase):
    r""" Clear multiplication c_i=c_j \cdot c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class mulm(base.MulBase):
    r""" Mixed (clear and secret) multiplication s_i=c_j \cdot s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULM']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class divc(base.Instruction):
    r""" Clear division c_i=c_j/c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value * pow(self.args[2].value, program.P - 2, program.P) % program.P


@base.vectorize
class modc(base.Instruction):
    r""" Clear modular reduction c_i=c_j/c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value % self.args[2].value


@base.vectorize
class legendrec(base.Instruction):
    r""" Clear Legendre symbol computation, c_i = (c_j / p).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LEGENDREC']
    arg_format = ['cw', 'c']


@base.vectorize
class digestc(base.Instruction):
    r""" Clear truncated hash computation, c_i = H(c_j)[bytes].
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIGESTC']
    arg_format = ['cw', 'c', 'int']


#
# Bitwise logic (Bitwise operations)
#

@base.vectorize
class andc(base.Instruction):
    r""" Clear logical AND c_i = c_j \land c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value & self.args[2].value) % program.P


@base.vectorize
class orc(base.Instruction):
    r""" Clear logical OR c_i = c_j \lor c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value | self.args[2].value) % program.P


@base.vectorize
class xorc(base.Instruction):
    r""" Clear logical XOR c_i = c_j \oplus c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value ^ self.args[2].value) % program.P


@base.vectorize
class notc(base.Instruction):
    r""" Clear logical NOT c_i = \lnot c_j
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['NOTC']
    arg_format = ['cw', 'c', 'int']

    def execute(self):
        self.args[0].value = (~self.args[1].value + 2 ** self.args[2]) % program.P


#
# Arithmetic with immediate values
#

@base.vectorize
class addci(base.ClearImmediate):
    """ Clear addition of immediate value c_i=c_j+n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDCI']
    op = '__add__'


@base.vectorize
class addsi(base.SharedImmediate):
    """ Secret addition of immediate value s_i=s_j+n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDSI']
    op = '__add__'


@base.vectorize
class subci(base.ClearImmediate):
    r""" Clear subtraction of immediate value c_i=c_j-n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCI']
    op = '__sub__'


@base.vectorize
class subsi(base.SharedImmediate):
    r""" Secret subtraction of immediate value s_i=s_j-n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSI']
    op = '__sub__'


@base.vectorize
class subcfi(base.ClearImmediate):
    r""" Clear subtraction from immediate value c_i=n-c_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCFI']
    op = '__rsub__'


@base.vectorize
class subsfi(base.SharedImmediate):
    r""" Secret subtraction from immediate value s_i=n-s_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSFI']
    op = '__rsub__'


@base.vectorize
class mulci(base.ClearImmediate):
    r""" Clear multiplication by immediate value c_i=c_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULCI']
    op = '__mul__'


@base.vectorize
class mulsi(base.SharedImmediate):
    r""" Secret multiplication by immediate value s_i=s_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULSI']
    op = '__mul__'


@base.vectorize
class divci(base.ClearImmediate):
    r""" Clear division by immediate value c_i=c_j/n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVCI']

    def execute(self):
        self.args[0].value = self.args[1].value * pow(self.args[2], program.P - 2, program.P) % program.P


@base.vectorize
class modci(base.ClearImmediate):
    r""" Clear modular reduction by immediate value c_i=c_j \mod{n}.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODCI']
    op = '__mod__'

# Bitwise Operations with immediate values

@base.vectorize
class andci(base.ClearImmediate):
    r""" Clear logical AND with immediate value c_i = c_j \land c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDCI']
    op = '__and__'


@base.vectorize
class xorci(base.ClearImmediate):
    r""" Clear logical XOR with immediate value c_i = c_j \oplus c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORCI']
    op = '__xor__'


@base.vectorize
class orci(base.ClearImmediate):
    r""" Clear logical OR with immediate value c_i = c_j \vee c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORCI']
    op = '__or__'


#
# Bitwise shifts(Shift instructions)
#

@base.vectorize
class shlc(base.Instruction):
    r""" Clear bitwise shift left c_i = c_j << c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value << self.args[2].value) % program.P


@base.vectorize
class shrc(base.Instruction):
    r""" Clear bitwise shift right c_i = c_j >> c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value >> self.args[2].value) % program.P


@base.vectorize
class shlci(base.ClearShiftInstruction):
    r""" Clear bitwise shift left by immediate value c_i = c_j << n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLCI']
    op = '__lshift__'


@base.vectorize
class shrci(base.ClearShiftInstruction):
    r""" Clear bitwise shift right by immediate value c_i = c_j >> n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRCI']
    op = '__rshift__'


#
# Data access instructions
#

@base.vectorize
class triple(base.DataInstruction):
    r""" Load secret variables s_i, s_j and s_k
    with the next multiplication triple.
         This instruction is vectorizable
     """
    __slots__ = ['data_type']
    code = base.opcodes['TRIPLE']
    arg_format = ['sw', 'sw', 'sw']
    data_type = 'triple'

    def execute(self):
        self.args[0].value = randint(0, program.P)
        self.args[1].value = randint(0, program.P)
        self.args[2].value = (self.args[0].value * self.args[1].value) % program.P


@base.vectorize
class bit(base.DataInstruction):
    r""" Load secret variable s_i
    with the next secret bit.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['BIT']
    arg_format = ['sw']
    data_type = 'bit'

    def execute(self):
        self.args[0].value = randint(0, 1)


@base.vectorize
class square(base.DataInstruction):
    r""" Load secret variables s_i and s_j
    with the next squaring tuple.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SQUARE']
    arg_format = ['sw', 'sw']
    data_type = 'square'

    def execute(self):
        self.args[0].value = randint(0, program.P)
        self.args[1].value = (self.args[0].value * self.args[0].value) % program.P


#
# I/O
#


class start_private_input(base.RawInputInstruction):
    r""" Receive inputs from player p.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['START_PRIVATE_INPUT']
    arg_format = ['p', 'int','i']
    field_type = 'modp'

    def add_usage(self, req_node):
        req_node.increment((self.field_type, 'input', self.args[0]), \
                           self.args[1])


class stop_private_input(base.RawInputInstruction):
    r""" Receive inputs from player p and put in the corresponding registers.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['STOP_PRIVATE_INPUT']
    arg_format = tools.chain(['p'], itertools.repeat('sw'))

    def has_var_args(self):
        return True


@base.vectorize
class print_mem(base.IOInstruction):
    r""" Print value in clear memory \verb|C[ci]| to stdout.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTMEM']
    arg_format = ['c']

    def execute(self):
        pass


@base.vectorize
class print_reg(base.IOInstruction):
    r""" Print value of register \verb|ci| to stdout and optional 4-char comment.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTREG']
    arg_format = ['c', 'i']

    def __init__(self, reg, comment=''):
        super(print_reg_class, self).__init__(reg, self.str_to_int(comment))

    def execute(self):
        pass


@base.vectorize
class print_reg_plain(base.IOInstruction):
    r""" Print only the value of register \verb|ci| to stdout.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTREGPLAIN']
    arg_format = ['c']

@base.vectorize
class print_fix_plain(base.IOInstruction):
    __slots__ = []
    code = base.opcodes['PRINTFIXPLAIN']
    arg_format = ['c', 'i', 'i']

@base.vectorize
class print_float_plain(base.IOInstruction):
    __slots__ = []
    code = base.opcodes['PRINTFLOATPLAIN']
    arg_format = ['c', 'c', 'c', 'c']


class print_int(base.IOInstruction):
    r""" Print only the value of register \verb|ci| to stdout. """
    __slots__ = []
    code = base.opcodes['PRINTINT']
    arg_format = ['ci']


class print_char(base.IOInstruction):
    r""" Print a single character to stdout. """
    code = base.opcodes['PRINTCHR']
    arg_format = ['int']

    def __init__(self, ch):
        super(print_char, self).__init__(ord(ch))


class print_char4(base.IOInstruction):
    r""" Print a 4 character string to stdout. """
    code = base.opcodes['PRINTSTR']
    arg_format = ['int']

    def __init__(self, val):
        super(print_char4, self).__init__(self.str_to_int(val))


@base.vectorize
class print_char_regint(base.IOInstruction):
    r""" Print register ci_i as a single character to stdout.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTCHRINT']
    arg_format = ['ci']

@base.vectorize
class print_char4_regint(base.IOInstruction):
    r""" Print register ci_i as a four character string to stdout.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTSTRINT']
    arg_format = ['ci']

@base.vectorize
class input_clear(base.PublicFileIOInstruction):
    r"""
        Gets public inputs from the IO class on channel n.
        Public inputs need to be the same for all players running the protocol, otherwise unstable behaviour will ensue.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['INPUT_CLEAR']
    arg_format = ['cw','i']


class output_shares(base.IOInstruction):
    r"""
       Write shares to the IO class channel n. This can be called from our meta-lenguage (python-like).
       Example:
       inp = [sint(1), sint(2), sint(3), sint(4)]
       output_shares(*inp,n)
    """
    __slots__ = []
    code = base.opcodes['OUTPUT_SHARE']
    arg_format = tools.chain(['i'],itertools.repeat('s'))

    def has_var_args(self):
        return True


class input_shares(base.IOInstruction):
    r"""
       Read shares from the IO class.
       Examples:
          inp = [sint(1), sint(2), sint(3), sint(4)]
          input_shares(*inp)
    """
    __slots__ = []
    code = base.opcodes['INPUT_SHARE']
    arg_format = tools.chain(['i'],itertools.repeat('sw'))

    def has_var_args(self):
        return True


@base.vectorize
class output_clear(base.PublicFileIOInstruction):
    r"""
         Public output of register \verb|ci| to IO class on channel n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['OUTPUT_CLEAR']
    arg_format = ['c','i']


@base.vectorize
class start_private_output(base.PublicFileIOInstruction):
    r""" Initiate private output to p of s_j via s_i.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['START_PRIVATE_OUTPUT']
    arg_format = ['sw', 's', 'p']


@base.vectorize
class stop_private_output(base.PublicFileIOInstruction):
    r""" Previously initialized private output to p on channel m via c_i.
         This instruction is vectorizable.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['STOP_PRIVATE_OUTPUT']
    arg_format = ['c', 'p', 'i']
 
# Others

@base.vectorize
class rand(base.Instruction):
    __slots__ = []
    code = base.opcodes['RAND']
    arg_format = ['ciw', 'ci']


#
# Integer operations
#

@base.vectorize
class ldint(base.Instruction):
    __slots__ = []
    code = base.opcodes['LDINT']
    arg_format = ['ciw', 'i']


@base.vectorize
class addint(base.IntegerInstruction):
    __slots__ = []
    code = base.opcodes['ADDINT']


@base.vectorize
class subint(base.IntegerInstruction):
    __slots__ = []
    code = base.opcodes['SUBINT']


@base.vectorize
class mulint(base.IntegerInstruction):
    __slots__ = []
    code = base.opcodes['MULINT']


@base.vectorize
class divint(base.IntegerInstruction):
    __slots__ = []
    code = base.opcodes['DIVINT']


#
# Clear comparison instructions
#

@base.vectorize
class eqzc(base.UnaryComparisonInstruction):
    r""" Clear comparison -zero test- c_i = (c_j \stackrel{?}{==} 0).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['EQZC']

    def execute(self):
        if self.args[1].value == 0:
            self.args[0].value = 1
        else:
            self.args[0].value = 0


@base.vectorize
class ltzc(base.UnaryComparisonInstruction):
    r""" Clear comparison c_i = (c_j \stackrel{?}{<} 0).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LTZC']


@base.vectorize
class ltc(base.IntegerInstruction):
    r""" Clear comparison c_i = (c_j \stackrel{?}{<} c_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LTC']


@base.vectorize
class gtc(base.IntegerInstruction):
    r""" Clear comparison c_i = (c_j \stackrel{?}{>} c_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['GTC']


@base.vectorize
class eqc(base.IntegerInstruction):
    r""" Clear comparison c_i = (c_j \stackrel{?}{==} c_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['EQC']


#
# Jumps etc
#

class jmp(base.JumpInstruction):
    r""" Unconditional relative jump of n+1 instructions. """
    __slots__ = []
    code = base.opcodes['JMP']
    arg_format = ['int']
    jump_arg = 0

    def execute(self):
        pass


class jmpi(base.JumpInstruction):
    r""" Unconditional relative jump of c_i+1 instructions. """
    __slots__ = []
    code = base.opcodes['JMPI']
    arg_format = ['ci']
    jump_arg = 0


class jmpnz(base.JumpInstruction):
    r""" Jump n+1 instructions if c_i \neq 0.
    Example:
    jmpnz(c, n) : advance n+1 instructions if c is non-zero
    jmpnz(c, 0) : do nothing
    jmpnz(c, -1): infinite loop if c is non-zero
    """
    __slots__ = []
    code = base.opcodes['JMPNZ']
    arg_format = ['ci', 'int']
    jump_arg = 1

    def execute(self):
        pass


class jmpeqz(base.JumpInstruction):
    r""" Jump n+1 instructions if c_i == 0. """
    __slots__ = []
    code = base.opcodes['JMPEQZ']
    arg_format = ['ci', 'int']
    jump_arg = 1

    def execute(self):
        pass


#
# Conversions
#

@base.vectorize
class convint(base.Instruction):
    """ Convert from integer register ci_j to clear modp register c_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVINT']
    arg_format = ['cw', 'ci']


@base.vectorize
class convmodp(base.Instruction):
    """ Convert from clear modp register c_j to integer register ci_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVMODP']
    arg_format = ['ciw', 'c', 'int']

    def __init__(self, *args, **kwargs):
        bitlength = kwargs.get('bitlength', program.bit_length)
        super(convmodp_class, self).__init__(*(args + (bitlength,)))


#
# Other instructions
#

@base.vectorize
class startopen(base.VarArgsInstruction):
    """ Start opening secret register s_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['STARTOPEN']
    arg_format = itertools.repeat('s')

    def execute(self):
        for arg in self.args[::-1]:
            program.curr_block.open_queue.append(arg.value)


@base.vectorize
class stopopen(base.VarArgsInstruction):
    """ Store previous opened value in c_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['STOPOPEN']
    arg_format = itertools.repeat('cw')

    def execute(self):
        for arg in self.args:
            arg.value = program.curr_block.open_queue.pop()


class start_clock(base.IOInstruction):
    r""" Re-initializes the specified timer n """
    code = base.opcodes['START_TIMER']
    arg_format = ['i']

class stop_clock(base.IOInstruction):
    r""" Prints the time since the last initialization of timer n """
    code = base.opcodes['STOP_TIMER']
    arg_format = ['i']



#
# CISC-style instructions (To Be Excluded TODO:Nigel)
#

# rename 'open' to avoid conflict with built-in open function
@base.vectorize
class asm_open(base.CISC):
    """ Open the value in s_j and assign it to c_i.
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['cw', 's']

    def expand(self):
        startopen(self.args[1])
        stopopen(self.args[0])


@base.vectorize
class muls(base.CISC):
    """ Secret multiplication s_i = s_j \cdot s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sw', 's', 's']

    def expand(self):
        s = [program.curr_block.new_reg('s') for i in range(9)]
        c = [program.curr_block.new_reg('c') for i in range(3)]
        triple(s[0], s[1], s[2])
        subs(s[3], self.args[1], s[0])
        subs(s[4], self.args[2], s[1])
        startopen(s[3], s[4])
        stopopen(c[0], c[1])
        mulm(s[5], s[1], c[0])
        mulm(s[6], s[0], c[1])
        mulc(c[2], c[0], c[1])
        adds(s[7], s[2], s[5])
        adds(s[8], s[7], s[6])
        addm(self.args[0], s[8], c[2])


@base.vectorize
class sqrs(base.CISC):
    """ Secret squaring s_i = s_j \cdot s_j.
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sw', 's']

    def expand(self):
        s = [program.curr_block.new_reg('s') for i in range(6)]
        c = [program.curr_block.new_reg('c') for i in range(2)]
        square(s[0], s[1])
        subs(s[2], self.args[1], s[0])
        asm_open(c[0], s[2])
        mulc(c[1], c[0], c[0])
        mulm(s[3], self.args[1], c[0])
        adds(s[4], s[3], s[3])
        adds(s[5], s[1], s[4])
        subml(self.args[0], s[5], c[1])


@base.vectorize
class lts(base.CISC):
    """ Secret comparison s_i = (s_j < s_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sw', 's', 's', 'int', 'int']

    def expand(self):
        a = program.curr_block.new_reg('s')
        subs(a, self.args[1], self.args[2])
        comparison.LTZ(self.args[0], a, self.args[3], self.args[4])

# hack for circular dependency
from Compiler import comparison
