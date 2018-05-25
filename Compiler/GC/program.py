from Compiler import types, instructions

class Program(object):
    def __init__(self, progname):
        types.program = self
        instructions.program = self
        self.curr_tape = None
        execfile(progname)
    def malloc(self, *args):
        pass
