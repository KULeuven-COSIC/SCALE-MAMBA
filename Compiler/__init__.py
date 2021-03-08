
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

import compilerLib, program, instructions, types, library, floatingpoint, mpc_math
import inspect
from config import *
from compilerLib import run


# add all instructions to the program VARS dictionary
compilerLib.VARS = {}
instr_classes = [t[1] for t in inspect.getmembers(instructions, inspect.isclass)]

instr_classes += [t[1] for t in inspect.getmembers(types, inspect.isclass)\
    if t[1].__module__ == types.__name__]

#instr_classes += [t[1] for t in inspect.getmembers(fixedpoint, inspect.isclass)\
#    if t[1].__module__ == fixedpoint.__name__]
#instr_classes += [t[1] for t in inspect.getmembers(fixedpoint, inspect.isfunction)\
#    if t[1].__module__ == fixedpoint.__name__]

instr_classes += [t[1] for t in inspect.getmembers(library, inspect.isfunction)\
    if t[1].__module__ == library.__name__]

for op in instr_classes:
    compilerLib.VARS[op.__name__] = op

# add open and input separately due to name conflict
compilerLib.VARS['open'] = instructions.asm_open
compilerLib.VARS['vopen'] = instructions.vasm_open
compilerLib.VARS['AdvInteger'] = AdvInteger
compilerLib.VARS['mpc_math'] = mpc_math
compilerLib.VARS['floatingpoint'] = floatingpoint
