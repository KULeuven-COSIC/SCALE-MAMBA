from __future__ import print_function
from past.builtins import execfile
from Compiler.program import Program
from Compiler.config import *
from Compiler.exceptions import *
import instructions, instructions_base, types, comparison, library

import random
import time
import sys


def run(args, options, param=-1, merge_opens=True, \
            reallocate=True, debug=False):
    """ Compile a file and output a Program object.
    
    If merge_opens is set to True, will attempt to merge any parallelisable open
    instructions. """
    
    prog = Program(args, options, param)
    instructions.program = prog
    instructions_base.program = prog
    types.program = prog
    comparison.program = prog
    prog.DEBUG = debug
    VARS['program'] = prog
    comparison.set_variant(options)
    
    print('Compiling file', prog.infile)
    
    if instructions_base.Instruction.count != 0:
        print('instructions count', instructions_base.Instruction.count)
        instructions_base.Instruction.count = 0
    prog.FIRST_PASS = False
    prog.reset_values()
    # make compiler modules directly accessible
    sys.path.insert(0, 'Compiler')
    # create the tapes
    execfile(prog.infile, VARS)
    
    # optimize the tapes
    for tape in prog.tapes:
        tape.optimize(options)
    
    if prog.main_thread_running:
        prog.update_req(prog.curr_tape)
    print('Program requires:', repr(prog.req_num))
    print('Memory size:', prog.allocated_mem)

    # finalize the memory
    prog.finalize_memory()

    return prog
