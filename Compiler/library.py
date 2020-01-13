from __future__ import print_function
from __future__ import division
from future import standard_library
standard_library.install_aliases()
from builtins import zip
from builtins import str
from builtins import range
from past.utils import old_div
from builtins import object
from Compiler.types import cint,sint,cfix,sfix,sfloat,cfloat,MPCThread,Array,MemValue,_number,_mem,_register,regint,Matrix,_types
from Compiler.instructions import *
from Compiler.util import tuplify,untuplify
from Compiler import instructions,instructions_base,comparison,program
import inspect,math
import random
import collections

def get_program():
    return instructions.program
def get_tape():
    return get_program().curr_tape
def get_block():
    return get_program().curr_block

def vectorize(function):
    def vectorized_function(*args, **kwargs):
        if len(args) > 0 and isinstance(args[0], program.Tape.Register):
            instructions_base.set_global_vector_size(args[0].size)
            res = function(*args, **kwargs)
            instructions_base.reset_global_vector_size()
        elif 'size' in kwargs:
            instructions_base.set_global_vector_size(kwargs['size'])
            del kwargs['size']
            res = function(*args, **kwargs)
            instructions_base.reset_global_vector_size()
        else:
            res = function(*args, **kwargs)
        return res
    vectorized_function.__name__ = function.__name__
    return vectorized_function

def set_instruction_type(function):
    def instruction_typed_function(*args, **kwargs):
        if len(args) > 0 and isinstance(args[0], program.Tape.Register):
            instructions_base.set_global_instruction_type('modp')                
            res = function(*args, **kwargs)
            instructions_base.reset_global_instruction_type()
        else:
            res = function(*args, **kwargs)
        return res
    instruction_typed_function.__name__ = function.__name__
    return instruction_typed_function


def print_str(s, *args):
    """ Print a string, with optional args for adding variables/registers with %s """
    def print_plain_str(ss):
        """ Print a plain string (no custom formatting options) """
        i = 1
        while 4*i < len(ss):
            print_char4(ss[4*(i-1):4*i])
            i += 1
        i = 4*(i-1)
        while i < len(ss):
            print_char(ss[i])
            i += 1

    if len(args) != s.count('%s'):
        raise CompilerError('Incorrect number of arguments for string format:', s)
    substrings = s.split('%s')
    for i,ss in enumerate(substrings):
        print_plain_str(ss)
        if i < len(args):
            if isinstance(args[i], MemValue):
                val = args[i].read()
            else:
                val = args[i]
            if isinstance(val, program.Tape.Register):
                if val.is_clear:
                    val.print_reg()
                else:
                    raise CompilerError('Cannot print secret value:', args[i])
            elif isinstance(val, sfix) or isinstance(val, sfloat):
                raise CompilerError('Cannot print secret value:', args[i])
            elif isinstance(val, cfloat):
                val.print_float()
            elif isinstance(val, cfix):
                val.print_fix()
            elif isinstance(val, list):
                print_str('[' + ', '.join('%s' for i in range(len(val))) + ']', *val)
            else:
                try:
                    val.output()
                except AttributeError:
                    print_plain_str(str(val))

def print_ln(s='', *args):
    """ Print line, with optional args for adding variables/registers with %s """
    print_str(s, *args)
    print_char('\n')

def runtime_error(msg='', *args):
    """ Print an error message and abort the runtime. """
    print_str('User exception: ')
    print_ln(msg, *args)
    crash()

# mostly obsolete functions
# use the equivalent from types.py

def load_int(value, size=None):
    return regint(value, size=size)

def load_int_to_secret(value, size=None):
    return sint(value, size=size)

def load_int_to_secret_vector(vector):
    res = sint(size=len(vector))
    for i,val in enumerate(vector):
        ldsi(res[i], val)
    return res

@vectorize
def load_float_to_secret(value, sec=40):
    def _bit_length(x):
        return len(bin(x).lstrip('-0b'))
    
    num,den = value.as_integer_ratio()
    exp = int(round(math.log(den, 2)))
    
    nbits = _bit_length(num)
    if nbits > sfloat.vlen:
        num >>= (nbits - sfloat.vlen)
        exp -= (nbits - sfloat.vlen)
    elif nbits < sfloat.vlen:
        num <<= (sfloat.vlen - nbits)
        exp += (sfloat.vlen - nbits)
    
    if _bit_length(exp) > sfloat.plen:
        raise CompilerException('Cannot load floating point to secret: overflow')
    if num < 0:
        s = load_int_to_secret(1)
        z = load_int_to_secret(0)
    else:
        s = load_int_to_secret(0)
        if num == 0:
            z = load_int_to_secret(1)
        else:
            z = load_int_to_secret(0)
    v = load_int_to_secret(num)
    p = load_int_to_secret(exp)
    err = load_int_to_secret(err)
    return sfloat(v, p, s, z, err)

def load_clear_mem(address):
    return cint.load_mem(address)

def load_secret_mem(address):
    return sint.load_mem(address)

def load_mem(address, value_type):
    if value_type in _types:
        value_type = _types[value_type]
    return value_type.load_mem(address)

@vectorize
def store_in_mem(value, address):
    if isinstance(value, int):
        value = load_int(value)
    try:
        value.store_in_mem(address)
    except AttributeError:
        # legacy
        if value.is_clear:
            if isinstance(address, cint):
                stmci(value, address)
            else:
                stmc(value, address)
        else:
            if isinstance(address, cint):
                stmsi(value, address)
            else:
                stms(value, address)

@set_instruction_type
@vectorize
def reveal(secret):
    try:
        return secret.reveal()
    except AttributeError:
        res = cint()
        instructions.asm_open(res, secret)
        return res

@vectorize
def compare_secret(a, b, length, sec=40):
    res = sint()
    instructions.lts(res, a, b, length, sec)

