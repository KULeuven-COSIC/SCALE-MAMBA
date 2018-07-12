import Compiler.instructions_base as base
import Compiler.instructions as spdz
import Compiler.tools as tools
import collections
import itertools

class SecretBitsAF(base.RegisterArgFormat):
    reg_type = 'sb'
class ClearBitsAF(base.RegisterArgFormat):
    reg_type = 'cb'

base.ArgFormats['sb'] = SecretBitsAF
base.ArgFormats['sbw'] = SecretBitsAF
base.ArgFormats['cb'] = ClearBitsAF
base.ArgFormats['cbw'] = ClearBitsAF

opcodes = dict(
    XORS = 0x200,
    XORM = 0x201,
    ANDRS = 0x202,
    BITDECS = 0x203,
    BITCOMS = 0x204,
    CONVSINT = 0x205,
    LDMSDI = 0x206,
    STMSDI = 0x207,
    LDMSD = 0x208,
    STMSD = 0x209,
    XORCI = 0x210,
    BITDECC = 0x211,
    CONVCINT = 0x213,
    REVEAL = 0x214,
)

class xors(base.Instruction):
    code = opcodes['XORS']
    arg_format = ['int','sbw','sb','sb']

class xorm(base.Instruction):
    code = opcodes['XORM']
    arg_format = ['int','sbw','sb','cb']

class xorci(base.Instruction):
    code = opcodes['XORCI']
    arg_format = ['cbw','cb','int']

class andrs(base.Instruction):
    code = opcodes['ANDRS']
    arg_format = ['int','sbw','sb','sb']

class mulci(base.Instruction):
    code = base.opcodes['MULCI']
    arg_format = ['cbw','cb','int']

class bitdecs(base.VarArgsInstruction):
    code = opcodes['BITDECS']
    arg_format = tools.chain(['sb'], itertools.repeat('sbw'))

class bitcoms(base.VarArgsInstruction):
    code = opcodes['BITCOMS']
    arg_format = tools.chain(['sbw'], itertools.repeat('sb'))

class bitdecc(base.VarArgsInstruction):
    code = opcodes['BITDECC']
    arg_format = tools.chain(['cb'], itertools.repeat('cbw'))

class shrci(base.Instruction):
    code = base.opcodes['SHRCI']
    arg_format = ['cbw','cb','int']

class ldsi(base.Instruction):
    code = base.opcodes['LDSI']
    arg_format = ['sbw','i']

class ldms(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = base.opcodes['LDMS']
    arg_format = ['sbw','int']

class stms(base.DirectMemoryWriteInstruction):
    code = base.opcodes['STMS']
    arg_format = ['sb','int']
    # def __init__(self, *args, **kwargs):
    #     super(type(self), self).__init__(*args, **kwargs)
    #     import inspect
    #     self.caller = [frame[1:] for frame in inspect.stack()[1:]]

class ldmc(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = base.opcodes['LDMC']
    arg_format = ['cbw','int']

class stmc(base.DirectMemoryWriteInstruction):
    code = base.opcodes['STMC']
    arg_format = ['cb','int']

class ldmsi(base.ReadMemoryInstruction):
    code = base.opcodes['LDMSI']
    arg_format = ['sbw','r']

class stmsi(base.WriteMemoryInstruction):
    code = base.opcodes['STMSI']
    arg_format = ['sb','r']

class ldmsdi(base.ReadMemoryInstruction):
    code = opcodes['LDMSDI']
    arg_format = ['sbw','cb']

class stmsdi(base.WriteMemoryInstruction):
    code = opcodes['STMSDI']
    arg_format = ['sb','cb']

class ldmsd(base.ReadMemoryInstruction):
    code = opcodes['LDMSD']
    arg_format = ['sbw','int']

class stmsd(base.WriteMemoryInstruction):
    code = opcodes['STMSD']
    arg_format = ['sb','int']

class convsint(base.Instruction):
    code = opcodes['CONVSINT']
    arg_format = ['sbw','r']

class convcint(base.Instruction):
    code = opcodes['CONVCINT']
    arg_format = ['cbw','r']

class movs(base.Instruction):
    code = base.opcodes['MOVS']
    arg_format = ['sbw','sb']

class bit(base.Instruction):
    code = base.opcodes['BIT']
    arg_format = ['sbw']

class reveal(base.Instruction):
    code = opcodes['REVEAL']
    arg_format = ['int','cbw','sb']

class print_reg(base.IOInstruction):
    code = base.opcodes['PRINTREG']
    arg_format = ['cb','i']
    def __init__(self, reg, comment=''):
        super(print_reg, self).__init__(reg, self.str_to_int(comment))

class print_reg_plain(base.IOInstruction):
    code = base.opcodes['PRINTREGPLAIN']
    arg_format = ['cb']
