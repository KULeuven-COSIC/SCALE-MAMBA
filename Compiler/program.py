
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

# (C) 2017 University of Bristol. See License.txt

from Compiler.config import *
from Compiler.exceptions import *
from Compiler.instructions_base import RegType
import Compiler.instructions
import Compiler.instructions_base
import compilerLib
import allocator as al
import random
import time
import sys, os, errno
import inspect
from collections import defaultdict
import itertools
import math


data_types = dict(
    triple = 0,
    square = 1,
    bit = 2,
    inverse = 3,
    bittriple = 4,
    dabit = 5,
)

field_types = dict(
    modp = 0
)


class Program(object):
    """ A program consists of a list of tapes and a scheduled order
    of execution for these tapes.
    
    These are created by executing a file containing appropriate instructions
    and threads. """
    def __init__(self, args, options, param=-1):
        self.options = options
        self.args = args
        self.init_names(args)
        self.param = param
        sshare = open('Data/SharingData.txt')
        self.P = int(sshare.readline())
        print 'p = %d' % self.P
        prime_size = int(math.ceil(self.P.bit_length()/8))*8
        self.bit_length=64
        self.security = 40
        if prime_size<48:
           self.bit_length=24
           self.security=6
        elif prime_size<85:
           self.bit_length=32
           self.security=30
        print 'Prime size:', prime_size
        print 'Default bit length:', self.bit_length
        print 'Default statistical security parameter:', self.security
        self.fdflag = options.fdflag
        print 'Under Over Flow flag: ', self.fdflag
        self.schedule = [('start', [])]
        self.main_ctr = 0
        self.tapes = []
        self._curr_tape = None
        self.FIRST_PASS = False
        self.DEBUG = False
        self.main_thread_running = False
        self.allocated_mem = RegType.create_dict(lambda: USER_MEM)
        self.free_mem_blocks = defaultdict(set)
        self.allocated_mem_blocks = {}
        self.req_num = None
        self.tape_stack = []
        self.n_threads = 1
        self.free_threads = set()
        self.public_input_file = open(self.programs_dir + '/%s' % self.name, 'w')
        self.types = {}
        Program.prog = self
        
        self.reset_values()

    def get_args(self):
        return self.args

    def max_par_tapes(self):
        """ Upper bound on number of tapes that will be run in parallel.
        (Excludes empty tapes) """
        if self.n_threads > 1:
            if len(self.schedule) > 1:
                raise CompilerError('Static and dynamic parallelism not compatible')
            return self.n_threads
        res = 1
        running = defaultdict(lambda: 0)
        for action,tapes in self.schedule:
            tapes = [t[0] for t in tapes if not t[0].is_empty()]
            if action == 'start':
                for tape in tapes:
                    running[tape] += 1
            elif action == 'stop':
                for tape in tapes:
                    running[tape] -= 1
            else:
                raise CompilerError('Invalid schedule action')
            res = max(res, sum(running.itervalues()))
        return res
    
    def init_names(self, args):
        self.programs_dir=args[0];
        print 'Compiling program in', self.programs_dir
        if  self.programs_dir.endswith('/'):
           self.programs_dir = self.programs_dir[:-1]
        progname = self.programs_dir.split('/')[-1]
        
        if progname.endswith('.mpc'):
            progname = progname[:-4]
        
        self.infile = self.programs_dir + '/' + progname + '.mpc'
        print progname
        """
        self.name is input file name (minus extension) + any optional arguments.
        Used to generate output filenames
        """
        self.name = progname
        if len(args) > 1:
            self.name += '-' + '-'.join(args[1:])

    def new_tape(self, function, args=[], name=None):
        if name is None:
            name = function.__name__
        name = "%s-%s" % (self.name, name)
        # make sure there is a current tape
        self.curr_tape
        tape_index = len(self.tapes)
        name += "-%d" % tape_index
        self.tape_stack.append(self.curr_tape)
        self.curr_tape = Tape(name, self)
        self.curr_tape.prevent_direct_memory_write = True
        self.tapes.append(self.curr_tape)
        function(*args)
        self.finalize_tape(self.curr_tape)
        if self.tape_stack:
            self.curr_tape = self.tape_stack.pop()
        return tape_index

    def run_tape(self, tape_index, arg):
        if self.curr_tape is not self.tapes[0]:
            raise CompilerError('Compiler does not support ' \
                                    'recursive spawning of threads')
        if self.free_threads:
            thread_number = self.free_threads.pop()
        else:
            thread_number = self.n_threads
            self.n_threads += 1
        self.curr_tape.start_new_basicblock(name='pre-run_tape')
        Compiler.instructions.run_tape(thread_number, arg, tape_index, 0)
        self.curr_tape.start_new_basicblock(name='post-run_tape')
        self.curr_tape.req_node.children.append(self.tapes[tape_index].req_tree)
        return thread_number

    def join_tape(self, thread_number):
        self.curr_tape.start_new_basicblock(name='pre-join_tape')
        Compiler.instructions.join_tape(thread_number)
        self.curr_tape.start_new_basicblock(name='post-join_tape')
        self.free_threads.add(thread_number)

    def start_thread(self, thread, arg):
        if self.main_thread_running:
            # wait for main thread to finish
            self.schedule_wait(self.curr_tape)
            self.main_thread_running = False
        
        # compile thread if not been used already
        if thread.tape not in self.tapes:
            self.curr_tape = thread.tape
            self.tapes.append(thread.tape)
            thread.target(*thread.args)
        
        # add thread to schedule
        self.schedule_start(thread.tape, arg)
        self.curr_tape = None
    
    def stop_thread(self, thread):
        tape = thread.tape
        self.schedule_wait(tape)

    def update_req(self, tape):
        if self.req_num is None:
            self.req_num = tape.req_num
        else:
            self.req_num += tape.req_num
    
    def read_memory(self, filename):
        """ Read the clear and shared memory from a file """
        f = open(filename)
        n = int(f.next())
        self.mem_c = [0]*n
        self.mem_s = [0]*n
        mem = self.mem_c
        done_c = False
        for line in f:
            line = line.split(' ')
            a = int(line[0])
            b = int(line[1])
            if a != -1:
                mem[a] = b
            elif done_c:
                break
            else:
                mem = self.mem_s
                done_c = True
    
    def get_memory(self, mem_type, i):
        if mem_type == 'c':
            return self.mem_c[i]
        elif mem_type == 's':
            return self.mem_s[i]
        raise CompilerError('Invalid memory type')
    
    def reset_values(self):
        """ Reset register and memory values. """
        for tape in self.tapes:
            tape.reset_registers()
        self.mem_c = range(USER_MEM + TMP_MEM)
        self.mem_s = range(USER_MEM + TMP_MEM)
        random.seed(0)
    
    def write_bytes(self, outfile=None):
        """ Write all non-empty threads and schedule to files. """
        # runtime doesn't support 'new-style' parallelism yet
        old_style = True

        nonempty_tapes = [t for t in self.tapes]

        sch_filename = self.programs_dir + '/%s.sch' % self.name
        sch_file = open(sch_filename, 'w')
        print 'Writing to', sch_filename
        sch_file.write(str(self.max_par_tapes()) + '\n')
        sch_file.write(str(len(nonempty_tapes)) + '\n')
        sch_file.write(' '.join(tape.name for tape in nonempty_tapes) + '\n')
        
        # assign tapes indices (needed for scheduler)
        for i,tape in enumerate(nonempty_tapes):
            tape.index = i
        
        for sch in self.schedule:
            # schedule may still contain empty tapes: ignore these
            tapes = filter(lambda x: not x[0].is_empty(), sch[1])
            # no empty line
            if not tapes:
                continue
            line = ' '.join(str(t[0].index) +
                            (':' + str(t[1]) if t[1] is not None else '') for t in tapes)
            if old_style:
                if sch[0] == 'start':
                    sch_file.write('%d %s\n' % (len(tapes), line))
            else:
                sch_file.write('%s %d %s\n' % (tapes[0], len(tapes), line))
        
        sch_file.write('0\n')
        sch_file.write(' '.join(sys.argv) + '\n')
   
        if self.options.asmoutfile or self.options.produce_asm:
           return
        
        for tape in self.tapes:
            tape.write_bytes()
    
    def schedule_start(self, tape, arg=None):
        """ Schedule the start of a thread. """
        if self.schedule[-1][0] == 'start':
            self.schedule[-1][1].append((tape, arg))
        else:
            self.schedule.append(('start', [(tape, arg)]))
    
    def schedule_wait(self, tape):
        """ Schedule the end of a thread. """
        if self.schedule[-1][0] == 'stop':
            self.schedule[-1][1].append((tape, None))
        else:
            self.schedule.append(('stop', [(tape, None)]))
        self.finalize_tape(tape)
        self.update_req(tape)

    def finalize_tape(self, tape):
        if not tape.purged:
            tape.optimize(self.options)
            if self.options.asmoutfile:
                tape.write_str(self.options.asmoutfile + '-' + tape.name+'.asm')
            elif self.options.produce_asm:
                tape.write_str(self.args[0]+'/'+tape.name+'.asm')
            else:
                tape.write_bytes()
            tape.purge()
    
    
    def restart_main_thread(self):
        if self.main_thread_running:
            # wait for main thread to finish
            self.schedule_wait(self._curr_tape)
            self.main_thread_running = False
        name = '%s-%d' % (self.name, self.main_ctr)
        self._curr_tape = Tape(name, self)
        self.tapes.append(self._curr_tape)
        self.main_ctr += 1
        # add to schedule
        self.schedule_start(self._curr_tape)
        self.main_thread_running = True
    
    @property
    def curr_tape(self):
        """ The tape that is currently running."""
        if self._curr_tape is None:
            # Create a new main thread if necessary
            self.restart_main_thread()
        return self._curr_tape

    @curr_tape.setter
    def curr_tape(self, value):
        self._curr_tape = value

    @property
    def curr_block(self):
        """ The basic block that is currently being created. """
        return self.curr_tape.active_basicblock
    
    def malloc(self, size, mem_type, reg_type=None):
        """ Allocate memory from the top """
        if not isinstance(size, (int, long)):
            raise CompilerError('size must be known at compile time')
        if size == 0:
            return
        if isinstance(mem_type, type):
            self.types[mem_type.reg_type] = mem_type
            mem_type = mem_type.reg_type
        elif reg_type is not None:
            self.types[mem_type] = reg_type
        key = size, mem_type
        if self.free_mem_blocks[key]:
            addr = self.free_mem_blocks[key].pop()
        else:
            addr = self.allocated_mem[mem_type]
            self.allocated_mem[mem_type] += size
            if len(str(addr)) != len(str(addr + size)):
                print "Memory of type '%s' now of size %d" % (mem_type, addr + size)
        self.allocated_mem_blocks[addr,mem_type] = size
        return addr

    def free(self, addr, mem_type):
        """ Free memory """
        if self.curr_block.alloc_pool \
           is not self.curr_tape.basicblocks[0].alloc_pool:
            raise CompilerError('Cannot free memory within function block')
        size = self.allocated_mem_blocks.pop((addr,mem_type))
        self.free_mem_blocks[size,mem_type].add(addr)

    def public_input(self, x):
        self.public_input_file.write('%s\n' % str(x))

    def set_bit_length(self, bit_length):
        self.bit_length = bit_length
        print 'Changed bit length for comparisons etc. to', bit_length

    def set_security(self, security):
        self.security = security
        print 'Changed statistical security for comparison etc. to', security