def get_random_triple(size=None):
    return sint.get_random_triple(size=size)

def get_random_bit(size=None):
    return sint.get_random_bit(size=size)

def get_random_square(size=None):
    return sint.get_random_square(size=size)

def get_random_int(bits, size=None):
    return sint.get_random_int(bits, size=size)

@vectorize
def get_thread_number():
    res = regint()
    ldtn(res)
    return res

@vectorize
def get_arg():
    res = regint()
    ldarg(res)
    return res

def make_array(l):
    if isinstance(l, program.Tape.Register):
        res = Array(1, type(l))
        res[0] = l
    else:
        l = list(l)
        res = Array(len(l), type(l[0]) if l else cint)
        res.assign(l)
    return res


class FunctionTapeCall(object):
    def __init__(self, thread, base, bases):
        self.thread = thread
        self.base = base
        self.bases = bases
    def start(self):
        self.thread.start(self.base)
        return self
    def join(self):
        self.thread.join()
        instructions.program.free(self.base, 'r')
        for reg_type,addr in self.bases.items():
            get_program().free(addr, reg_type.reg_type)

class Function(object):
    def __init__(self, function, name=None, compile_args=[]):
        self.type_args = {}
        self.function = function
        self.name = name
        if name is None:
            self.name = self.function.__name__ + '-' + str(id(function))
        self.compile_args = compile_args
    def __call__(self, *args):
        args = tuple(arg.read() if isinstance(arg, MemValue) else arg for arg in args)
        get_reg_type = lambda x: regint if isinstance(x, (int, int)) else type(x)
        if len(args) not in self.type_args:
            # first call
            type_args = collections.defaultdict(list)
            for i,arg in enumerate(args):
                type_args[get_reg_type(arg)].append(i)
            def wrapped_function(*compile_args):
                base = get_arg()
                bases = dict((t, regint.load_mem(base + i)) \
                                 for i,t in enumerate(type_args))
                runtime_args = [None] * len(args)
                for t,i_args in type_args.items():
                    for i,i_arg in enumerate(i_args):
                        runtime_args[i_arg] = t.load_mem(bases[t] + i)
                return self.function(*(list(compile_args) + runtime_args))
            self.on_first_call(wrapped_function)
            self.type_args[len(args)] = type_args
        type_args = self.type_args[len(args)]
        base = instructions.program.malloc(len(type_args), 'r')
        bases = dict((t, get_program().malloc(len(type_args[t]), t)) \
                         for t in type_args)
        for i,reg_type in enumerate(type_args):
            store_in_mem(bases[reg_type], base + i)
            for j,i_arg in enumerate(type_args[reg_type]):
                if get_reg_type(args[i_arg]) != reg_type:
                    raise CompilerError('type mismatch')
                store_in_mem(args[i_arg], bases[reg_type] + j)
        return self.on_call(base, bases)

class FunctionTape(Function):
    # not thread-safe
    def on_first_call(self, wrapped_function):
        self.thread = MPCThread(wrapped_function, self.name,
                                args=self.compile_args)
    def on_call(self, base, bases):
        return FunctionTapeCall(self.thread, base, bases)

def function_tape(function):
    return FunctionTape(function)

def function_tape_with_compile_args(*args):
    def wrapper(function):
        return FunctionTape(function, compile_args=args)
    return wrapper

def memorize(x):
    if isinstance(x, (tuple, list)):
        return tuple(memorize(i) for i in x)
    else:
        return MemValue(x)

def unmemorize(x):
    if isinstance(x, (tuple, list)):
        return tuple(unmemorize(i) for i in x)
    else:
        return x.read()

class FunctionBlock(Function):
    def on_first_call(self, wrapped_function):
        old_block = get_tape().active_basicblock
        parent_node = get_tape().req_node
        get_tape().open_scope(lambda x: x[0], None, 'begin-' + self.name)
        block = get_tape().active_basicblock
        block.alloc_pool = defaultdict(set)
        del parent_node.children[-1]
        self.node = get_tape().req_node
        print('Compiling function', self.name)
        result = wrapped_function(*self.compile_args)
        if result is not None:
            self.result = memorize(result)
        else:
            self.result = None
        print('Done compiling function', self.name)
        p_return_address = get_tape().program.malloc(1, 'r')
        get_tape().function_basicblocks[block] = p_return_address
        get_tape().active_basicblock.set_exit(instructions.RETURN(add_to_prog=False))
        self.last_sub_block = get_tape().active_basicblock
        get_tape().close_scope(old_block, parent_node, 'end-' + self.name)
        old_block.set_exit(instructions.jmp(0, add_to_prog=False), get_tape().active_basicblock)
        self.basic_block = block

    def on_call(self, base, bases):
        if base is not None:
            instructions.starg(regint(base))
        block = self.basic_block
        if block not in get_tape().function_basicblocks:
            raise CompilerError('unknown function')
        old_block = get_tape().active_basicblock
        old_block.set_exit(instructions.CALL(0, add_to_prog=False), block)
        p_return_address = get_tape().function_basicblocks[block]
        return_address = get_tape().new_reg('r')
        get_tape().start_new_basicblock(name='call-' + self.name)
        get_tape().active_basicblock.set_return(old_block, self.last_sub_block)
        get_tape().req_node.children.append(self.node)
        if self.result is not None:
            return unmemorize(self.result)

def function_block(function):
    return FunctionBlock(function)

def function_block_with_compile_args(*args):
    def wrapper(function):
        return FunctionBlock(function, compile_args=args)
    return wrapper

def method_block(function):
    # If you use this, make sure to use MemValue for all member
    # variables.
    compiled_functions = {}
    def wrapper(self, *args):
        if self in compiled_functions:
            return compiled_functions[self](*args)
        else:
            name = '%s-%s-%d' % (type(self).__name__, function.__name__, \
                                      id(self))
            block = FunctionBlock(function, name=name, compile_args=(self,))
            compiled_functions[self] = block
            return block(*args)
    return wrapper

