
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

class CompilerError(Exception):
    """Base class for compiler exceptions."""
    pass

class RegisterOverflowError(CompilerError):
    pass

class MemoryOverflowError(CompilerError):
    pass

class ArgumentError(CompilerError):
    """ Exception raised for errors in instruction argument parsing. """
    def __init__(self, arg, msg):
        self.arg = arg
        self.msg = msg