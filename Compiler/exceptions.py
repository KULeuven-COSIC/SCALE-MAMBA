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