def cond_swap(x,y):
    b = x < y
    if isinstance(x, sfloat):
        res = ([], [])
        for i,j in enumerate(('v','p','z','s', 'err')):
            xx = x.__getattribute__(j)
            yy = y.__getattribute__(j)
            bx = b * xx
            by = b * yy
            res[0].append(bx + yy - by)
            res[1].append(xx - bx + by)
        return sfloat(*res[0]), sfloat(*res[1])
    bx = b * x
    by = b * y
    return bx + y - by, x - bx + by

def sort(a):
    res = a
    
    for i in range(len(a)):
        for j in reversed(list(range(i))):
            res[j], res[j+1] = cond_swap(res[j], res[j+1])

    return res

def odd_even_merge(a):
    if len(a) == 2:
        a[0], a[1] = cond_swap(a[0], a[1])
    else:
        even = a[::2]
        odd = a[1::2]
        odd_even_merge(even)
        odd_even_merge(odd)
        a[0] = even[0]
        for i in range(1, old_div(len(a), 2)):
            a[2*i-1], a[2*i] = cond_swap(odd[i-1], even[i])
        a[-1] = odd[-1]

def odd_even_merge_sort(a):
    if len(a) == 1:
        return
    elif len(a) % 2 == 0:
        lower = a[:old_div(len(a),2)]
        upper = a[old_div(len(a),2):]
        odd_even_merge_sort(lower)
        odd_even_merge_sort(upper)
        a[:] = lower + upper
        odd_even_merge(a)
    else:
        raise CompilerError('Length of list must be power of two')

def chunky_odd_even_merge_sort(a):
    for i,j in enumerate(a):
        j.store_in_mem(i * j.sizeof())
    l = 1
    while l < len(a):
        l *= 2
        k = 1
        while k < l:
            k *= 2
            def round():
                for i in range(len(a)):
                    a[i] = type(a[i]).load_mem(i * a[i].sizeof())
                for i in range(old_div(len(a), l)):
                    for j in range(old_div(l, k)):
                        base = i * l + j
                        step = old_div(l, k)
                        if k == 2:
                            a[base], a[base+step] = cond_swap(a[base], a[base+step])
                        else:
                            b = a[base:base+k*step:step]
                            for m in range(base + step, base + (k - 1) * step, 2 * step):
                                a[m], a[m+step] = cond_swap(a[m], a[m+step])
                for i in range(len(a)):
                    a[i].store_in_mem(i * a[i].sizeof())
            chunk = MPCThread(round, 'sort-%d-%d-%03x' % (l,k,random.randrange(256**3)))
            chunk.start()
            chunk.join()
            #round()
    for i in range(len(a)):
        a[i] = type(a[i]).load_mem(i * a[i].sizeof())

def chunkier_odd_even_merge_sort(a, n=None, max_chunk_size=512, n_threads=7, use_chunk_wraps=False):
    if n is None:
        n = len(a)
        a_base = instructions.program.malloc(n, 's')
        for i,j in enumerate(a):
            store_in_mem(j, a_base + i)
        instructions.program.restart_main_thread()
    else:
        a_base = a
    tmp_base = instructions.program.malloc(n, 's')
    chunks = {}
    threads = []

    def run_threads():
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        del threads[:]

    def run_chunk(size, base):
        if size not in chunks:
            def swap_list(list_base):
                for i in range(old_div(size, 2)):
                    base = list_base + 2 * i
                    x, y = cond_swap(load_secret_mem(base),
                                     load_secret_mem(base + 1))
                    store_in_mem(x, base)
                    store_in_mem(y, base + 1)
            chunks[size] = FunctionTape(swap_list, 'sort-%d-%03x' %
                                        (size, random.randrange(256**3)))
        return chunks[size](base)

    def run_round(size):
        # minimize number of chunk sizes
        n_chunks = int(math.ceil(1.0 * size / max_chunk_size))
        lower_size = old_div(size, n_chunks / 2 * 2)
        n_lower_size = n_chunks - old_div((size - n_chunks * lower_size), 2)
        # print len(to_swap) == lower_size * n_lower_size + \
        #     (lower_size + 2) * (n_chunks - n_lower_size), \
        #     len(to_swap), n_chunks, lower_size, n_lower_size
        base = 0
        round_threads = []
        for i in range(n_lower_size):
            round_threads.append(run_chunk(lower_size, tmp_base + base))
            base += lower_size
        for i in range(n_chunks - n_lower_size):
            round_threads.append(run_chunk(lower_size + 2, tmp_base + base))
            base += lower_size + 2
        run_threads_in_rounds(round_threads)

    postproc_chunks = []
    wrap_chunks = {}
    post_threads = []
    pre_threads = []

    def load_and_store(x, y, to_right):
        if to_right:
            store_in_mem(load_secret_mem(x), y)
        else:
            store_in_mem(load_secret_mem(y), x)

    def run_setup(k, a_addr, step, tmp_addr):
        if k == 2:
            def mem_op(preproc, a_addr, step, tmp_addr):
                load_and_store(a_addr, tmp_addr, preproc)
                load_and_store(a_addr + step, tmp_addr + 1, preproc)
            res = 2
        else:
            def mem_op(preproc, a_addr, step, tmp_addr):
                instructions.program.curr_tape.merge_opens = False