class Tape:
    """ A tape contains a list of basic blocks, onto which instructions are added. """
    def __init__(self, name, program, param=-1):
        """ Set prime p and the initial instructions and registers. """
        self.program = program
        self.init_names(name)
        sshare = open('Data/SharingData.txt')
        self.P = int(sshare.readline())
        self.init_registers()
        self.req_tree = self.ReqNode(name)
        self.req_node = self.req_tree
        self.basicblocks = []
        self.purged = False
        self.active_basicblock = None
        self.start_new_basicblock()
        self._is_empty = False
        self.merge_opens = True
        self.if_states = []
        self.req_bit_length = defaultdict(lambda: 0)
        self.function_basicblocks = {}
        self.functions = []
        self.prevent_direct_memory_write = False

    class BasicBlock(object):
        def __init__(self, parent, name, scope, exit_condition=None):
            self.parent = parent
            self.P = parent.P
            self.instructions = []
            self.name = name
            self.index = len(parent.basicblocks)
            self.open_queue = []
            self.exit_condition = exit_condition
            self.exit_block = None
            self.previous_block = None
            self.scope = scope
            self.children = []
            if scope is not None:
                scope.children.append(self)
                self.alloc_pool = scope.alloc_pool
            else:
                self.alloc_pool = defaultdict(set)

        def new_reg(self, reg_type, size=None):
            return self.parent.new_reg(reg_type, size=size)

        def set_return(self, previous_block, sub_block):
            self.previous_block = previous_block
            self.sub_block = sub_block

        def set_exit(self, condition, exit_true=None):
            """ Sets the block which we start from next, depending on the condition.

            (Default is to go to next block in the list)
            """
            self.exit_condition = condition
            self.exit_block = exit_true
            for reg in condition.get_used():
                reg.can_eliminate = False
        
        def add_jump(self):
            """ Add the jump for this block's exit condition to list of
            instructions (must be done after merging) """
            self.instructions.append(self.exit_condition)
        
        def get_offset(self, next_block):
            return next_block.offset - (self.offset + len(self.instructions))
        
        def adjust_jump(self):
            """ Set the correct relative jump offset """
            offset = self.get_offset(self.exit_block)
            self.exit_condition.set_relative_jump(offset)
            #print 'Basic block %d jumps to %d (%d)' % (next_block_index, jump_index, offset)
        def __str__(self):
            return self.name

    def is_empty(self):
        """ Returns True if the list of basic blocks is empty.

        Note: False is returned even when tape only contains basic
        blocks with no instructions. However, these are removed when
        optimize is called. """
        if not self.purged:
            self._is_empty = (len(self.basicblocks) == 0)
        return self._is_empty

    def start_new_basicblock(self, scope=False, name=''):
        # use False because None means no scope
        if scope is False:
            scope = self.active_basicblock
        suffix = '%s-%d' % (name, len(self.basicblocks))
        sub = self.BasicBlock(self, self.name + '-' + suffix, scope)
        self.basicblocks.append(sub)
        self.active_basicblock = sub
        self.req_node.add_block(sub)
        print 'Compiling basic block', sub.name

    def init_registers(self):
        self.reset_registers()
        self.reg_counter = RegType.create_dict(lambda: 0)
   
    def init_names(self, name):
        # ignore path to file - source must be in Programs/Source
        name = name.split('/')[-1]
        if name.endswith('.asm'):
            self.name = name[:-4]
        else:
            self.name = name
        self.infile = self.program.programs_dir + '/' + self.name + '.asm'
        self.outfile = self.program.programs_dir + '/' + self.name + '.bc'

    def purge(self):
        self._is_empty = (len(self.basicblocks) == 0)
        del self.reg_values
        del self.basicblocks
        del self.active_basicblock
        self.purged = True

    def unpurged(function):
        def wrapper(self, *args, **kwargs):
            if self.purged:
                print '%s called on purged block %s, ignoring' % \
                    (function.__name__, self.name)
                return
            return function(self, *args, **kwargs)
        return wrapper

    @unpurged
    def optimize(self, options):
        if len(self.basicblocks) == 0:
            print 'Tape %s is empty' % self.name
            return

        if self.if_states:
            raise CompilerError('Unclosed if/else blocks')

        print 'Processing tape', self.name, 'with %d blocks' % len(self.basicblocks)

        for block in self.basicblocks:
            al.determine_scope(block, options)

        # merge open instructions
        # need to do this if there are several blocks
        if (options.merge_opens and self.merge_opens) or options.dead_code_elimination:
            for i,block in enumerate(self.basicblocks):
                if len(block.instructions) > 0:
                    print 'Processing basic block %s, %d/%d, %d instructions ' %(block.name, i, len(self.basicblocks), len(block.instructions))
                    sys.stdout.flush()
                # the next call is necessary for allocation later even without merging
                merger = al.Merger(block, options)
                if options.dead_code_elimination:
                    if len(block.instructions) > 10000:
                        print 'Eliminate dead code...'
                        sys.stdout.flush()
                    merger.eliminate_dead_code()
                if options.merge_opens and self.merge_opens:
                    if len(block.instructions) == 0:
                        block.used_from_scope = set()
                        block.defined_registers = set()
                        continue
                    if len(block.instructions) > 10000:
                        print 'Merging open instructions...'
                        sys.stdout.flush()
                    numrounds = merger.longest_paths_merge()
                    if numrounds > 0:
                        print 'Program requires %d rounds of communication' % numrounds
                        sys.stdout.flush()
                    numinv = sum(len(i.args) for i in block.instructions if isinstance(i, Compiler.instructions.startopen_class))
                    if numinv > 0:
                        print 'Program requires %d invocations' % numinv
                        sys.stdout.flush()
                if options.dead_code_elimination:
                    block.instructions = filter(lambda x: x is not None, block.instructions)
        if not (options.merge_opens and self.merge_opens):
            print 'Not merging open instructions in tape %s' % self.name
            sys.stdout.flush()

        # add jumps
        offset = 0
        for block in self.basicblocks:
            if block.exit_condition is not None:
                block.add_jump()
            block.offset = offset
            offset += len(block.instructions)
        for block in self.basicblocks:
            if block.exit_block is not None:
                block.adjust_jump()

        # now remove any empty blocks (must be done after setting jumps)
        self.basicblocks = filter(lambda x: len(x.instructions) != 0, self.basicblocks)

        # allocate registers
        reg_counts = self.count_regs()
        if not options.noreallocate:
            print 'Tape register usage:', reg_counts
            print 'modp: %d clear, %d secret' % (reg_counts[RegType.ClearModp], reg_counts[RegType.SecretModp])
            print 'Re-allocating...'
            sys.stdout.flush()
            allocator = al.StraightlineAllocator(REG_MAX)
            def alloc_loop(block):
                for reg in block.used_from_scope:
                    allocator.alloc_reg(reg, block.alloc_pool)
                for child in block.children:
                    if child.instructions:
                        alloc_loop(child)
            for i,block in enumerate(reversed(self.basicblocks)):
                if len(block.instructions) > 10000:
                    print 'Allocating %s, %d/%d' % \
                        (block.name, i, len(self.basicblocks))
                    sys.stdout.flush()
                if block.exit_condition is not None:
                    jump = block.exit_condition.get_relative_jump()
                    if jump != -1 and  \
                            isinstance(jump, (int,long)) and \
                            jump < 0 and \
                            block.exit_block.scope is not None:
                        alloc_loop(block.exit_block.scope)
                allocator.process(block.instructions, block.alloc_pool)

        # offline data requirements
        print 'Compile offline data requirements...'
        self.req_num = self.req_tree.aggregate()
        print 'Tape requires', self.req_num
        for req,num in self.req_num.items():
            if num == float('inf'):
                num = -1

        if not self.is_empty():
            # bit length requirement
            for x in ('p', '2'):
                if self.req_bit_length['p']:
                    self.basicblocks[-1].instructions.append(
                        Compiler.instructions.reqbl(self.req_bit_length['p'],
                                                    add_to_prog=False))
            print 'Tape requires prime bit length', self.req_bit_length['p']

    @unpurged
    def _get_instructions(self):
        return itertools.chain.\
            from_iterable(b.instructions for b in self.basicblocks)

    @unpurged
    def get_encoding(self):
        """ Get the encoding of the program, in human-readable format. """
        return [i.get_encoding() for i in self._get_instructions() if i is not None]
    
    @unpurged
    def get_bytes(self):
        """ Get the byte encoding of the program as an actual string of bytes. """
        return "".join(str(i.get_bytes()) for i in self._get_instructions() if i is not None)
    
    @unpurged
    def write_encoding(self, filename):
        """ Write the readable encoding to a file. """
        print 'Writing to', filename
        sys.stdout.flush()
        f = open(filename, 'w')
        for line in self.get_encoding():
            f.write(str(line) + '\n')
        f.close()
    
    @unpurged
    def write_str(self, filename):
        """ Write the sequence of instructions to a file. """
        print 'Writing to', filename
        sys.stdout.flush()
        f = open(filename, 'w')
        n = 0
        for block in self.basicblocks:
            if block.instructions:
                f.write('# %s\n' % block.name)
                for line in block.instructions:
                    f.write('%s # %d\n' % (line, n))
                    n += 1
        f.close()
    
    @unpurged
    def write_bytes(self, filename=None):
        """ Write the program's byte encoding to a file. """
        if filename is None:
            filename = self.outfile
        print 'Writing to', filename
        sys.stdout.flush()
        if not filename.endswith('.bc'):
            filename += '.bc'
        print 'Writing to', filename
        sys.stdout.flush()
        f = open(filename, 'w')
        f.write(self.get_bytes())
        f.close()
    
    def new_reg(self, reg_type, size=None):
        return self.Register(reg_type, self, size=size)
    
    def count_regs(self, reg_type=None):
        if reg_type is None:
            return self.reg_counter
        else:
            return self.reg_counter[reg_type]
    
    def reset_registers(self):
        """ Reset register values to zero. """
        self.reg_values = RegType.create_dict(lambda: [0] * INIT_REG_MAX)
    
    def get_value(self, reg_type, i):
        return self.reg_values[reg_type][i]
    
    def __str__(self):
        return self.name

    class ReqNum(defaultdict):
        def __init__(self, init={}):
            super(Tape.ReqNum, self).__init__(lambda: 0, init)
        def __add__(self, other):
            res = Tape.ReqNum()
            for i,count in self.items():
                res[i] += count            
            for i,count in other.items():
                res[i] += count
            return res
        def __mul__(self, other):
            res = Tape.ReqNum()
            for i in self:
                res[i] = other * self[i]
            return res
        __rmul__ = __mul__
        def set_all(self, value):
            res = Tape.ReqNum()
            for i in self:
                res[i] = value
            return res
        def max(self, other):
            res = Tape.ReqNum()
            for i in self:
                res[i] = max(self[i], other[i])
            for i in other:
                res[i] = max(self[i], other[i])
            return res
        def __str__(self):
            return ", ".join('%s inputs in %s from player %d' \
                                % (num, req[0], req[2]) \
                                if req[1] == 'input' \
                                else '%s %ss in %s' % (num, req[1], req[0]) \
                                for req,num in self.items())
        def __repr__(self):
            return repr(dict(self))

    class ReqNode(object):
        __slots__ = ['num', 'children', 'name', 'blocks']
        def __init__(self, name):
            self.children = []
            self.name = name
            self.blocks = []
        def aggregate(self, *args):
            self.num = Tape.ReqNum()
            for block in self.blocks:
                for inst in block.instructions:
                    inst.add_usage(self)
            res = reduce(lambda x,y: x + y.aggregate(self.name),
                         self.children, self.num)
            return res
        def increment(self, data_type, num=1):
            self.num[data_type] += num
        def add_block(self, block):
            self.blocks.append(block)

    class ReqChild(object):
        __slots__ = ['aggregator', 'nodes', 'parent']
        def __init__(self, aggregator, parent):
            self.aggregator = aggregator
            self.nodes = []
            self.parent = parent
        def aggregate(self, name):
            res = self.aggregator([node.aggregate() for node in self.nodes])
            return res
        def add_node(self, tape, name):
            new_node = Tape.ReqNode(name)
            self.nodes.append(new_node)
            tape.req_node = new_node

    def open_scope(self, aggregator, scope=False, name=''):
        child = self.ReqChild(aggregator, self.req_node)
        self.req_node.children.append(child)
        child.add_node(self, '%s-%d' % (name, len(self.basicblocks)))
        self.start_new_basicblock(name=name)
        return child

    def close_scope(self, outer_scope, parent_req_node, name):
        self.req_node = parent_req_node
        self.start_new_basicblock(outer_scope, name)

    def require_bit_length(self, bit_length, t='p'):
        if t == 'p':
            self.req_bit_length[t] = max(bit_length + 1, \
                                         self.req_bit_length[t])
            if self.program.param != -1 and bit_length >= self.program.param:
                raise CompilerError('Inadequate bit length %d for prime, ' \
                                    'program requires %d bits' % \
                                    (self.program.param, self.req_bit_length['p']))
        else:
            self.req_bit_length[t] = max(bit_length, self.req_bit_length)

    class Register(object):
        """
        Class for creating new registers. The register's index is automatically assigned
        based on the block's  reg_counter dictionary.
        
        The 'value' property is for emulation.
        """
        __slots__ = ["reg_type", "program", "i", "value", "_is_active", \
                         "size", "vector", "vectorbase", "caller", \
                         "can_eliminate"]

        def __init__(self, reg_type, program, value=None, size=None, i=None):
            """ Creates a new register.
                reg_type must be one of those defined in RegType. """
            self.reg_type = reg_type
            self.program = program
            if size is None:
                size = Compiler.instructions_base.get_global_vector_size()
            self.size = size
            if i:
                self.i = i
            else:
                self.i = program.reg_counter[reg_type]
                program.reg_counter[reg_type] += size
            self.vector = []
            self.vectorbase = self
            if value is not None:
                self.value = value
            self._is_active = False
            self.can_eliminate = True
            if Program.prog.DEBUG:
                self.caller = [frame[1:] for frame in inspect.stack()[1:]]
            else:
                self.caller = None
            if self.i % 1000000 == 0 and self.i > 0:
                print "Initialized %d registers at" % self.i, time.asctime()
                sys.stdout.flush()

        def set_size(self, size):
            if self.size == size:
                return
            elif self.size == 1 and self.vectorbase is self:
                if '%s%d' % (self.reg_type, self.i) in compilerLib.VARS:
                    # create vector register in assembly mode
                    self.size = size
                    self.vector = [self]
                    for i in range(1,size):
                        reg = compilerLib.VARS['%s%d' % (self.reg_type, self.i + i)]
                        reg.set_vectorbase(self)
                        self.vector.append(reg)
                else:
                    raise CompilerError('Cannot find %s in VARS' % str(self))
            else:
                raise CompilerError('Cannot reset size of vector register')

        def set_vectorbase(self, vectorbase):
            if self.vectorbase != self:
                raise CompilerError('Cannot assign one register' \
                                        'to several vectors')
            self.vectorbase = vectorbase

        def create_vector_elements(self):
            if self.vector:
                return
            elif self.size == 1:
                self.vector = [self]
                return
            self.vector = [self]
            for i in range(1,self.size):
                reg = Tape.Register(self.reg_type, self.program, size=1, i=self.i+i)
                reg.set_vectorbase(self)
                self.vector.append(reg)

        def __getitem__(self, index):
            if not self.vector:
                self.create_vector_elements()
            return self.vector[index]

        def __len__(self):
            return self.size
        
        def activate(self):
            """ Activating a register signals that it will at some point be used
            in the program.
            
            Inactive registers are reserved for temporaries for CISC instructions. """
            if not self._is_active:
                self._is_active = True
        
        @property
        def value(self):
            return self.program.reg_values[self.reg_type][self.i]
        
        @value.setter
        def value(self, val):
            while (len(self.program.reg_values[self.reg_type]) <= self.i):
                self.program.reg_values[self.reg_type] += [0] * INIT_REG_MAX
            self.program.reg_values[self.reg_type][self.i] = val
        
        @property
        def is_active(self):
            return self._is_active

        @property
        def is_clear(self):
            return self.reg_type == RegType.ClearModp or \
                self.reg_type == RegType.ClearInt

        def __str__(self):
            return self.reg_type + str(self.i)

        __repr__ = __str__
