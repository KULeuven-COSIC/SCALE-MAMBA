
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

""" This module is for classes of actual assembly instructions.
        Details:
        -Each Class name is the same as its opcode, except when indicated otherwise.
        -I/O Classes names differ in some cases from their opcode in order to facilitate readibility.
        -I/O SHARE instructions are used to implement Classes I/O SHARES, hence the variation on their class name,
    Remark: In comments C: stands for Class Name.

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
  MOVSB = 0xE,
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
  RETURN= 0x15,
  CALLR = 0x16,
  JMPR = 0x17,
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
  LDSBIT = 0x7C,

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

The memory based vector add/sub/mult instructions are not
accessable from Mamba. There syntax is
  INSTR i j k n
Where the size of the vector is in register rn
  ri, rj and rk point to the start of the vectors 
We then execute
    MEM[ri+t] = MEM[rj+t] op MEM[rk+t] t=0...rn-1

MINV* and MEVAL* are also not used in Mamba

   MREVC i j k
Takes memory pointed to by rj of length rk and puts it
in the reverse order in position ri

   MEVALSC i j k t
Takes S memory pointed to by rj of length rk and a
C variable ct and evaluates the poly in ct, returning
the result in register si

   MBITDECC i j k
Takes cint register cj and decomposes it into rk bits
and places them in C[ri+t] for t=0...rk-1

   MBITDECCS i j k
Takes cint register cj and decomposes it into rk bits
and places them in C[ri+t] for t=0...rk-1. Assumes
cj is signed, i.e. if cj>p/2 then this really bit decomposes
cj-p/2.

  MBITDECINT i j k
Takes register rj and decomposes it into rk bits
and places them in R[ri+t] for t=0...rk-1
 

Many instructions can be vectorized, this is done by taking the opcode
being a 32 bit value. The last nine bits being the base opcode and previous
23 bits being how many times the instruction should be executed in
parallel. If this first 23 bits are zero, then this is interpreted as
one. The arguments of a vectorized instruction given the first
of the consecutive registers which should be accessed in turn.

Arguments to instructions can have various types
    'c'   : Clear Modp Register a.k.a. cint (Read Only),
    'cw'  : Clear Modp Register (Write Only),
    's'   : Secret Modp Register a.k.a sint (Read Only),
    'sw'  : Secret Modp Register (Write Only),
    'r'   : Clear RegInt Register a.k.a. regint (64-bit value) (Read Only),
    'rw'  : Clear RegInt Register (64-bit value) (Write Only),
    'sb'  : A secret bit (Read Only)
    'sbw' : A secret bit (Write Only)
    'sr'  : A secret RegInt (64-bit value) (Read Only)
    'srw' : A secret RegInt (64-bit value) (Write Only)
    'i'  : Integer Value Possibly Signed
    'int': Integer Value Unsigned
    'str': String

Global memory comes in three variants, which is not thread locked
    S[i] : sint memory
    C[i] : cint memory
    R[i] : regint memory

Registers come in the same types, but are thread local

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


@base.vectorize
class ldsi(base.Instruction):
    r""" LDSI i n
         Assigns sint register s_i a share of the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDSI']
    arg_format = ['sw', 'i']


## ##########################NEW OPCODES##########################
## Added opcodes for new data types

#access opcodes
@base.vectorize
class ldsint(base.Instruction):
    r""" LDSINT i n
         Assigns sregint register sr_i a share of the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDSINT']
    arg_format = ['srw', 'i']


@base.vectorize
class movsint(base.Instruction):
    r""" MOVSINT i j
         Assigns secure register sr_i the value in the secure register sr_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVSINT']
    arg_format = ['srw', 'sr']


@base.vectorize
class opensint(base.Instruction):
    """ OPENSINT i j
        Open the sregint in sr_j and assign it to r_i.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['OPENSINT']
    arg_format = ['rw', 'sr']


@base.vectorize
class opensbit(base.Instruction):
    """ OPENSBIT i j
        Open the sbit in sb_j and assign it to r_i.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['OPENSBIT']
    arg_format = ['rw', 'sb']


#arithmetic opcodes
@base.vectorize
class addsint(base.Instruction):
    r""" ADDSINT i j k
         Adds secret registers sr_i=sr_j+sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDSINT']
    arg_format = ['srw', 'sr', 'sr']

@base.vectorize
class addsintc(base.Instruction):
    r""" ADDSINTC i j k
         Adds secrent and clear registers sr_i=sr_j+r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDSINTC']
    arg_format = ['srw', 'sr', 'r']


@base.vectorize
class mulsint(base.Instruction):
    r""" MULSINT i j k
         Multiplication of secret registers sr_i=sr_j \cdot sr_k, result is mod 2^64.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULSINT']
    arg_format = ['srw', 'sr', 'sr']


@base.vectorize
class mulsintc(base.Instruction):
    r""" MULSINTC i j k
         Multiplication of secret and clear registers sr_i=sr_j \cdot r_k, result is mod 2^64.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULSINTC']
    arg_format = ['srw', 'sr', 'r']

@base.vectorize
class mul2sint(base.Instruction):
    r""" MUL2SINT i j u v
         Full multiplication of secret registers (sr_i || sr_j )=sr_u \cdot sr_v.
         Where sr_i is the most significant word and sr_j is the least
         significant word of the output.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MUL2SINT']
    arg_format = ['srw', 'srw', 'sr', 'sr']

class GC(base.StackInstruction):
    r""" GC n
         This calls the Garbled Circuit with index n.
         The inputs are pulled from the srint stack, and the
         outputs are pushed back to the srint stack.
    """
    __slots__ = []
    code = base.opcodes['GC']
    arg_format = ['i']

class LF(base.StackInstruction):
    r""" LF n
         This calls the Local Function with index n.
         The inputs are pulled from various stacks as needed, and the
         outputs are pushed back to the same stacks.
    """
    __slots__ = []
    code = base.opcodes['LF']
    arg_format = ['i']


@base.vectorize
class subsintc(base.Instruction):
    r""" SUBSINTC i j k
         Subtracts secret and clear registers sr_i=sr_j-r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSINTC']
    arg_format = ['srw', 'sr', 'r']