#                for i,m in enumerate(range(a_addr + step, a_addr + (k - 1) * step, step)):
                for i in range(k - 2):
                    m = a_addr + step + i * step
                    load_and_store(m, tmp_addr + i, preproc)
            res = k - 2
        if not use_chunk_wraps or k <= 4:
            mem_op(True, a_addr, step, tmp_addr)
            postproc_chunks.append((mem_op, (a_addr, step, tmp_addr)))
        else:
            if k not in wrap_chunks:
                pre_chunk = FunctionTape(mem_op, 'pre-%d-%03x' % (k,random.randrange(256**3)),
                                         compile_args=[True])
                post_chunk = FunctionTape(mem_op, 'post-%d-%03x' % (k,random.randrange(256**3)),
                                          compile_args=[False])
                wrap_chunks[k] = (pre_chunk, post_chunk)
            pre_chunk, post_chunk = wrap_chunks[k]
            pre_threads.append(pre_chunk(a_addr, step, tmp_addr))
            post_threads.append(post_chunk(a_addr, step, tmp_addr))
        return res

    def run_threads_in_rounds(all_threads):
        for thread in all_threads:
            if len(threads) == n_threads:
                run_threads()
            threads.append(thread)
        run_threads()
        del all_threads[:]

    def run_postproc():
        run_threads_in_rounds(post_threads)
        for chunk,args in postproc_chunks:
            chunk(False, *args)
        postproc_chunks[:] = []

    l = 1
    while l < n:
        l *= 2
        k = 1
        while k < l:
            k *= 2
            size = 0
            instructions.program.curr_tape.merge_opens = False
            for i in range(old_div(n, l)):
                for j in range(old_div(l, k)):
                    base = i * l + j
                    step = old_div(l, k)
                    size += run_setup(k, a_base + base, step, tmp_base + size)
            run_threads_in_rounds(pre_threads)
            run_round(size)
            run_postproc()

    if isinstance(a, list):
        instructions.program.restart_main_thread()
        for i in range(n):
            a[i] = load_secret_mem(a_base + i)
        instructions.program.free(a_base, 's')
    instructions.program.free(tmp_base, 's')

def loopy_chunkier_odd_even_merge_sort(a, n=None, max_chunk_size=512, n_threads=7):
    if n is None:
        n = len(a)
        a_base = instructions.program.malloc(n, 's')
        for i,j in enumerate(a):
            store_in_mem(j, a_base + i)
        instructions.program.restart_main_thread()
    else:
        a_base = a
    tmp_base = instructions.program.malloc(n, 's')
    tmp_i = instructions.program.malloc(1, 'r')
    chunks = {}
    threads = []

    def run_threads():
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        del threads[:]

    def run_threads_in_rounds(all_threads):
        for thread in all_threads:
            if len(threads) == n_threads:
                run_threads()
            threads.append(thread)
        run_threads()
        del all_threads[:]

    def run_chunk(size, base):
        if size not in chunks:
            def swap_list(list_base):
                for i in range(old_div(size, 2)):
                    base = list_base + 2 * i
                    x, y = cond_swap(load_secret_mem(base),
                                     load_secret_mem(base + 1))
                    store_in_mem(x, base)
                    store_in_mem(y, base + 1)
            chunks[size] = FunctionTape(swap_list, 'sort-%d-%03x' %
                                        (size, random.randrange(256**3)))
        return chunks[size](base)

    def run_round(size):
        # minimize number of chunk sizes
        n_chunks = int(math.ceil(1.0 * size / max_chunk_size))
        lower_size = old_div(size, n_chunks / 2 * 2)
        n_lower_size = n_chunks - old_div((size - n_chunks * lower_size), 2)
        # print len(to_swap) == lower_size * n_lower_size + \
        #     (lower_size + 2) * (n_chunks - n_lower_size), \
        #     len(to_swap), n_chunks, lower_size, n_lower_size
        base = 0
        round_threads = []
        for i in range(n_lower_size):
            round_threads.append(run_chunk(lower_size, tmp_base + base))
            base += lower_size
        for i in range(n_chunks - n_lower_size):
            round_threads.append(run_chunk(lower_size + 2, tmp_base + base))
            base += lower_size + 2
        run_threads_in_rounds(round_threads)

    l = 1
    while l < n:
        l *= 2
        k = 1
        while k < l:
            k *= 2
            def load_and_store(x, y):
                if to_tmp:
                    store_in_mem(load_secret_mem(x), y)
                else:
                    store_in_mem(load_secret_mem(y), x)
            def outer(i):
                def inner(j):
                    base = j
                    step = old_div(l, k)
                    if k == 2:
                        tmp_addr = regint.load_mem(tmp_i)
                        load_and_store(base, tmp_addr)
                        load_and_store(base + step, tmp_addr + 1)
                        store_in_mem(tmp_addr + 2, tmp_i)
                    else:
                        def inner2(m):
                            tmp_addr = regint.load_mem(tmp_i)
                            load_and_store(m, tmp_addr)
                            store_in_mem(tmp_addr + 1, tmp_i)
                        range_loop(inner2, base + step, base + (k - 1) * step, step)
                range_loop(inner, a_base + i * l, a_base + i * l + old_div(l, k))
            instructions.program.curr_tape.merge_opens = False
            to_tmp = True
            store_in_mem(tmp_base, tmp_i)
            range_loop(outer, old_div(n, l))
            if k == 2:
                run_round(n)
            else:
                run_round(old_div(n, k * (k - 2)))
            instructions.program.curr_tape.merge_opens = False
            to_tmp = False
            store_in_mem(tmp_base, tmp_i)
            range_loop(outer, old_div(n, l))

    if isinstance(a, list):
        instructions.program.restart_main_thread()
        for i in range(n):
            a[i] = load_secret_mem(a_base + i)
        instructions.program.free(a_base, 's')
    instructions.program.free(tmp_base, 's')
    instructions.program.free(tmp_i, 'r')


