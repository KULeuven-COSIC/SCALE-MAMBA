import Compiler.compilerLib
import Compiler.program
import Compiler.instructions
import Compiler.types
import Compiler.library
import Compiler.floatingpoint
import Compiler.mpc_math
import inspect
from Compiler.config import *
from Compiler.compilerLib import run


# add all instructions to the program VARS dictionary
compilerLib.VARS = {}
instr_classes = [t[1] for t in inspect.getmembers(Compiler.instructions, inspect.isclass)]

instr_classes += [t[1] for t in inspect.getmembers(Compiler.types, inspect.isclass)\
    if t[1].__module__ == Compiler.types.__name__]

#instr_classes += [t[1] for t in inspect.getmembers(fixedpoint, inspect.isclass)\
#    if t[1].__module__ == fixedpoint.__name__]
#instr_classes += [t[1] for t in inspect.getmembers(fixedpoint, inspect.isfunction)\
#    if t[1].__module__ == fixedpoint.__name__]

instr_classes += [t[1] for t in inspect.getmembers(Compiler.library, inspect.isfunction)\
    if t[1].__module__ == Compiler.library.__name__]

for op in instr_classes:
    Compiler.compilerLib.VARS[op.__name__] = op

# add open and input separately due to name conflict
Compiler.compilerLib.VARS['open'] = Compiler.instructions.asm_open
Compiler.compilerLib.VARS['vopen'] = Compiler.instructions.vasm_open

Compiler.compilerLib.VARS['comparison'] = Compiler.comparison
Compiler.compilerLib.VARS['mpc_math'] = Compiler.mpc_math
Compiler.compilerLib.VARS['floatingpoint'] = Compiler.floatingpoint