@base.vectorize
class subsint(base.Instruction):
    r""" SUBSINT i j k
         Subtracts secret registers sr_i=sr_j-sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSINT']
    arg_format = ['srw', 'sr', 'sr']


@base.vectorize
class subcints(base.Instruction):
    r""" SUBCINTS i j k
         Subtracts clear and secret registers sr_i=r_j-sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCINTS']
    arg_format = ['srw', 'r', 'sr']


@base.vectorize
class divsint(base.Instruction):
    r""" DIVSINT i j k
         Division of sregint registers sr_i=sr_j / sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVSINT']
    arg_format = ['srw', 'sr', 'sr']

@base.vectorize
class shlsint(base.Instruction):
    r""" SHLSINT i j k
         Shift an sregint register left by k values
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLSINT']
    arg_format = ['srw', 'sr', 'int']

@base.vectorize
class shrsint(base.Instruction):
    r""" SHRSINT i j k
         Shift an sregint register right by k values
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRSINT']
    arg_format = ['srw', 'sr', 'int']


@base.vectorize
class neg(base.Instruction):
    r""" NEG i j
         Negation of a regint sr_i=-sr_j .
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['NEG']
    arg_format = ['srw', 'sr']


# Bitwise and Comparison opcodes
@base.vectorize
class sand(base.Instruction):
    r""" SAND i j k
         ANDs secret and secret bit sr_i= sr_j & sb_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SAND']
    arg_format = ['srw', 'sr', 'sb']

@base.vectorize
class andint(base.Instruction):
    r""" ANDINT i j k
         ANDs registers r_i= r_j & r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDINT']
    arg_format = ['rw', 'r', 'r']


@base.vectorize
class orint(base.Instruction):
    r""" ORINT i j k
         ORs registers r_i= r_j | r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORINT']
    arg_format = ['rw', 'r', 'r']

@base.vectorize
class xorint(base.Instruction):
    r""" XORINT i j k
         XORs registers r_i= r_j ^ r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORINT']
    arg_format = ['rw', 'r', 'r']


@base.vectorize
class invint(base.Instruction):
    r""" INVINT i j
         Bitwise inversion of the register
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['INVINT']
    arg_format = ['srw', 'sr']

@base.vectorize
class shlint(base.Instruction):
    r""" SHLINT i j k
         SHL register r_i= r_j << r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHLINT']
    arg_format = ['rw', 'r', 'r']

@base.vectorize
class shrint(base.Instruction):
    r""" SHRINT i j k
         SHR register r_i= r_j >> r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRINT']
    arg_format = ['rw', 'r', 'r']

@base.vectorize
class andsint(base.Instruction):
    r""" ANDSINT i j k
         ANDs secret registers sr_i= sr_j & sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDSINT']
    arg_format = ['srw', 'sr', 'sr']

@base.vectorize
class andsintc(base.Instruction):
    r""" ANDSINT i j k
         ANDs secret register with a clear sr_i= sr_j & r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDSINTC']
    arg_format = ['srw', 'sr', 'r']

@base.vectorize
class orsint(base.Instruction):
    r""" ORSINT i j k
         ORs secret registers sr_i= sr_j | sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORSINT']
    arg_format = ['srw', 'sr', 'sr']

@base.vectorize
class orsintc(base.Instruction):
    r""" ORSINTC i j k
         ORs secret register with a clear sr_i= sr_j | r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORSINTC']
    arg_format = ['srw', 'sr', 'r']

@base.vectorize
class xorsint(base.Instruction):
    r""" XORSINT i j k
         XORs secret registers sr_i= sr_j ^ sr_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORSINT']
    arg_format = ['srw', 'sr', 'sr']

@base.vectorize
class xorsintc(base.Instruction):
    r""" XORSINTC i j k
         XORs secret register with a clear sr_i= sr_j ^ r_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORSINTC']
    arg_format = ['srw', 'sr', 'r']

@base.vectorize
class invsint(base.Instruction):
    r""" INVSINT i j
         Bitwise inversion of the register
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['INVSINT']
    arg_format = ['srw', 'sr']