def loopy_odd_even_merge_sort(a, sorted_length=1, n_parallel=32):
    l = sorted_length
    while l < len(a):
        l *= 2
        k = 1
        while k < l:
            k *= 2
            n_outer = old_div(len(a), l)
            n_inner = old_div(l, k)
            n_innermost = 1 if k == 2 else old_div(k, 2) - 1
            @for_range_parallel(old_div(n_parallel, n_innermost / n_inner), n_outer)
            def loop(i):
                @for_range_parallel(old_div(n_parallel, n_innermost), n_inner)
                def inner(j):
                    base = i*l + j
                    step = old_div(l,k)
                    if k == 2:
                        a[base], a[base+step] = cond_swap(a[base], a[base+step])
                    else:
                        @for_range_parallel(n_parallel, n_innermost)
                        def f(i):
                            m1 = step + i * 2 * step
                            m2 = m1 + base
                            a[m2], a[m2+step] = cond_swap(a[m2], a[m2+step])

def mergesort(A):
    B = Array(len(A), sint)

    def merge(i_left, i_right, i_end):
        i0 = MemValue(i_left)
        i1 = MemValue(i_right)
        @for_range(i_left, i_end)
        def loop(j):
            if_then(and_(lambda: i0 < i_right,
                         or_(lambda: i1 >= i_end,
                             lambda: regint(reveal(A[i0] <= A[i1])))))
            B[j] = A[i0]
            i0.iadd(1)
            else_then()
            B[j] = A[i1]
            i1.iadd(1)
            end_if()

    width = MemValue(1)
    @do_while
    def width_loop():
        @for_range(0, len(A), 2 * width)
        def merge_loop(i):
            merge(i, i + width, i + 2 * width)
        A.assign(B)
        width.imul(2)
        return width < len(A)

def range_loop(loop_body, start, stop=None, step=None):
    if stop is None:
        stop = start
        start = 0
    if step is None:
        step = 1
    def loop_fn(i):
        loop_body(i)
        return i + step
    if isinstance(step, int):
        if step > 0:
            condition = lambda x: x < stop
        elif step < 0:
            condition = lambda x: x > stop
        else:
            raise CompilerError('step must not be zero')
    else:
        b = step > 0
        condition = lambda x: b * (x < stop) + (1 - b) * (x > stop)
    while_loop(loop_fn, condition, start)
    if isinstance(start, int) and isinstance(stop, int) \
            and isinstance(step, int):
        # known loop count
        if condition(start):
            get_tape().req_node.children[-1].aggregator = \
                lambda x: (old_div((stop - start), step)) * x[0]

def for_range(start, stop=None, step=None):
    def decorator(loop_body):
        range_loop(loop_body, start, stop, step)
        return loop_body
    return decorator

def for_range_parallel(n_parallel, n_loops):
    return map_reduce_single(n_parallel, n_loops, \
                                 lambda *x: [], lambda *x: [])

def map_reduce_single(n_parallel, n_loops, initializer, reducer, mem_state=None):
    if not isinstance(n_parallel, int):
        raise CompilerException('Number of parallel executions' \
                                    'must be constant')
    n_parallel = n_parallel or 1
    if mem_state is None:
        # default to list of MemValues to allow varying types
        mem_state = [MemValue(x) for x in initializer()]
        use_array = False
    else:
        # use Arrays for multithread version
        use_array = True
    def decorator(loop_body):
        if isinstance(n_loops, int):
            loop_rounds = old_div(n_loops, n_parallel) \
                if n_parallel < n_loops else 0
        else:
            loop_rounds = old_div(n_loops, n_parallel)
        def write_state_to_memory(r):
            if use_array:
                mem_state.assign(r)
            else:
                # cannot do mem_state = [...] due to scope issue
                for j,x in enumerate(r):
                    mem_state[j].write(x)
        # will be optimized out if n_loops <= n_parallel
        @for_range(loop_rounds)
        def f(i):
            state = tuplify(initializer())
            for k in range(n_parallel):
                j = i * n_parallel + k
                state = reducer(tuplify(loop_body(j)), state)
            r = reducer(mem_state, state)
            write_state_to_memory(r)
        if isinstance(n_loops, int):
            state = mem_state
            for j in range(loop_rounds * n_parallel, n_loops):
                state = reducer(tuplify(loop_body(j)), state)
        else:
            @for_range(loop_rounds * n_parallel, n_loops)
            def f(j):
                r = reducer(tuplify(loop_body(j)), mem_state)
                write_state_to_memory(r)
            state = mem_state
        for i,x in enumerate(state):
            if use_array:
                mem_state[i] = x
            else:
                mem_state[i].write(x)
        def returner():
            return untuplify(tuple(state))
        return returner
    return decorator

def for_range_multithread(n_threads, n_parallel, n_loops, thread_mem_req={}):
    return map_reduce(n_threads, n_parallel, n_loops, \
                          lambda *x: [], lambda *x: [], thread_mem_req)

