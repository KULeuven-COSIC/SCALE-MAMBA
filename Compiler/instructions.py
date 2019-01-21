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
    CLEAR_MEMORY = 0x1D,
    CLEAR_REGISTERS = 0x1E,

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

    # IO
    OUTPUT_CLEAR = 0x40,
    INPUT_CLEAR = 0x41,
    OUTPUT_SHARE = 0x42, (Class is OUTPUT_SHARES)
    INPUT_SHARE = 0x43, (Class is INPUT_SHARES)
    PRIVATE_INPUT = 0x44,
    PRIVATE_OUTPUT = 0x46,
    OUTPUT_INT = 0x48,
    INPUT_INT = 0x49,
    OPEN_CHAN = 0x4A,
    CLOSE_CHAN = 0x4B,

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
    EQZINT= 0x93,
    LTZINT= 0x94,
    LTINT= 0x95,
    GTINT= 0x96,
    EQINT= 0x97,
    JMPI= 0x98,

    # Integer Operations
    LDINT = 0x9A,
    ADDINT = 0x9B,
    SUBINT = 0x9C,
    MULINT = 0x9D,
    DIVINT = 0x9E,

    # Conversion
    CONVINT = 0xC0,
    CONVMODP = 0xC1,

    # Debug Printing
    PRINTMEM = 0xB0, (C: print_mem, base.opcodes "PRINTMEM")
    PRINTREG = 0XB1, (C: print_reg, base.opcode "PRINTREG")
    PRINTREGPLAIN = 0xB2, (C: print_reg_plain, base.opcode "PRINTREGPLAIN")
    PRINTCHR = 0xB3, (C: print_char, base.opcode "PRINTCHR")
    PRINTSTR = 0xB4 (C: print_char4 , base.opcode "PRINTSTR"
    PRINTCHRINT = 0xB5, (C: print_char_int, base.opcode "PRINTCHRINT")
    PRINTSTRINT = 0xB6, (C: print_char4_reg_int, base.opcode "PRINTSTRINT")
    PRINTFLOATPLAIN = 0xB7, (C: print_float_plain, base.opcode "PRINTFLOATPLAIN")
    PRINTFIXPLAIN = 0xB8, (C: print_fix_plain, base.opcode "PRINTFIXPLAIN")
    PRINTINT = 0xB9 (C: print_int, base.opcode "PRINTINT")

    # Others
    RAND = 0xE0,
    START_TIMER= 0xE1,
    STOP_TIMER= 0xE2


Many instructions can be vectorized, this is done by taking the opcode
being a 32 bit value. The last nine bits being the base opcode and previous
23 bits being how many times the instruction should be executed in
parallel. If this first 23 bits are zero, then this is interpreted as
one. The arguments of a vectorized instruction given the first
of the consecutive registers which should be accessed in turn.

Arguments to instructions can have various types
    'c'  : Clear Modp Register a.k.a. cint (Read Only),
    'cw' : Clear Modp Register (Write Only),
    's'  : Secret Modp Register a.k.a sint (Read Only),
    'sw' : Secret Modp Register (Write Only),
    'r'  : Clear RegInt Register a.k.a. regint (64-bit value) (Read Only),
    'rw' : Clear RegInt Register (64-bit value) (Write Only),
    'i'  : Integer Value Possibly Signed
    'int': Integer Value Unsigned
    'p'  : A Player Number
    'str': String

Global memory comes in three variants, which is not thread locked
    S[i] : sint memory
    C[i] : cint memory
    R[i] : regint memory

Registers come in the same types, but are thread local
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
    r""" LDI i n
         Assigns (loads) cint register c_i the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDI']
    arg_format = ['cw', 'i']

    def execute(self):
        self.args[0].value = self.args[1]


@base.vectorize
class ldsi(base.Instruction):
    r""" LDSI i n
         Assigns sint register s_i a share of the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDSI']
    arg_format = ['sw', 'i']

    def execute(self):
        self.args[0].value = self.args[1]


@base.vectorize
class ldmc(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMC i n
         Assigns cint register c_i the value in memory C[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMC']
    arg_format = ['cw', 'int']

    def execute(self):
        self.args[0].value = program.mem_c[self.args[1]]


@base.vectorize
class ldms(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMS i n
         Assigns sint register s_i the value in memory S[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMS']
    arg_format = ['sw', 'int']

    def execute(self):
        self.args[0].value = program.mem_s[self.args[1]]


@base.vectorize
class stmc(base.DirectMemoryWriteInstruction):
    r""" STMC i n
         Sets memory C[n] to be the value in cint register c_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMC']
    arg_format = ['c', 'int']

    def execute(self):
        program.mem_c[self.args[1]] = self.args[0].value


@base.vectorize
class stms(base.DirectMemoryWriteInstruction):
    r""" STMS i n
         Sets memory S[n] to be the value in sint register s_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMS']
    arg_format = ['s', 'int']

    def execute(self):
        program.mem_s[self.args[1]] = self.args[0].value


@base.vectorize
class ldmint(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMINT i n
         Assigns regint register r_i the value in memory R[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMINT']
    arg_format = ['rw', 'int']

    def execute(self):
        self.args[0].value = program.mem_i[self.args[1]]


@base.vectorize
class stmint(base.DirectMemoryWriteInstruction):
    r""" STMINT i n
         Sets regint memory R[n] to be the value r_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMINT']
    arg_format = ['r', 'int']

    def execute(self):
        program.mem_i[self.args[1]] = self.args[0].value


# must have seperate instructions because address is always modp
@base.vectorize
class ldmci(base.ReadMemoryInstruction):
    r""" LDMCI i j 
         Assigns cint register c_i the value in cint memory R[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMCI']
    arg_format = ['cw', 'r']

    def execute(self):
        self.args[0].value = program.mem_c[self.args[1].value]


@base.vectorize
class ldmsi(base.ReadMemoryInstruction):
    r""" LDMSI i j
         Assigns sint register s_i the value in sint memory S[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMSI']
    arg_format = ['sw', 'r']

    def execute(self):
        self.args[0].value = program.mem_s[self.args[1].value]


@base.vectorize
class stmci(base.WriteMemoryInstruction):
    r""" STMCI i j 
         Sets cint memory C[r_j] to be the value in cint register c_i, where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMCI']
    arg_format = ['c', 'r']

    def execute(self):
        program.mem_c[self.args[1].value] = self.args[0].value


@base.vectorize
class stmsi(base.WriteMemoryInstruction):
    r""" STMSI i j 
         Sets sint memory S[r_j] to be the value in sint register s_i, where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMSI']
    arg_format = ['s', 'r']

    def execute(self):
        program.mem_s[self.args[1].value] = self.args[0].value


@base.vectorize
class ldminti(base.ReadMemoryInstruction):
    r""" LDMINTI i j 
         Assigns regint register r_i the value in memory R[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMINTI']
    arg_format = ['rw', 'r']

    def execute(self):
        self.args[0].value = program.mem_i[self.args[1].value]


@base.vectorize
class stminti(base.WriteMemoryInstruction):
    r""" STMINTI i j
         Sets regint memory R[r_j] to be the value in regint register r_i,  where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMINTI']
    arg_format = ['r', 'r']

    def execute(self):
        program.mem_i[self.args[1].value] = self.args[0].value


@base.vectorize
class movc(base.Instruction):
    r""" MOVC i j
         Assigns cint register c_i the value in the cint register c_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVC']
    arg_format = ['cw', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value


@base.vectorize
class movs(base.Instruction):
    r""" MOVS i j
         Assigns sint register s_i the value in the sint register s_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVS']
    arg_format = ['sw', 's']

    def execute(self):
        self.args[0].value = self.args[1].value


@base.vectorize
class movint(base.Instruction):
    r""" MOVINT i j 
         Assigns regint register r_i the value in the regint register r_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVINT']
    arg_format = ['rw', 'r']


@base.vectorize
class pushint(base.StackInstruction):
    r""" PUSHINT i
         Pushes regint register r_i to the thread-local stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHINT']
    arg_format = ['r']


@base.vectorize
class popint(base.StackInstruction):
    r""" POPINT i
         Pops from the thread-local stack to regint register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPINT']
    arg_format = ['rw']


#
# Machine
#

@base.vectorize
class ldtn(base.Instruction):
    r""" LDTN i
         Assigns regint register r_i the number of the current thread.
         This instruction is vectorizable
     """
    code = base.opcodes['LDTN']
    arg_format = ['rw']


@base.vectorize
class ldarg(base.Instruction):
    r""" LDARG i
         Assigns the argument passed to the current thread to the regint register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['LDARG']
    arg_format = ['rw']


@base.vectorize
class starg(base.Instruction):
    r""" STARG i
         Assigns register r_i to variable in the thread argument.
         This instruction is vectorizable
     """
    code = base.opcodes['STARG']
    arg_format = ['r']


class reqbl(base.Instruction):
    r""" REQBL n
         Signals tape has been built so that it requires bit length n". 
     """
    code = base.opcodes['REQBL']
    arg_format = ['int']



class run_tape(base.Instruction):
    r""" RUN_TAPE i j n 
         In thread i start tape n with argument j. 
     """
    code = base.opcodes['RUN_TAPE']
    arg_format = ['int', 'int', 'int']


class join_tape(base.Instruction):
    r""" JOIN_TAPE i
         Wait until tape in thread i has finished. 
     """
    code = base.opcodes['JOIN_TAPE']
    arg_format = ['int']


class crash(base.IOInstruction):
    r""" CRASH
         Crash the runtime by calling CRASH on the IO class.
     """
    code = base.opcodes['CRASH']
    arg_format = []

class restart(base.IOInstruction):
    r""" RESTART
         Restart the runtime by reloading the schedule file. 
         Note, the schedule file may have changed since it has last been loaded. 
         See the main documentation for how this instruction is intended to be used.
         This can only be called by thread zero, otherwise the runtime aborts.
     """
    code = base.opcodes['RESTART']
    arg_format = []

class clear_memory(base.WriteMemoryInstruction):
    r""" CLEAR_MEMORY
         Clears the main memory. This can cause problems if executed in one thread
         and the memory is still being used in another. It is really for usage in
         thread zero, when all other threads are doing nothing. Say before a RESTART
     """
    code = base.opcodes['CLEAR_MEMORY']
    arg_format = [] 

class clear_registers(base.IOInstruction):
    r""" CLEAR_REGISTERS
         Like CLEAR_MEMORY but this clears the registers of the current processor,
         i.e. within the current thread. The order that this instruction is output
         by the MAMBA compiler may be a little erratic. It has mainly be added for
         some external users who are compiling their own byte-codes, so think of
         it as an experimental instruction.
     """
    code = base.opcodes['CLEAR_REGISTERS']
    arg_format = [] 


#
# Basic arithmetic
#

@base.vectorize
class addc(base.AddBase):
    r""" ADDC i j k
         Adds clear registers c_i=c_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class adds(base.AddBase):
    r""" ADDS i j k
         Adds secret registers s_i=s_j+s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class addm(base.AddBase):
    r""" ADDM i j k
         Adds a clear register onto a secret one s_i=s_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDM']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class subc(base.SubBase):
    r""" SUBC i j k
         Subtracts clear registers c_i=c_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class subs(base.SubBase):
    r""" SUBS i j k
         Subtracts secret registers s_i=s_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class subml(base.SubBase):
    r""" SUBML i j k
         Subtracts a clear register from a secret one s_i=s_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBML']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class submr(base.SubBase):
    r""" SUBMR i j k
         Subtracts a secret register from a clear one s_i=c_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBMR']
    arg_format = ['sw', 'c', 's']

# Multiplication/division

@base.vectorize
class mulc(base.MulBase):
    r""" MULC i j k
         Multiplication of clear registers c_i=c_j \cdot c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class mulm(base.MulBase):
    r""" MULM i j k
         Multiplication of clear and secret registers s_i=c_j \cdot s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULM']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class divc(base.Instruction):
    r""" DIVC i j k
         Division of clear registers c_i=c_j/c_k modulo p.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value * pow(self.args[2].value, program.P - 2, program.P) % program.P


@base.vectorize
class modc(base.Instruction):
    r""" MODC i j k
         Clear division with remainder c_i=c_j%c_k (after lifting to the integers).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = self.args[1].value % self.args[2].value


@base.vectorize
class legendrec(base.Instruction):
    r""" LEGENDREC i j
         Legendre symbol computation on a clear register c_i = (c_j / p).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LEGENDREC']
    arg_format = ['cw', 'c']


@base.vectorize
class digestc(base.Instruction):
    r""" DIGESTC i j
         Truncated hash computation on a clear register c_i = H(c_j)
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIGESTC']
    arg_format = ['cw', 'c']


#
# Bitwise logic (Bitwise operations)
#

@base.vectorize
class andc(base.Instruction):
    r""" ANDC i j k
         Lifts to integers, performs logical AND then converts back to modp, c_i = c_j and c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value & self.args[2].value) % program.P


@base.vectorize
class orc(base.Instruction):
    r""" ORC i j k
         Lifts to integers, performs logical OR then converts back to modp, c_i = c_j or c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value | self.args[2].value) % program.P


@base.vectorize
class xorc(base.Instruction):
    r""" XORC i j k
         Lifts to integers, performs logical OR then converts back to modp, c_i = c_j oplus c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value ^ self.args[2].value) % program.P


@base.vectorize
class notc(base.Instruction):
    r""" NOTC i j n
         This takes the integer representation of the clear register c_j, flips all the bits, adds 2^n and then converts back to clear modp register c_i.
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
    """ ADDCI i j n
        Addition of clear register to an immediate value c_i=c_j+n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDCI']
    op = '__add__'


@base.vectorize
class addsi(base.SharedImmediate):
    """ ADDSI i j n
        Addition of secret register to an immediate value s_i=s_j+n.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDSI']
    op = '__add__'


@base.vectorize
class subci(base.ClearImmediate):
    r""" SUBCI i j n
         Subtraction of clear register by an immediate value c_i=c_j-n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCI']
    op = '__sub__'


@base.vectorize
class subsi(base.SharedImmediate):
    r""" SUBSI i j n
         Subtraction of secret register by an immediate value s_i=s_j-n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSI']
    op = '__sub__'


@base.vectorize
class subcfi(base.ClearImmediate):
    r""" SUBCFI i j n
         Subtraction of clear register from an immediate value c_i=n-c_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCFI']
    op = '__rsub__'


@base.vectorize
class subsfi(base.SharedImmediate):
    r""" SUBSFI i j n
         Subtraction of secret register from an immediate value s_i=n-s_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSFI']
    op = '__rsub__'


@base.vectorize
class mulci(base.ClearImmediate):
    r""" MULCI i j n
         Multiplication of clear register by immediate value c_i=c_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULCI']
    op = '__mul__'


@base.vectorize
class mulsi(base.SharedImmediate):
    r""" MULCI i j n
         Multiplication of secret register by immediate value s_i=s_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULSI']
    op = '__mul__'


@base.vectorize
class divci(base.ClearImmediate):
    r""" DIVCI i j n
         Division of clear register by an immediate value c_i=c_j/n mod p.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVCI']

    def execute(self):
        self.args[0].value = self.args[1].value * pow(self.args[2], program.P - 2, program.P) % program.P


@base.vectorize
class modci(base.ClearImmediate):
    r""" MODC i j n
         Clear division with remainder c_i=c_j%n (after lifting to the integers) by an immediate
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODCI']
    op = '__mod__'

# Bitwise Operations with immediate values

@base.vectorize
class andci(base.ClearImmediate):
    r""" ANDCI i j n
         Equivalent of ANDC with an immediate value c_i = c_j and n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDCI']
    op = '__and__'


@base.vectorize
class xorci(base.ClearImmediate):
    r""" XORCI i j n
         Equivalent of XORC with an immediate value c_i = c_j oplus n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORCI']
    op = '__xor__'


@base.vectorize
class orci(base.ClearImmediate):
    r""" ORCI i j n 
         Equivalent of ORC with an immediate value c_i = c_j or n
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
    r""" SHLC i j k
         Clear bitwise shift left of cint register c_i = c_j << c_k (after converting to integers)
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value << self.args[2].value) % program.P


@base.vectorize
class shrc(base.Instruction):
    r""" SHRC i j k
         Clear bitwise shift right of cint register c_i = c_j >> c_k,(after converting to integers)
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRC']
    arg_format = ['cw', 'c', 'c']

    def execute(self):
        self.args[0].value = (self.args[1].value >> self.args[2].value) % program.P


@base.vectorize
class shlci(base.ClearShiftInstruction):
    r""" SHLCI i j n 
         Clear bitwise shift left of cint register by immediate value c_i = c_j << n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLCI']
    op = '__lshift__'


@base.vectorize
class shrci(base.ClearShiftInstruction):
    r""" SHRCI i j n 
         Clear bitwise shift right of cint register by immediate value c_i = c_j >> n
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
    r""" TRIPLE i j k
         Load sint registers s_i, s_j and s_k with the next multiplication triple.
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
    r""" BIT i
         Load sint register s_i with the next secret bit.
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
    r""" SQUARE i j
         Load sint registers s_i and s_j with the next squaring tuple.
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

class private_input(base.IOInstruction):
    r""" PRIVATE_INPUT i p m  
         Private input from player p on channel m assign result to sint s_i
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['PRIVATE_INPUT']
    arg_format = ['sw', 'p','int']


@base.vectorize
class print_mem(base.IOInstruction):
    r""" PRINTMEM i
         Print value in clear memory C[i] to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTMEM']
    arg_format = ['int']

    def execute(self):
        pass


@base.vectorize
class print_reg(base.IOInstruction):
    r""" PRINTREG i j
         Print value of cint register c_i to debug IO channel and 4-char comment j
         Can only be executed in thread zero.
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
    r""" PRINTREGPLAIN i
         As above but skips the comment j
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTREGPLAIN']
    arg_format = ['c']

@base.vectorize
class print_fix_plain(base.IOInstruction):
    r""" PRINTFIXPLAIN i f k
         Prints the fixed point number in cint register c_i using parameters f and k
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTFIXPLAIN']
    arg_format = ['c', 'i', 'i']

@base.vectorize
class print_float_plain(base.IOInstruction):
    r""" PRINTFLOATPLAIN i j k l
         Prints the floating point number in cint registers (c_i, c_j, c_k, c_l) assuming they map to the representation (v,p,z,s)
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTFLOATPLAIN']
    arg_format = ['c', 'c', 'c', 'c']


class print_int(base.IOInstruction):
    r""" PRINTINT i
         Prints the value of register r_i to debug IO channel. 
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINTINT']
    arg_format = ['r']


class print_char(base.IOInstruction):
    r""" PRINTCHAR i
         Prints a single character i to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTCHR']
    arg_format = ['int']

    def __init__(self, ch):
        super(print_char, self).__init__(ord(ch))


class print_char4(base.IOInstruction):
    r""" PRINTSTR i
         Print a 4 character string i to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTSTR']
    arg_format = ['int']

    def __init__(self, val):
        super(print_char4, self).__init__(self.str_to_int(val))


@base.vectorize
class print_char_regint(base.IOInstruction):
    r""" PRINTCHRINT i
         Print regint register r_i as a single character to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTCHRINT']
    arg_format = ['r']

@base.vectorize
class print_char4_regint(base.IOInstruction):
    r""" PRINTSTRINT i
         Print regint register r_i as a four character string to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINTSTRINT']
    arg_format = ['r']

@base.vectorize
class input_clear(base.IOInstruction):
    r""" INPUT_CLEAR i n
         Gets cint public input c_i from the IO class on channel n.
         Public inputs need to be the same for all players running the protocol, otherwise a crash will occur.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['INPUT_CLEAR']
    arg_format = ['cw','i']


@base.vectorize
class input_int(base.IOInstruction):
    r""" INPUT_INT i n
         Gets public regint input r_i from the IO class on channel n.
         Public inputs need to be the same for all players running the protocol, otherwise a crash will occur.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['INPUT_INT']
    arg_format = ['rw','i']

class open_chan(base.IOInstruction):
    r""" OPEN_CHAN n
         Opens channel number n for reading/writing on the IO class.
         Channels are assumed to be bi-directional, i.e. can read and write.
         This is provided as some IO classes may require this to be called explicitly, the default one does not need this.
         The return value *can* be some error code which the IO class may want to return.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OPEN_CHAN']
    arg_format = ['rw','i']

class close_channel(base.IOInstruction):
    r""" CLOSE_CHAN n
         Closes channel number n for reading/writing on the IO class.
         This is provided as some IO classes may require this to be called explicitly, the default one does not need this.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['CLOSE_CHAN']
    arg_format = ['i']


class output_shares(base.IOInstruction):
    r""" OUTPUT_SHARE (n+1) ch i1 i2 ... in
         Write shares s_{i_j} to the IO class channel ch. This can be called from our MAMBA language using 
               inp = [sint(1), sint(2), sint(3), sint(4)]
               output_shares(ch, *inp)
         Can only be executed in thread zero.
    """
    __slots__ = []
    code = base.opcodes['OUTPUT_SHARE']
    arg_format = tools.chain(['i'],itertools.repeat('s'))

    def has_var_args(self):
        return True


class input_shares(base.IOInstruction):
    r""" INPUT_SHARE (n+1) ch i1 i2 ... in
         Read shares s_{i_j} to the IO class channel ch. This can be called from our MAMBA language using
            inp = [sint(1), sint(2), sint(3), sint(4)]
            input_shares(ch,*inp)
         Can only be executed in thread zero.
    """
    __slots__ = []
    code = base.opcodes['INPUT_SHARE']
    arg_format = tools.chain(['i'],itertools.repeat('sw'))

    def has_var_args(self):
        return True

@base.vectorize
class output_clear(base.IOInstruction):
    r""" OUTPUT i n
         Public output of cint register c_i to IO class on channel n.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OUTPUT_CLEAR']
    arg_format = ['c','i']

@base.vectorize
class output_int(base.IOInstruction):
    r""" OUTPUT_INT i n 
         Public output of regint register r_i to IO class on channel n.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OUTPUT_INT']
    arg_format = ['r','i']


@base.vectorize
class private_output(base.IOInstruction):
    r""" PRIVATE_OUTPUT i p m
         Private output to p of the sint value s_i on channel m
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['PRIVATE_OUTPUT']
    arg_format = ['s', 'p', 'i']

# Others

@base.vectorize
class rand(base.Instruction):
    r""" RAND i j
         Writes to the regint register r_i a random value reduced modulo r_j
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RAND']
    arg_format = ['rw', 'r']


#
# Integer operations
#

@base.vectorize
class ldint(base.Instruction):
    r""" LDINT i n
         Sets regint register r_i to the immediate value n
         This instruction is vectorizable
    """
    __slots__ = []
    code = base.opcodes['LDINT']
    arg_format = ['rw', 'i']


@base.vectorize
class addint(base.IntegerInstruction):
    r""" ADDINT i j k
         Addition regint registers r_i=r_j+r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    code = base.opcodes['ADDINT']


@base.vectorize
class subint(base.IntegerInstruction):
    r""" SUBINT i j k
         Subtraction of regint registers r_i=r_j-r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    code = base.opcodes['SUBINT']


@base.vectorize
class mulint(base.IntegerInstruction):
    r""" MULINT i j k
         Multiplication of regint registers r_i=r_j*r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    code = base.opcodes['MULINT']


@base.vectorize
class divint(base.IntegerInstruction):
    r""" DIVINT i j k
         Division of regint registers r_i=r_j/r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    code = base.opcodes['DIVINT']


#
# Clear comparison instructions
#

@base.vectorize
class eqzc(base.UnaryComparisonInstruction):
    r""" EQZINT i j
         Clear comparison to zero test of regint registers r_i = (r_j == 0).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['EQZINT']

    def execute(self):
        if self.args[1].value == 0:
            self.args[0].value = 1
        else:
            self.args[0].value = 0


@base.vectorize
class ltzc(base.UnaryComparisonInstruction):
    r""" LTZINT i j
         Clear comparison of regint registers r_i = (r_j < 0).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LTZINT']


@base.vectorize
class ltc(base.IntegerInstruction):
    r""" LTINT i j k
         Clear comparison of regint registers r_i = (r_j < r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LTINT']


@base.vectorize
class gtc(base.IntegerInstruction):
    r""" GTINT i j k
         Clear comparison of regint registers r_i = (r_j > r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['GTINT']


@base.vectorize
class eqc(base.IntegerInstruction):
    r""" EQINT i j k
         Clear comparison of regint registers r_i = (r_j == r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['EQINT']


#
# Jumps etc
#

class jmp(base.JumpInstruction):
    r""" JMP n
         Unconditional relative jump of n+1 instructions. 
     """
    __slots__ = []
    code = base.opcodes['JMP']
    arg_format = ['int']
    jump_arg = 0

    def execute(self):
        pass


class jmpi(base.JumpInstruction):
    r""" JMPI i
         Unconditional relative jump of r_i+1 instructions. 
     """
    __slots__ = []
    code = base.opcodes['JMPI']
    arg_format = ['r']
    jump_arg = 0


class jmpnz(base.JumpInstruction):
    r""" JMPNZ i n
         Jump of n+1 instructions if regint register r_i \neq 0.
         Example:
            jmpnz(c, n) : advance n+1 instructions if c is non-zero
            jmpnz(c, 0) : do nothing
            jmpnz(c, -1): infinite loop if c is non-zero
     """
    __slots__ = []
    code = base.opcodes['JMPNZ']
    arg_format = ['r', 'int']
    jump_arg = 1

    def execute(self):
        pass


class jmpeqz(base.JumpInstruction):
    r""" JMPEQZ i n
         Jump n+1 instructions if regint register r_i == 0. 
     """
    __slots__ = []
    code = base.opcodes['JMPEQZ']
    arg_format = ['r', 'int']
    jump_arg = 1

    def execute(self):
        pass


#
# Conversions
#

@base.vectorize
class convint(base.Instruction):
    """ CONVINT i j
         Convert from regint register r_j to cint register c_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVINT']
    arg_format = ['cw', 'r']


@base.vectorize
class convmodp(base.Instruction):
    """ CONVMODP i j
        Convert from cint register c_j to regint register r_i.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVMODP']
    arg_format = ['rw', 'c', 'int']

    def __init__(self, *args, **kwargs):
        bitlength = kwargs.get('bitlength', program.bit_length)
        super(convmodp_class, self).__init__(*(args + (bitlength,)))


#
# Other instructions
#

@base.vectorize
class startopen(base.VarArgsInstruction):
    """ STARTOPEN n, s1,...,sn
        Start opening sint register s_i.
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
    """ STOPOPEN n, c1, ..., cn
        Store previous opened value in cint register c_i.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['STOPOPEN']
    arg_format = itertools.repeat('cw')

    def execute(self):
        for arg in self.args:
            arg.value = program.curr_block.open_queue.pop()


class start_clock(base.Instruction):
    r""" START_TIMER n
         Re-initializes the specified timer n 
     """
    code = base.opcodes['START_TIMER']
    arg_format = ['i']

class stop_clock(base.Instruction):
    r""" STOP_TIMER n
         Prints the time since the last initialization of timer n 
     """
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