@base.vectorize
class xorsb(base.Instruction):
    r""" XORSB i j k
         Secret xor of sbit registers sb_i = (sb_j xor sb_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sbw', 'sb', 'sb']
    code = base.opcodes['XORSB']


@base.vectorize
class andsb(base.Instruction):
    r""" ANDSB i j k
         Secret and of sbit registers sb_i = (sb_j and sb_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sbw', 'sb', 'sb']
    code = base.opcodes['ANDSB']


@base.vectorize
class orsb(base.Instruction):
    r""" ORSB i j k
         Secret and of sbit registers sr_i = (sb_j or sb_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sbw', 'sb', 'sb']
    code = base.opcodes['ORSB']


@base.vectorize
class negb(base.Instruction):
    r""" NEGB i j
         Secret neg of sbit registers sb_i = ~sb_j.
         This instruction is vectorizable
     """
    __slots__ = []

    arg_format = ['sbw', 'sb']
    code = base.opcodes['NEGB']


@base.vectorize
class ltzsint(base.Instruction):
    r""" LTZSINT i j
         Secret less than of zero of secret register sb_i = ~sr_j < 0.
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sbw', 'sr']
    code = base.opcodes['LTZSINT']


@base.vectorize
class eqzsint(base.Instruction):
    r""" EQZSINT i j
         Secret equality test to zero of secret register sb_i = ~sr_j == 0.
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['sbw', 'sr']
    code = base.opcodes['EQZSINT']

@base.vectorize
class bitsint(base.Instruction):
    r""" BITSINT i j n
         Assigns sbit register sb_i the n-th bit of register sr_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['BITSINT']
    arg_format = ['sbw', 'sr', 'int']

@base.vectorize
class sintbit(base.Instruction):
    r""" SINTBIT i j k n
         Assigns srj to sri, and then sets the n-th bit to be sb_k
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['SINTBIT']
    arg_format = ['srw', 'sr', 'sb', 'int']

@base.vectorize
class setbit(base.Instruction):
    r""" SETBIT i k n
         Assigns zero to sri, and then sets the n-th bit to be sb_k
         The assignment of zero, rather than take an existing register
         is to ensure we maintain SSA.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['SETBIT']
    arg_format = ['srw', 'sb', 'int']

@base.vectorize
class ldsbit(base.Instruction):
    r""" LDSBIT i n
         Assigns sbit register sb_i a share of the value n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['LDSBIT']
    arg_format = ['sbw', 'i']



# Conversion opcodes
@base.vectorize
class convsintsreg(base.Instruction):
    """ CONVSINTSREG i j
         Convert from sint  s_j to sregint register sr_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVSINTSREG']
    arg_format = ['srw', 's']


@base.vectorize
class convregsreg(base.Instruction):
    """ CONVREGSREG i j
         Convert from regint register r_j to secret register sr_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVREGSREG']
    arg_format = ['srw', 'r']


@base.vectorize
class convsregsint(base.Instruction):
    """ CONVSREGSINT i j
         Convert from signed sregint sr_j to sint register s_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVSREGSINT']
    arg_format = ['sw', 'sr']

@base.vectorize
class convsuregsint(base.Instruction):
    """ CONVSUREGSINT i j
         Convert from unsigned sregint sr_j to sint register s_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVSUREGSINT']
    arg_format = ['sw', 'sr']

@base.vectorize
class convsintsbit(base.Instruction):
    """ CONVSINTSBIT i j
         Convert from sint register s_j to sbit register s_i.
         Assumes sr_j holds only a bit. If not all security
         assumptions are invalid.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVSINTSBIT']
    arg_format = ['sbw', 's']


@base.vectorize
class convsbitsint(base.Instruction):
    """ CONVSBITSINT i j
         Convert from sbit register sb_j to sint register s_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['CONVSBITSINT']
    arg_format = ['sw', 'sb']


# memory instructions

@base.vectorize
class ldmsint(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMSINT i n
         Assigns sregint register sr_i the value in memory SR[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMSINT']
    arg_format = ['srw', 'int']


@base.vectorize
class ldmsinti(base.ReadMemoryInstruction):
    r""" LDMSINTI i j
         Assigns sregint register sr_i the value in sint memory SR[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMSINTI']
    arg_format = ['srw', 'r']


@base.vectorize
class stmsint(base.DirectMemoryWriteInstruction):
    r""" STMSINT i n
         Sets memory SR[n] to be the value in sregint register sr_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMSINT']
    arg_format = ['sr', 'int']


@base.vectorize
class stmsinti(base.WriteMemoryInstruction):
    r""" STMSINTI i j
         Sets sregint memory SR[r_j] to be the value in sregint register sr_i, where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMSINTI']
    arg_format = ['sr', 'r']


## #########################END NEW OPCODES#########################
@base.vectorize
class ldmc(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMC i n
         Assigns cint register c_i the value in memory C[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMC']
    arg_format = ['cw', 'int']


@base.vectorize
class ldms(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMS i n
         Assigns sint register s_i the value in memory S[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMS']
    arg_format = ['sw', 'int']


@base.vectorize
class stmc(base.DirectMemoryWriteInstruction):
    r""" STMC i n
         Sets memory C[n] to be the value in cint register c_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMC']
    arg_format = ['c', 'int']


@base.vectorize
class stms(base.DirectMemoryWriteInstruction):
    r""" STMS i n
         Sets memory S[n] to be the value in sint register s_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMS']
    arg_format = ['s', 'int']


@base.vectorize
class ldmint(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    r""" LDMINT i n
         Assigns regint register r_i the value in memory R[n].
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['LDMINT']
    arg_format = ['rw', 'int']


@base.vectorize
class stmint(base.DirectMemoryWriteInstruction):
    r""" STMINT i n
         Sets regint memory R[n] to be the value r_i.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['STMINT']
    arg_format = ['r', 'int']


# must have seperate instructions because address is always modp
@base.vectorize
class ldmci(base.ReadMemoryInstruction):
    r""" LDMCI i j
         Assigns cint register c_i the value in cint memory R[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMCI']
    arg_format = ['cw', 'r']


@base.vectorize
class ldmsi(base.ReadMemoryInstruction):
    r""" LDMSI i j
         Assigns sint register s_i the value in sint memory S[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMSI']
    arg_format = ['sw', 'r']


@base.vectorize
class stmci(base.WriteMemoryInstruction):
    r""" STMCI i j
         Sets cint memory C[r_j] to be the value in cint register c_i, where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMCI']
    arg_format = ['c', 'r']


@base.vectorize
class stmsi(base.WriteMemoryInstruction):
    r""" STMSI i j
         Sets sint memory S[r_j] to be the value in sint register s_i, where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMSI']
    arg_format = ['s', 'r']


@base.vectorize
class ldminti(base.ReadMemoryInstruction):
    r""" LDMINTI i j
         Assigns regint register r_i the value in memory R[r_j], where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['LDMINTI']
    arg_format = ['rw', 'r']


@base.vectorize
class stminti(base.WriteMemoryInstruction):
    r""" STMINTI i j
         Sets regint memory R[r_j] to be the value in regint register r_i,  where r_j is the j-th regint register.
         This instruction is vectorizable
     """
    code = base.opcodes['STMINTI']
    arg_format = ['r', 'r']


@base.vectorize
class movc(base.Instruction):
    r""" MOVC i j
         Assigns cint register c_i the value in the cint register c_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVC']
    arg_format = ['cw', 'c']


@base.vectorize
class movs(base.Instruction):
    r""" MOVS i j
         Assigns sint register s_i the value in the sint register s_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVS']
    arg_format = ['sw', 's']


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
class movsb(base.Instruction):
    r""" MOVSB i j
         Assigns sbit register sb_i the value in the sbitint register sb_j.
         This instruction is vectorizable
     """
    __slots__ = ["code"]
    code = base.opcodes['MOVSB']
    arg_format = ['sbw', 'sb']


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
         This is also used to pass variables to functions.
         This instruction is vectorizable
     """
    code = base.opcodes['LDARG']
    arg_format = ['rw']


@base.vectorize
class starg(base.Instruction):
    r""" STARG i
         Assigns register r_i to variable in the thread argument.
         This is also used to pass variables to functions.
         This instruction is vectorizable
     """
    code = base.opcodes['STARG']
    arg_format = ['r']


class reqbl(base.Instruction):
    r""" REQBL n
         Signals tape has been built so that it requires prime bit length n".
     """
    code = base.opcodes['REQBL']
    arg_format = ['int']



class run_tape(base.Instruction):
    r""" RUN_TAPE i j k n
         In thread i start tape k with argument j.
         Execute with program counter starting at n
         In MAMBA we always assume n=0
     """
    code = base.opcodes['RUN_TAPE']
    arg_format = ['int', 'int', 'int', 'int']


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

class CALL(base.JumpInstruction):
    r""" CALL n
         Pushes the current PC onto the stack, and then performs an unconditional relative jump of n+1 instructions.
     """
    __slots__ = []
    code = base.opcodes['CALL']
    arg_format = ['int']
    jump_arg = 0

class RETURN(base.JumpInstruction):
    r""" RETURN
         Pops an integer off the stack, and sets the program counter
         to this value. Used to return from sub-routines executed
         by CALL
     """
    code = base.opcodes['RETURN']
    arg_format = []
    jump_arg = -1

class CALLR(base.JumpInstruction):
    r""" CALLR i
         Pushes the current PC onto the stack, and then jump to the
         address ri.
     """
    __slots__ = []
    code = base.opcodes['CALLR']
    arg_format = ['r']
    jump_arg = 0

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
class addc(base.Instruction):
    r""" ADDC i j k
         Adds clear registers c_i=c_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class adds(base.Instruction):
    r""" ADDS i j k
         Adds secret registers s_i=s_j+s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class addm(base.Instruction):
    r""" ADDM i j k
         Adds a clear register onto a secret one s_i=s_j+c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDM']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class subc(base.Instruction):
    r""" SUBC i j k
         Subtracts clear registers c_i=c_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class subs(base.Instruction):
    r""" SUBS i j k
         Subtracts secret registers s_i=s_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBS']
    arg_format = ['sw', 's', 's']


@base.vectorize
class subml(base.Instruction):
    r""" SUBML i j k
         Subtracts a clear register from a secret one s_i=s_j-c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBML']
    arg_format = ['sw', 's', 'c']


@base.vectorize
class submr(base.Instruction):
    r""" SUBMR i j k
         Subtracts a secret register from a clear one s_i=c_j-s_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBMR']
    arg_format = ['sw', 'c', 's']

# Multiplication/division

@base.vectorize
class mulc(base.Instruction):
    r""" MULC i j k
         Multiplication of clear registers c_i=c_j \cdot c_k.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class mulm(base.Instruction):
    r""" MULM i j k
         Multiplication of clear and secret registers s_i=s_j \cdot c_k.
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


@base.vectorize
class modc(base.Instruction):
    r""" MODC i j k
         Clear division with remainder c_i=c_j%c_k (after lifting to the integers).
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODC']
    arg_format = ['cw', 'c', 'c']


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


@base.vectorize
class orc(base.Instruction):
    r""" ORC i j k
         Lifts to integers, performs logical OR then converts back to modp, c_i = c_j or c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class xorc(base.Instruction):
    r""" XORC i j k
         Lifts to integers, performs logical OR then converts back to modp, c_i = c_j oplus c_k
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORC']
    arg_format = ['cw', 'c', 'c']


@base.vectorize
class notc(base.Instruction):
    r""" NOTC i j n
         This takes the integer representation of the clear register c_j, flips all the bits, adds 2^n and then converts back to clear modp register c_i.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['NOTC']
    arg_format = ['cw', 'c', 'int']


#
# Arithmetic with immediate values
#

@base.vectorize
class addci(base.Instruction):
    """ ADDCI i j n
        Addition of clear register to an immediate value c_i=c_j+n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDCI']
    arg_format = ['cw', 'c', 'i']
    op = '__add__'


@base.vectorize
class addsi(base.Instruction):
    """ ADDSI i j n
        Addition of secret register to an immediate value s_i=s_j+n.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ADDSI']
    arg_format = ['sw', 's', 'i']
    op = '__add__'


@base.vectorize
class subci(base.Instruction):
    r""" SUBCI i j n
         Subtraction of clear register by an immediate value c_i=c_j-n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCI']
    arg_format = ['cw', 'c', 'i']
    op = '__sub__'


@base.vectorize
class subsi(base.Instruction):
    r""" SUBSI i j n
         Subtraction of secret register by an immediate value s_i=s_j-n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSI']
    arg_format = ['sw', 's', 'i']
    op = '__sub__'


@base.vectorize
class subcfi(base.Instruction):
    r""" SUBCFI i j n
         Subtraction of clear register from an immediate value c_i=n-c_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBCFI']
    arg_format = ['cw', 'c', 'i']
    op = '__rsub__'


@base.vectorize
class subsfi(base.Instruction):
    r""" SUBSFI i j n
         Subtraction of secret register from an immediate value s_i=n-s_j.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SUBSFI']
    arg_format = ['sw', 's', 'i']
    op = '__rsub__'


@base.vectorize
class mulci(base.Instruction):
    r""" MULCI i j n
         Multiplication of clear register by immediate value c_i=c_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULCI']
    arg_format = ['cw', 'c', 'i']
    op = '__mul__'


@base.vectorize
class mulsi(base.Instruction):
    r""" MULSI i j n
         Multiplication of secret register by immediate value s_i=s_j \cdot n.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MULSI']
    arg_format = ['sw', 's', 'i']
    op = '__mul__'


@base.vectorize
class divci(base.Instruction):
    r""" DIVCI i j n
         Division of clear register by an immediate value c_i=c_j/n mod p.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DIVCI']
    arg_format = ['cw', 'c', 'i']


@base.vectorize
class modci(base.Instruction):
    r""" MODCI i j n
         Clear division with remainder c_i=c_j%n (after lifting to the integers) by an immediate
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['MODCI']
    arg_format = ['cw', 'c', 'i']
    op = '__mod__'

# Bitwise Operations with immediate values

@base.vectorize
class andci(base.Instruction):
    r""" ANDCI i j n
         Equivalent of ANDC with an immediate value c_i = c_j and n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ANDCI']
    arg_format = ['cw', 'c', 'i']
    op = '__and__'


@base.vectorize
class xorci(base.Instruction):
    r""" XORCI i j n
         Equivalent of XORC with an immediate value c_i = c_j oplus n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['XORCI']
    arg_format = ['cw', 'c', 'i']
    op = '__xor__'


@base.vectorize
class orci(base.Instruction):
    r""" ORCI i j n
         Equivalent of ORC with an immediate value c_i = c_j or n
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['ORCI']
    arg_format = ['cw', 'c', 'i']
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


@base.vectorize
class shrc(base.Instruction):
    r""" SHRC i j k
         Clear bitwise shift right of cint register c_i = c_j >> c_k,(after converting to integers)
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SHRC']
    arg_format = ['cw', 'c', 'c']


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


@base.vectorize
class eqzc(base.ClearShiftInstruction):
    r""" EQZC i j
         Sets cint register ci the value of (cj==0)
     """
    __slots__ = []
    code = base.opcodes['EQZC']
    arg_format = ['cw', 'c']

@base.vectorize
class ltzc(base.ClearShiftInstruction):
    r""" LTZC i j
         Sets cint register ci the value of (cj<0), i.e. whether the top bit of cj is set
     """
    __slots__ = []
    code = base.opcodes['LTZC']
    arg_format = ['cw', 'c']


#
# Data access instructions
#

@base.vectorize
class triple(base.DataInstruction, base.VarArgsInstruction):
    r""" TRIPLE n, s1,...,sn
         Load sint registers s_{3i+1}, s_{3i+2} and s_{3i+3} with the next 
         multiplication triple, for i=0,..,n/3-1
         This instruction is vectorizable
     """
    __slots__ = ['data_type']
    code = base.opcodes['TRIPLE']
    arg_format = itertools.repeat('sw')
    data_type = 'triple'


@base.vectorize
class bit(base.DataInstruction, base.VarArgsInstruction):
    r""" BIT n, s1,...,sn
         Load sint register s_i with the next secret bit.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['BIT']
    arg_format = itertools.repeat('sw')
    data_type = 'bit'

@base.vectorize
class dabit(base.DataInstruction):
    r""" DABIT i j
         Load sint, sbit registers s_i and sb_j with the next secret dabit.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['DABIT']
    arg_format = ['sw', 'sbw']
    data_type = 'dabit'

@base.vectorize
class square(base.DataInstruction, base.VarArgsInstruction):
    r""" SQUARE n, s1,...,sn
         Load sint registers s_{2i+1}, s_{2i+2} with the next square tuple,
         for i=0,..,n/2-1
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['SQUARE']
    arg_format = itertools.repeat('sw')
    data_type = 'square'


#
# I/O
#

@base.vectorize
class private_input(base.IOInstruction):
    r""" PRIVATE_INPUT i j k
         Private input from player r_j on channel r_k assign result to sint s_i
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['PRIVATE_INPUT']
    arg_format = ['sw', 'r','r']

@base.vectorize
class mprivate_input(base.IOInstruction):
    r""" MPRIVATE_INPUT i j k l 
         Private input of rj items from player rk on channel rl
         assign the result to sint memory [ri+t] for t=0...rj-1
         When vectorized ri increases, but rj stays the same.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['MPRIVATE_INPUT']
    arg_format = ['r', 'r',  'r','r']


@base.vectorize
class print_mem(base.IOInstruction):
    r""" PRINT_MEM i
         Print value in clear memory C[i] to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_MEM']
    arg_format = ['int']


@base.vectorize
class print_reg(base.IOInstruction):
    r""" PRINT_REG i
         As above but skips the comment j
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_REG']
    arg_format = ['c']

@base.vectorize
class print_fix(base.IOInstruction):
    r""" PRINT_FIX i f k
         Prints the fixed point number in cint register c_i using parameters f and k
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_FIX']
    arg_format = ['c', 'i', 'i']

@base.vectorize
class print_float(base.IOInstruction):
    r""" PRINT_FLOAT i j k l m
         Prints the floating point number in cint registers (c_i, c_j, c_k, c_l, c_e) assuming they map to the representation (v,p,z,s,e)
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_FLOAT']
    arg_format = ['c', 'c', 'c', 'c', 'c']

@base.vectorize
class print_int(base.IOInstruction):
    r""" PRINT_INT i
         Prints the value of register r_i to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_INT']
    arg_format = ['r']

@base.vectorize
class print_ieee_float(base.IOInstruction):
    r""" PRINT_IEEE_FLOAT i
         Prints the value of register r_i to debug IO channel as a double
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['PRINT_IEEE_FLOAT']
    arg_format = ['r']



class print_char(base.IOInstruction):
    r""" PRINT_CHAR i
         Prints a single character i to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINT_CHAR']
    arg_format = ['int']

    def __init__(self, ch):
        super(print_char, self).__init__(ord(ch))


class print_char4(base.IOInstruction):
    r""" PRINT_CHAR4 i
         Print a 4 character string i to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINT_CHAR4']
    arg_format = ['int']

    def __init__(self, val):
        super(print_char4, self).__init__(self.str_to_int(val))


@base.vectorize
class print_char_regint(base.IOInstruction):
    r""" PRINT_CHAR_REGINT i
         Print regint register r_i as a single character to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINT_CHAR_REGINT']
    arg_format = ['r']

@base.vectorize
class print_char4_regint(base.IOInstruction):
    r""" PRINTi_CHAR4_REGINT i
         Print regint register r_i as a four character string to debug IO channel.
         Can only be executed in thread zero.
         This instruction is vectorizable
     """
    code = base.opcodes['PRINT_CHAR4_REGINT']
    arg_format = ['r']

@base.vectorize
class input_clear(base.IOInstruction):
    r""" INPUT_CLEAR i j
         Gets cint public input c_i from the IO class on channel rj.
         Public inputs need to be the same for all players running the protocol, otherwise a crash will occur.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['INPUT_CLEAR']
    arg_format = ['cw','r']


@base.vectorize
class input_int(base.IOInstruction):
    r""" INPUT_INT i j
         Gets public regint input r_i from the IO class on channel rj.
         Public inputs need to be the same for all players running the protocol, otherwise a crash will occur.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['INPUT_INT']
    arg_format = ['rw','r']

class open_channel(base.IOInstruction):
    r""" OPEN_CHANNEL i j
         Opens channel number rj for reading/writing on the IO class.
         Channels are assumed to be bi-directional, i.e. can read and write.
         This is provided as some IO classes may require this to be called explicitly, the default one does not need this.
         The return value r_i *can* be some error code which the IO class may want to return.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OPEN_CHANNEL']
    arg_format = ['rw','r']

class close_channel(base.IOInstruction):
    r""" CLOSE_CHANNEL i
         Closes channel number ri for reading/writing on the IO class.
         This is provided as some IO classes may require this to be called explicitly, the default one does not need this.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['CLOSE_CHANNEL']
    arg_format = ['r']


@base.vectorize
class output_shares(base.IOInstruction):
    r""" OUTPUT_SHARES (n+1) ri i1 i2 ... in
         Write shares s_{i_j} to the IO class channel ri. This can be called from our MAMBA language using
               inp = [sint(1), sint(2), sint(3), sint(4)]
               output_shares(ch, *inp)
         Can only be executed in thread zero.
    """
    __slots__ = []
    code = base.opcodes['OUTPUT_SHARES']
    arg_format = tools.chain(['r'],itertools.repeat('s'))

    def has_var_args(self):
        return True

@base.vectorize
class input_shares(base.IOInstruction):
    r""" INPUT_SHARES (n+1) ri i1 i2 ... in
         Read shares s_{i_j} to the IO class channel ri. This can be called from our MAMBA language using
            inp = [sint(1), sint(2), sint(3), sint(4)]
            input_shares(ch,*inp)
         Can only be executed in thread zero.
    """
    __slots__ = []
    code = base.opcodes['INPUT_SHARES']
    arg_format = tools.chain(['r'],itertools.repeat('sw'))

    def has_var_args(self):
        return True

@base.vectorize
class output_clear(base.IOInstruction):
    r""" OUTPUT_CLEAR i j
         Public output of cint register c_i to IO class on channel rj.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OUTPUT_CLEAR']
    arg_format = ['c','r']

@base.vectorize
class output_int(base.IOInstruction):
    r""" OUTPUT_INT i j
         Public output of regint register r_i to IO class on channel rj.
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['OUTPUT_INT']
    arg_format = ['r','r']


@base.vectorize
class private_output(base.IOInstruction):
    r""" PRIVATE_OUTPUT i j k
         Private output to rj of the sint value s_i on channel rk 
         This instruction is vectorizable
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['PRIVATE_OUTPUT']
    arg_format = ['s', 'r', 'r']


@base.vectorize
class mprivate_output(base.IOInstruction):
    r""" MPRIVATE_OUTPUT i j k l 
         Private output of rj items from player rk on channel rl
         outputing the values in sint memory [ri+t] for t=0...rj-1
         When vectorized ri increases, but rj stays the same.
         Can only be executed in thread zero.
     """
    __slots__ = []
    code = base.opcodes['MPRIVATE_OUTPUT']
    arg_format = ['r', 'r', 'r','r']

  

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

@base.vectorize
class randc(base.Instruction):
    r""" RANDC i 
         Writes to the cint register c_i a random value mod p
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RANDC']
    arg_format = ['cw']

@base.vectorize
class randint(base.Instruction):
    r""" RANDINT i
         Writes to the regint register r_i a random value (mod 2^64)
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RANDINT']
    arg_format = ['rw']


@base.vectorize
class randsint(base.Instruction):
    r""" RANDSINT i
         Writes to the sregint register sr_i a random value (mod 2^64)
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RANDSINT']
    arg_format = ['srw']

@base.vectorize
class randfloat(base.Instruction):
    r""" RANDFLOAT i
         Writes to the regint register r_i a random value representing a float in the range [0,..,1]
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RANDFLOAT']
    arg_format = ['rw']

@base.vectorize
class randsbit(base.Instruction):
    r""" RANDSBIT i
         Writes to the sbit register sb_i a random value 
         The random value is the same for all players, so in particular it is not really random.
     """
    __slots__ = []
    code = base.opcodes['RANDSBIT']
    arg_format = ['sbw']

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
class addint(base.Instruction):
    r""" ADDINT i j k
         Addition regint registers r_i=r_j+r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['ADDINT']


@base.vectorize
class subint(base.Instruction):
    r""" SUBINT i j k
         Subtraction of regint registers r_i=r_j-r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['SUBINT']


@base.vectorize
class mulint(base.Instruction):
    r""" MULINT i j k
         Multiplication of regint registers r_i=r_j*r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['MULINT']


@base.vectorize
class divint(base.Instruction):
    r""" DIVINT i j k
         Division of regint registers r_i=r_j/r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['DIVINT']

@base.vectorize
class modint(base.Instruction):
    r""" MODINT i j k
         Modular remainder of regint registers r_i=r_j % r_k.
         This instruction is vectorizable
    """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['MODINT']


#
# Clear comparison instructions
#

@base.vectorize
class eqzint(base.Instruction):
    r""" EQZINT i j
         Clear comparison to zero test of regint registers r_i = (r_j == 0).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['rw', 'r']
    code = base.opcodes['EQZINT']


@base.vectorize
class ltzint(base.Instruction):
    r""" LTZINT i j
         Clear comparison of regint registers r_i = (r_j < 0).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['rw', 'r']
    code = base.opcodes['LTZINT']


@base.vectorize
class ltint(base.Instruction):
    r""" LTINT i j k
         Clear comparison of regint registers r_i = (r_j < r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['LTINT']


@base.vectorize
class gtint(base.Instruction):
    r""" GTINT i j k
         Clear comparison of regint registers r_i = (r_j > r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
    code = base.opcodes['GTINT']


@base.vectorize
class eqint(base.Instruction):
    r""" EQINT i j k
         Clear comparison of regint registers r_i = (r_j == r_k).
         This instruction is vectorizable
     """
    __slots__ = []
    arg_format = ['rw', 'r', 'r']
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

class jmpr(base.JumpInstruction):
    r""" JMPR i
         Unconditional jump to the absolute address ri.
     """
    __slots__ = []
    code = base.opcodes['JMPR']
    arg_format = ['r']
    jump_arg = 0


class jmpne(base.JumpInstruction):
    r""" JMPNE i j n
         Jump of n+1 instructions if regint register r_i \neq j.
         Example:
            jmpne(c, 0, n) : advance n+1 instructions if c is non-zero
            jmpne(c, 0, 0) : do nothing
            jmpne(c, 0, -1): infinite loop if c is non-zero
     """
    __slots__ = []
    code = base.opcodes['JMPNE']
    arg_format = ['r', 'int', 'int']
    jump_arg = 2


class jmpeq(base.JumpInstruction):
    r""" JMPEQ i j n
         Jump n+1 instructions if regint register r_i == j.
     """
    __slots__ = []
    code = base.opcodes['JMPEQ']
    arg_format = ['r', 'int', 'int']
    jump_arg = 2


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
    """ CONVMODP i j n
        Convert from cint register c_j to regint register r_i with
        bitlength of c_j equal to n
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


@base.vectorize
class stopopen(base.VarArgsInstruction):
    """ STOPOPEN n, c1, ..., cn
        Store previous opened value in cint register c_i.
        This instruction is vectorizable
     """
    __slots__ = []
    code = base.opcodes['STOPOPEN']
    arg_format = itertools.repeat('cw')


class start_clock(base.Instruction):
    r""" START_CLOCK n
         Re-initializes the specified timer n
     """
    code = base.opcodes['START_CLOCK']
    arg_format = ['i']

class stop_clock(base.Instruction):
    r""" STOP_CLOCK n
         Prints the time since the last initialization of timer n
     """
    code = base.opcodes['STOP_CLOCK']
    arg_format = ['i']

#
# Memory Management Instructions
#

@base.vectorize
class newc(base.ReadMemoryInstruction):
    r""" NEWC i j
         Allocated r_j locations of cint memory and returns
         the location in register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['NEWC']
    arg_format = ['rw', 'r']

@base.vectorize
class deletec(base.WriteMemoryInstruction):
    r""" DELETEC i
         Deletes the cint memory pointed to by regint register r_i
         This instruction is vectorizable
     """
    code = base.opcodes['DELETEC']
    arg_format = ['r']


@base.vectorize
class news(base.ReadMemoryInstruction):
    r""" NEWS i j
         Allocated r_j locations of sint memory and returns
         the location in register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['NEWS']
    arg_format = ['rw', 'r']

@base.vectorize
class deletes(base.WriteMemoryInstruction):
    r""" DELETES i
         Deletes the sint memory pointed to by regint register r_i
         This instruction is vectorizable
     """
    code = base.opcodes['DELETES']
    arg_format = ['r']

@base.vectorize
class newint(base.ReadMemoryInstruction):
    r""" NEWINT i j
         Allocated r_j locations of regint memory and returns
         the location in register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['NEWINT']
    arg_format = ['rw', 'r']

@base.vectorize
class deleteint(base.WriteMemoryInstruction):
    r""" DELETEINT i
         Deletes the regint memory pointed to by regint register r_i
         This instruction is vectorizable
     """
    code = base.opcodes['DELETEINT']
    arg_format = ['r']

@base.vectorize
class newsint(base.ReadMemoryInstruction):
    r""" NEWSINT i j
         Allocated r_j locations of sregint memory and returns
         the location in register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['NEWSINT']
    arg_format = ['rw', 'r']

@base.vectorize
class deletesint(base.WriteMemoryInstruction):
    r""" DELETESINT i
         Deletes the sregint memory pointed to by regint register r_i
         This instruction is vectorizable
     """
    code = base.opcodes['DELETESINT']
    arg_format = ['r']


#
# Stack Instructions
#

@base.vectorize
class pushint(base.StackInstruction):
    r""" PUSHINT i
         Pushes regint register r_i to the thread-local regint stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHINT']
    arg_format = ['r']

@base.vectorize
class popint(base.StackInstruction):
    r""" POPINT i
         Pops from the thread-local regint stack to regint register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPINT']
    arg_format = ['rw']

@base.vectorize
class peekint(base.StackInstruction):
    r""" PEEKINT i j
         Peeks at position pointed to by register r_j from the thread-local
         regint stack and assigns to regint register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['PEEKINT']
    arg_format = ['rw', 'r']

@base.vectorize
class pokeint(base.StackInstruction):
    r""" POKEINT i j
         Replaces the data item pointed to by register r_i on the thread-local regint
         local stack with the contents of register r_j
         This instruction is vectorizable
     """
    code = base.opcodes['POKEINT']
    arg_format = ['r', 'r']

@base.vectorize
class rpeekint(base.StackInstruction):
    r""" RPEEKINT i j
         Peeks at position pointed to by register stack_pointer - r_j from the thread-local
         regint stack and assigns to regint register r_i.
         This instruction is vectorizable
     """
    code = base.opcodes['RPEEKINT']
    arg_format = ['rw', 'r']

@base.vectorize
class rpokeint(base.StackInstruction):
    r""" RPOKEINT i j
         Replaces the data item pointed to by register stack_pointer - r_i on the thread-local regint
         local stack with the contents of register r_j
         This instruction is vectorizable
     """
    code = base.opcodes['RPOKEINT']
    arg_format = ['r', 'r']

class getspint(base.StackInstruction):
    r""" GETSPINT i
         Assigns the current stack pointer on the regint stack to register r_i
     """
    code = base.opcodes['GETSPINT']
    arg_format = ['rw']


@base.vectorize
class pushsint(base.StackInstruction):
    r""" PUSHSINT i
         Pushes sregint register sr_i to the thread-local sregint stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHSINT']
    arg_format = ['sr']

@base.vectorize
class popsint(base.StackInstruction):
    r""" POPSINT i
         Pops from the thread-local sregint stack to sregint register sr_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPSINT']
    arg_format = ['srw']

@base.vectorize
class peeksint(base.StackInstruction):
    r""" PEEKSINT i j
         Peeks at position pointed to by register r_j from the thread-local
         sregint stack and assigns to sregint register sr_i.
         This instruction is vectorizable
     """
    code = base.opcodes['PEEKSINT']
    arg_format = ['srw', 'r']

@base.vectorize
class pokesint(base.StackInstruction):
    r""" POKESINT i j
         Replaces the data item pointed to by register r_i on the thread-local  sregint
         local stack with the contents of register sr_j
         This instruction is vectorizable
     """
    code = base.opcodes['POKESINT']
    arg_format = ['r', 'sr']

@base.vectorize
class rpeeksint(base.StackInstruction):
    r""" RPEEKSINT i j
         Peeks at position pointed to by register stack_pointer - r_j from the thread-local
         sregint stack and assigns to sregint register sr_i.
         This instruction is vectorizable
     """
    code = base.opcodes['RPEEKSINT']
    arg_format = ['srw', 'r']

@base.vectorize
class rpokesint(base.StackInstruction):
    r""" RPOKESINT i j
         Replaces the data item pointed to by register stack_pointer - r_i on the thread-local  sregint
         local stack with the contents of register sr_j
         This instruction is vectorizable
     """
    code = base.opcodes['RPOKESINT']
    arg_format = ['r', 'sr']

class getspsint(base.StackInstruction):
    r""" GETSPSINT i
         Assigns the current stack pointer on the sregint stack to register r_i
     """
    code = base.opcodes['GETSPSINT']
    arg_format = ['rw']

@base.vectorize
class pushs(base.StackInstruction):
    r""" PUSHS i
         Pushes sint register s_i to the thread-local sint stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHS']
    arg_format = ['s']

@base.vectorize
class pops(base.StackInstruction):
    r""" POPS i
         Pops from the thread-local sint stack to sint register s_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPS']
    arg_format = ['sw']

@base.vectorize
class peeks(base.StackInstruction):
    r""" PEEKS i j
         Peeks at position pointed to by register r_j from the thread-local
         sint stack and assigns to sint register s_i.
         This instruction is vectorizable
     """
    code = base.opcodes['PEEKS']
    arg_format = ['sw', 'r']

@base.vectorize
class pokes(base.StackInstruction):
    r""" POKES i j
         Replaces the data item pointed to by register r_i on the thread-local  sint
         local stack with the contents of register s_j
         This instruction is vectorizable
     """
    code = base.opcodes['POKES']
    arg_format = ['r', 's']

@base.vectorize
class rpeeks(base.StackInstruction):
    r""" RPEEKS i j
         Peeks at position pointed to by register stack_pointer - r_j from the thread-local
         sint stack and assigns to sint register s_i.
         This instruction is vectorizable
     """
    code = base.opcodes['RPEEKS']
    arg_format = ['sw', 'r']

@base.vectorize
class rpokes(base.StackInstruction):
    r""" RPOKES i j
         Replaces the data item pointed to by register stack_pointer - r_i on the thread-local  sint
         local stack with the contents of register s_j
         This instruction is vectorizable
     """
    code = base.opcodes['RPOKES']
    arg_format = ['r', 's']

class getsps(base.StackInstruction):
    r""" GETSPS i
         Assigns the current stack pointer on the sint stack to register r_i
     """
    code = base.opcodes['GETSPS']
    arg_format = ['rw']


@base.vectorize
class pushc(base.StackInstruction):
    r""" PUSHC i
         Pushes cint register c_i to the thread-local cint stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHC']
    arg_format = ['c']

@base.vectorize
class popc(base.StackInstruction):
    r""" POPC i
         Pops from the thread-local cint stack to cint register c_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPC']
    arg_format = ['cw']

@base.vectorize
class peekc(base.StackInstruction):
    r""" PEEKC i j
         Peeks at position pointed to by register r_j from the thread-local
         cint stack and assigns to cint register c_i.
         This instruction is vectorizable
     """
    code = base.opcodes['PEEKC']
    arg_format = ['cw', 'r']

@base.vectorize
class pokec(base.StackInstruction):
    r""" POKEC i j
         Replaces the data item pointed to by register r_i on the thread-local  cint
         local stack with the contents of register c_j
         This instruction is vectorizable
     """
    code = base.opcodes['POKEC']
    arg_format = ['r', 'c']

@base.vectorize
class rpeekc(base.StackInstruction):
    r""" RPEEKC i j
         Peeks at position pointed to by register stack_pointer - r_j from the thread-local
         cint stack and assigns to cint register c_i.
         This instruction is vectorizable
     """
    code = base.opcodes['RPEEKC']
    arg_format = ['cw', 'r']

@base.vectorize
class rpokec(base.StackInstruction):
    r""" RPOKEC i j
         Replaces the data item pointed to by register stack_pointer - r_i on the thread-local  cint
         local stack with the contents of register c_j
         This instruction is vectorizable
     """
    code = base.opcodes['RPOKEC']
    arg_format = ['r', 'c']

class getspc(base.StackInstruction):
    r""" GETSPC i
         Assigns the current stack pointer on the cint stack to register r_i
     """
    code = base.opcodes['GETSPC']
    arg_format = ['rw']


@base.vectorize
class pushsbit(base.StackInstruction):
    r""" PUSHSBIT i
         Pushes sbit register sb_i to the thread-local sbit stack.
         This instruction is vectorizable
     """
    code = base.opcodes['PUSHSBIT']
    arg_format = ['sb']

@base.vectorize
class popsbit(base.StackInstruction):
    r""" POPSBIT i
         Pops from the thread-local sbit stack to sbit register sb_i.
         This instruction is vectorizable
     """
    code = base.opcodes['POPSBIT']
    arg_format = ['sbw']

@base.vectorize
class peeksbit(base.StackInstruction):
    r""" PEEKSBIT i j
         Peeks at position pointed to by register r_j from the thread-local
         sbit stack and assigns to sbit register sb_i.
         This instruction is vectorizable
     """
    code = base.opcodes['PEEKSBIT']
    arg_format = ['sbw', 'r']

@base.vectorize
class pokesbit(base.StackInstruction):
    r""" POKESBIT i j
         Replaces the data item pointed to by register r_i on the thread-local  sbit
         local stack with the contents of register sb_j
         This instruction is vectorizable
     """
    code = base.opcodes['POKESBIT']
    arg_format = ['r', 'sb']

@base.vectorize
class rpeeksbit(base.StackInstruction):
    r""" RPEEKSBIT i j
         Peeks at position pointed to by register stack_pointer - r_j from the thread-local
         sbit stack and assigns to sbit register sb_i.
         This instruction is vectorizable
     """
    code = base.opcodes['RPEEKSBIT']
    arg_format = ['sbw', 'r']

@base.vectorize
class rpokesbit(base.StackInstruction):
    r""" RPOKESBIT i j
         Replaces the data item pointed to by register stack_pointer - r_i on the thread-local  sbit
         local stack with the contents of register sb_j
         This instruction is vectorizable
     """
    code = base.opcodes['RPOKESBIT']
    arg_format = ['r', 'sb']

class getspsbit(base.StackInstruction):
    r""" GETSPSBIT i
         Assigns the current stack pointer on the sbit stack to register r_i
     """
    code = base.opcodes['GETSPSBIT']
    arg_format = ['rw']


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
        from AdvInteger import LTZ
        a = program.curr_block.new_reg('s')
        subs(a, self.args[1], self.args[2])
        LTZ(self.args[0], a, self.args[3], self.args[4])