def map_reduce(n_threads, n_parallel, n_loops, initializer, reducer, \
                   thread_mem_req={}):
    n_threads = n_threads or 1
    if n_threads == 1 or n_loops == 1:
        dec = map_reduce_single(n_parallel, n_loops, initializer, reducer)
        if thread_mem_req:
            thread_mem = Array(thread_mem_req[regint], regint)
            return lambda loop_body: dec(lambda i: loop_body(i, thread_mem))
        else:
            return dec
    def decorator(loop_body):
        thread_rounds = old_div(n_loops, n_threads)
        remainder = n_loops % n_threads
        for t in thread_mem_req:
            if t != regint:
                raise CompilerError('Not implemented for other than regint')
        args = Matrix(n_threads, 2 + thread_mem_req.get(regint, 0), 'r')
        state = tuple(initializer())
        def f(inc):
            if thread_mem_req:
                thread_mem = Array(thread_mem_req[regint], regint, \
                                       args[get_arg()].address + 2)
            mem_state = Array(len(state), type(state[0]) \
                                  if state else cint, args[get_arg()][1])
            base = args[get_arg()][0]
            @map_reduce_single(n_parallel, thread_rounds + inc, \
                                   initializer, reducer, mem_state)
            def f(i):
                if thread_mem_req:
                    return loop_body(base + i, thread_mem)
                else:
                    return loop_body(base + i)
        prog = get_program()
        threads = []
        if thread_rounds:
            tape = prog.new_tape(f, (0,), 'multithread')
            for i in range(n_threads - remainder):
                mem_state = make_array(initializer())
                args[remainder + i][0] = i * thread_rounds
                if len(mem_state):
                    args[remainder + i][1] = mem_state.address
                threads.append(prog.run_tape(tape, remainder + i))
        if remainder:
            tape1 = prog.new_tape(f, (1,), 'multithread1')
            for i in range(remainder):
                mem_state = make_array(initializer())
                args[i][0] = (n_threads - remainder + i) * thread_rounds + i
                if len(mem_state):
                    args[i][1] = mem_state.address
                threads.append(prog.run_tape(tape1, i))
        for thread in threads:
            prog.join_tape(thread)
        if state:
            if thread_rounds:
                for i in range(n_threads - remainder):
                    state = reducer(Array(len(state), type(state[0]), \
                                              args[remainder + i][1]), state)
            if remainder:
                for i in range(remainder):
                    state = reducer(Array(len(state), type(state[0]).reg_type, \
                                              args[i][1]), state)
        def returner():
            return untuplify(state)
        return returner
    return decorator

def map_sum(n_threads, n_parallel, n_loops, n_items, value_types):
    value_types = tuplify(value_types)
    if len(value_types) == 1:
        value_types *= n_items
    elif len(value_types) != n_items:
        raise CompilerError('Incorrect number of value_types.')
    initializer = lambda: [t(0) for t in value_types]
    def summer(x,y):
        return tuple(a + b for a,b in zip(x,y))
    return map_reduce(n_threads, n_parallel, n_loops, initializer, summer)

def foreach_enumerate(a):
    for x in a:
        get_program().public_input(' '.join(str(y) for y in tuplify(x)))
    def decorator(loop_body):
        @for_range(len(a))
        def f(i):
            loop_body(i, *(public_input() for j in range(len(tuplify(a[0])))))
        return f
    return decorator

def while_loop(loop_body, condition, arg):
    if not callable(condition):
        raise CompilerError('Condition must be callable')
    # store arg in stack
    pre_condition = condition(arg)
    if not isinstance(pre_condition, (bool,int)) or pre_condition:
        pushint(arg if isinstance(arg,regint) else regint(arg))
        def loop_fn():
            result = loop_body(regint.pop())
            pushint(result)
            return condition(result)
        if_statement(pre_condition, lambda: do_while(loop_fn))
        regint.pop()

def while_do(condition, *args):
    def decorator(loop_body):
        while_loop(loop_body, condition, *args)
        return loop_body
    return decorator

def do_loop(condition, loop_fn):
    # store initial condition to stack
    pushint(condition if isinstance(condition,regint) else regint(condition))
    def wrapped_loop():
        # save condition to stack
        new_cond = regint.pop()
        # run the loop
        condition = loop_fn(new_cond)
        pushint(condition)
        return condition
    do_while(wrapped_loop)
    regint.pop()

def do_while(loop_fn):
    scope = instructions.program.curr_block
    parent_node = get_tape().req_node
    # possibly unknown loop count
    get_tape().open_scope(lambda x: x[0].set_all(float('Inf')), \
                              name='begin-loop')
    loop_block = instructions.program.curr_block
    condition = loop_fn()
    if callable(condition):
        condition = condition()
    branch = instructions.jmpnz(regint.conv(condition), 0, add_to_prog=False)
    instructions.program.curr_block.set_exit(branch, loop_block)
    get_tape().close_scope(scope, parent_node, 'end-loop')
    return loop_fn

def if_then(condition):
    class State(object): pass
    state = State()
    if callable(condition):
        condition = condition()
    state.condition = regint.conv(condition)
    state.start_block = instructions.program.curr_block
    state.req_child = get_tape().open_scope(lambda x: x[0].max(x[1]), \
                                                   name='if-block')
    state.has_else = False
    instructions.program.curr_tape.if_states.append(state)

def else_then():
    try:
        state = instructions.program.curr_tape.if_states[-1]
    except IndexError:
        raise CompilerError('No open if block')
    if state.has_else:
        raise CompilerError('else block already defined')
    # run the else block
    state.if_exit_block = instructions.program.curr_block
    state.req_child.add_node(get_tape(), 'else-block')
    instructions.program.curr_tape.start_new_basicblock(state.start_block, \
                                                            name='else-block')
    state.else_block = instructions.program.curr_block
    state.has_else = True

def end_if():
    try:
        state = instructions.program.curr_tape.if_states.pop()
    except IndexError:
        raise CompilerError('No open if/else block')
    branch = instructions.jmpeqz(regint.conv(state.condition), 0, \
                                     add_to_prog=False)
    # start next block
    get_tape().close_scope(state.start_block, state.req_child.parent, 'end-if')
    if state.has_else:
        # jump to else block if condition == 0
        state.start_block.set_exit(branch, state.else_block)
        # set if block to skip else
        jump = instructions.jmp(0, add_to_prog=False)
        state.if_exit_block.set_exit(jump, instructions.program.curr_block)
    else:
        # set start block's conditional jump to next block
        state.start_block.set_exit(branch, instructions.program.curr_block)
        # nothing to compute without else
        state.req_child.aggregator = lambda x: x[0]

def if_statement(condition, if_fn, else_fn=None):
    if condition is True or condition is False:
        # condition known at compile time
        if condition:
            if_fn()
        elif else_fn is not None:
            else_fn()
    else:
        state = if_then(condition)
        if_fn()
        if else_fn is not None:
            else_then()
            else_fn()
        end_if()

def if_(condition):
    def decorator(body):
        if_then(condition)
        body()
        end_if()
    return decorator

def if_e(condition):
    def decorator(body):
        if_then(condition)
        body()
    return decorator

def else_(body):
    else_then()
    body()
    end_if()

def and_(*terms):
    # not thread-safe
    p_res = instructions.program.malloc(1, 'r')
    for term in terms:
        if_then(term())
    store_in_mem(1, p_res)
    for term in terms:
        else_then()
        store_in_mem(0, p_res)
        end_if()
    def load_result():
        res = regint.load_mem(p_res)
        instructions.program.free(p_res, 'r')
        return res
    return load_result

def or_(*terms):
    # not thread-safe
    p_res = instructions.program.malloc(1, 'r')
    res = regint()
    for term in terms:
        if_then(term())
        store_in_mem(1, p_res)
        else_then()
    store_in_mem(0, p_res)
    for term in terms:
        end_if()
    def load_result():
        res = regint.load_mem(p_res)
        instructions.program.free(p_res, 'r')
        return res
    return load_result

def not_(term):
    return lambda: 1 - term()

def start_timer(timer_id=0):
    get_tape().start_new_basicblock(name='pre-start-timer')
    start_clock(timer_id)
    get_tape().start_new_basicblock(name='post-start-timer')

def stop_timer(timer_id=0):
    get_tape().start_new_basicblock(name='pre-stop-timer')
    stop_clock(timer_id)
    get_tape().start_new_basicblock(name='post-stop-timer')

def test(value, lower=None, upper=None, prec=None):
    lineno = inspect.currentframe().f_back.f_lineno
    if isinstance(value, _mem):
        value = value.read()
        reg_type = value.reg_type
    if isinstance(value, sfloat):
        lineno *= 1000
        store_in_mem(reveal(value.v), lineno + 1000)
        store_in_mem(reveal(value.p), lineno + 1250)
        store_in_mem(reveal(value.z), lineno + 1500)
        store_in_mem(reveal(value.s), lineno + 1750)
        #store_in_mem(reveal(value.err), lineno + 2000)
        reg_type = 'c'
    elif isinstance(value, sfix):
        lineno *= 1000
        store_in_mem(reveal(value.v), lineno + 1000)
        reg_type = 'c'
    elif isinstance(value, cfix):
        lineno *= 1000
        store_in_mem(value.v, lineno + 1000)
        reg_type = 'c'
    # tuple construction for mult2sint
    # msw and lsw could be replaced by val1 and val2
    # names where chosen to improve readibility
    elif type(value) is tuple:
        msw = reveal(value[0])
        lsw = reveal(value[1])
        store_in_mem(msw, lineno + 1000)
        store_in_mem(lsw, lineno + 2000)
        reg_type = 'c'
    else:
        if not value.is_clear:
            value = reveal(value)
        if value.size > 1:
            lineno *= 1000
        store_in_mem(value, lineno + 1000)
        reg_type = 'c'
    print("Test at", lineno)
    if lineno + 2000 > get_program().allocated_mem[reg_type]:
        get_program().allocated_mem[reg_type] = 2 * (lineno + 1000)

def test1(value, lower=None, upper=None):
    if isinstance(value, sint) or value.reg_type == 's':
        value = reveal(value)
    lineno = inspect.currentframe().f_back.f_lineno
    stmc(value, lineno + 1000)
    print("Test at", lineno)

def test_mem(value, address, lower=None, upper=None):
    lineno = inspect.currentframe().f_back.f_lineno
    print("Test at", lineno)

def no_result_testing():
    pass

# Fixed point ops

from math import ceil, log
from floatingpoint import PreOR, TruncPr, two_power

def approximate_reciprocal(divisor, k, f, theta):
    """
        returns aproximation of 1/divisor
        where type(divisor) = cint
    """
    def twos_complement(x):
        bits = x.bit_decompose(k)[::-1]
        bit_array = Array(k, cint)
        bit_array.assign(bits)

        twos_result = MemValue(cint(0))
        @for_range(k)
        def block(i):
            val = twos_result.read()
            val <<= 1
            val += 1 - bit_array[i]
            twos_result.write(val)

        return twos_result.read() + 1

    bit_array = Array(k, cint)
    bits = divisor.bit_decompose(k)[::-1]
    bit_array.assign(bits)

    cnt_leading_zeros = MemValue(regint(0))

    flag = MemValue(regint(0))
    cnt_leading_zeros = MemValue(regint(0))
    normalized_divisor = MemValue(divisor)

    @for_range(k)
    def block(i):
        flag.write(flag.read() | bit_array[i] == 1)
        @if_(flag.read() == 0)
        def block():
            cnt_leading_zeros.write(cnt_leading_zeros.read() + 1)
            normalized_divisor.write(normalized_divisor << 1)

    q = MemValue(two_power(k))
    e = MemValue(twos_complement(normalized_divisor.read()))

    @for_range(theta)
    def block(i):
        qread = q.read()
        eread = e.read()
        qread += (qread * eread) >> k
        eread = (eread * eread) >> k

        q.write(qread)
        e.write(eread)

    res = q >> (2*k - 2*f - cnt_leading_zeros)

    return res


def cint_cint_division(a, b, k, f):
    """
        Goldschmidt method implemented with
        SE aproximation:
        http://stackoverflow.com/questions/2661541/picking-good-first-estimates-for-goldschmidt-division
    """
    # theta can be replaced with something smaller
    # for safety we assume that is the same theta from previous GS method

    theta = int(ceil(old_div(log(k/3.5), log(2))))
    two = cint(2) * two_power(f)

    sign_b = cint(1) - 2 * cint(b < 0)
    sign_a = cint(1) - 2 * cint(a < 0)
    absolute_b = b * sign_b
    absolute_a = a * sign_a
    w0 = approximate_reciprocal(absolute_b, k, f, theta)

    A = Array(theta, cint)
    B = Array(theta, cint)
    W = Array(theta, cint)

    A[0] = absolute_a
    B[0] = absolute_b
    W[0] = w0
    @for_range(1, theta)
    def block(i):
        A[i] = (A[i - 1] * W[i - 1]) >> f
        B[i] = (B[i - 1] * W[i - 1]) >> f
        W[i] = two - B[i]
    return (sign_a * sign_b) * A[theta - 1]

from Compiler.program import Program
def sint_cint_division(a, b, k, f, kappa):
    """
        type(a) = sint, type(b) = cint
    """
    theta = int(ceil(old_div(log(k/3.5), log(2))))
    two = cint(2) * two_power(f)
    sign_b = cint(1) - 2 * cint(b < 0)
    sign_a = sint(1) - 2 * sint(a < 0)
    absolute_b = b * sign_b
    absolute_a = a * sign_a
    w0 = approximate_reciprocal(absolute_b, k, f, theta)

    A = Array(theta, sint)
    B = Array(theta, cint)
    W = Array(theta, cint)

    A[0] = absolute_a
    B[0] = absolute_b
    W[0] = w0

    @for_range(1, theta)
    def block(i):
        A[i] = TruncPr(A[i - 1] * W[i - 1], 2*k, f, kappa)
        temp = (B[i - 1] * W[i - 1]) >> f
        # no reading and writing to the same variable in a for loop.
        W[i] = two - temp
        B[i] = temp
    return (sign_a * sign_b) * A[theta - 1]

def FPDiv(a, b, k, f, kappa, simplex_flag=False):
    """
        Goldschmidt method as presented in Catrina10,
    """
    theta = int(ceil(old_div(log(k/3.5), log(2))))
    alpha = two_power(2*f)
    w = AppRcr(b, k, f, kappa, simplex_flag)
    x = alpha - b * w

    y = a * w
    y = TruncPr(y, 2*k, f, kappa)

    for i in range(theta):
        y = y * (alpha + x)
        x = x * x
        y = TruncPr(y, 2*k, 2*f, kappa)
        x = TruncPr(x, 2*k, 2*f, kappa)

    y = y * (alpha + x)
    y = TruncPr(y, 2*k, 2*f, kappa)
    return y
def AppRcr(b, k, f, kappa, simplex_flag=False):
    """
        Approximate reciprocal of [b]:
        Given [b], compute [1/b]
    """
    alpha = cint(int(2.9142 * 2**k))
    c, v = Norm(b, k, f, kappa, simplex_flag)
    #v should be 2**{k - m} where m is the length of the bitwise repr of [b]
    d = alpha - 2 * c
    w = d * v
    w = TruncPr(w, 2 * k, 2 * (k - f))
    # now w * 2 ^ {-f} should be an initial approximation of 1/b
    return w

def Norm(b, k, f, kappa, simplex_flag=False):
    """
        Computes secret integer values [c] and [v_prime] st.
        2^{k-1} <= c < 2^k and c = b*v_prime
    """
    # For simplex, we can get rid of computing abs(b)
    temp = None
    if simplex_flag == False:
        temp = sint(b < 0)
    elif simplex_flag == True:
        temp = cint(0)

    sign = 1 - 2 * temp # 1 - 2 * [b < 0]
    absolute_val = sign * b

    #next 2 lines actually compute the SufOR for little indian encoding
    bits = absolute_val.bit_decompose(k)[::-1]
    suffixes = PreOR(bits)[::-1]

    z = [0] * k
    for i in range(k - 1):
        z[i] = suffixes[i] - suffixes[i+1]
    z[k - 1] = suffixes[k-1]

    #doing complicated stuff to compute v = 2^{k-m}
    acc = cint(0)
    for i in range(k):
        acc += two_power(k-i-1) * z[i]

    part_reciprocal = absolute_val * acc
    signed_acc = sign * acc

    return part_reciprocal, signed_acc


def open_channel(channel=0):
   r"""Open channel and return any possible error message
   """
   res = regint()
   open_chan(res,channel)
   return res


##
#Conversion routine from int value a to cfloat representation.
# input is normalized using [ABZ13].
# Method mimics behaviour from secure implementation.
# @param a: int input to be normalized.
# @param gamma: input bitwise size
# @param l: float representation bitwise size
# @param kappa: security parameter
def int2FL_plain(a, gamma, l, kappa):
    lam = gamma - 1
    a_abs = 0
    v = cint(0)
    p = cint(0)
    s = cint(0)
    z = cint(0)

    # extracts the sign and calculates the abs
    s =  cint(a < 0)
    a_abs = a * (1 - 2 * s)

    # isolates most significative bit
    a_bits = a_abs.bit_decompose()
    b = 0
    b_c = 1
    blen = 0
    for a_i in range(len(a_bits) - 1, -1, -1):  # enumerate(a_bits):

        b = (a_bits[a_i]) * (b == 0) * (old_div((b_c), 2)) + b
        blen = (a_bits[a_i]) * (blen == 0) * ((a_i + 1)) + blen
        b_c = b_c * 2

    # obtains p
    # blen= len(a_bits) - blen

    v = a_abs * b  # (2 ** (b))#scale a
    p = - (lam - blen)  # (len(a_bits)-blen))

    # reduces v
    v_l = MemValue(v)
    z_l = MemValue(z)
    if_then(a_abs > 0)

    if (lam > l):
        v_l.write(old_div(v_l.read(), (2 ** (gamma - l - 1))))
    else:
        v_l.write(v_l.read() * (2 ** l - lam))

    else_then()
    z_l.write(cint(1))
    end_if()

    # corrects output
    # s is coming from the abs extraction
    v = cint(v_l.read())
    z = cint(z_l.read())
    p = cint((p + lam - l) * (1 - z))


    return v, p, z, s


