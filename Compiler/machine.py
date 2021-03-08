
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

from types import *
from oram import *
from path_oram import OptimalORAM
import oram

debug = False
do_tick = True

oram.n_threads = 1

arg1 = lambda in2,args: (args[1],)
no_arg = lambda in2,args: (0,)
deref = lambda in2,args: (in2,)

next = lambda PC,*args: (PC + 1,)

add = (arg1, lambda in1,in2,args: (in1 + in2, args[0], 1), next)
mul = (arg1, lambda in1,in2,args: (in1 * in2, args[0], 1), next)
lt = (arg1, lambda in1,in2,args: (in1 < in2, args[0], 1), next)
br = (no_arg, lambda in1,in2,args: (0, 0, 0), \
          lambda PC,in1,in2,args: if_else(in2, args[0], args[1]))
mov = (arg1, lambda in1,in2,args: (in1, args[0], 1), next)
load = (deref, lambda in1,in2,args: (in1, args[0], 1), next)
store = (arg1, lambda in1,in2,args: (in1, in2, 1), next)
store_const = (no_arg, lambda in1,in2,args: (args[1], args[0], 1), next)
store_const_ind = (no_arg, lambda in1,in2,args: (args[1], in2, 1), next)
add_const = (no_arg, lambda in1,in2,args: (args[1] + in2, args[0], 1), next)
mul_const = (no_arg, lambda in1,in2,args: (args[1] * in2, args[0], 1), next)
lt_const = (no_arg, lambda in1,in2,args: (in2 < args[1], args[0], 1), next)
gt_const = (no_arg, lambda in1,in2,args: (in2 > args[1], args[0], 1), next)
eq = (arg1, lambda in1,in2,args: (in1 == in2, args[0], 1), next)
eq_const = (no_arg, lambda in1,in2,args: (in2 == args[1], args[0], 1), next)
le = (arg1, lambda in1,in2,args: (in1 <= in2, args[0], 1), next)
le_const = (no_arg, lambda in1,in2,args: (in2 <= args[1], args[0], 1), next)
ge_const = (no_arg, lambda in1,in2,args: (in2 >= args[1], args[0], 1), next)
ne = (arg1, lambda in1,in2,args: (in1 != in2, args[0], 1), next)
ne_const = (no_arg, lambda in1,in2,args: (in2 != args[1], args[0], 1), next)
jmp = (no_arg, lambda in1,in2,args: (0, 0, 0), \
           lambda PC,in1,in2,args: args[0])
jmp_ind = (no_arg, lambda in1,in2,args: (0, 0, 0), \
               lambda PC,in1,in2,args: in2)
shr1 = (no_arg, lambda in1,in2,args: (in2 >> 1, args[0], 1), next)
shr_const = (no_arg, lambda in1,in2,args: (in2 >> args[1], args[0], 1), next)
shr = (arg1, lambda in1,in2,args: (in2 >> in1, args[0], 1), next)
shl = (arg1, lambda in1,in2,args: (in2 << in1, args[0], 1), next)
rsub_const = (no_arg, lambda in1,in2,args: (in2 - args[1], args[0], 1), next)
sub_const = (no_arg, lambda in1,in2,args: (args[1] - in2, args[0], 1), next)
sub = (arg1, lambda in1,in2,args: (in1 - in2, args[0], 1), next)
ult_pos_const = (no_arg, lambda in1,in2,args: ((in2 < args[1]) * (in2 >= 0), args[0], 1), next)
ule_pos_const = (no_arg, lambda in1,in2,args: ((in2 <= args[1]) * (in2 >= 0), args[0], 1), next)
and_ = (arg1, lambda in1,in2,args: (in1 & in2, args[0], 1), next)

def run_inst(inst_index, instructions, *args):
    return (sum(map(lambda x,y: x * x.hard_conv(y), inst_index, results)) \
                for results in zip(*(op(*args) for op in instructions)))

def run(code, data, operations, start=0, data_type=sint):
    preops, postops, jumps = zip(*operations)
    PC = MemValue(data_type(start))
    tick = MemValue(cint(0))
    @do_while
    def f():
        if do_tick:
            @if_(tick % 10 == 0)
            def f():
                tick.read().print_reg('tick')
                time()
            tick.iadd(1)
        args = list(code[PC])
        op = args.pop(0)
        op_index = op.bit_decompose(len(operations))
        in2 = data_type(data[args[2]])
        args = [data_type(arg) for arg in args]
        p_in1, = run_inst(op_index, preops, in2, args)
        in1 = data_type(data[p_in1])
        op_res, p_out, write = run_inst(op_index, postops, in1, in2, args)
        data.access(p_out, op_res, write)
        jump, = run_inst(op_index, jumps, PC, in1, in2, args)
        PC.write(data_type(jump))
        not_done = (sum(op_index)).reveal()
        if debug:
            print_ln('op: %s, args: %s, %s, %s', op.reveal(), \
                         *(x.reveal() for x in args))
            print_ln('p_in1: %s, in1: %s, in2: %s, op_res: %s', \
                         p_in1.reveal(), in1.reveal(), in2.reveal(), \
                         op_res.reveal())
            print_ln('write: %s, jump: %s, PC: %s, done: %s', \
                         write.reveal(), jump.reveal(), PC.reveal(), \
                         (1 - not_done))
        return regint(not_done)
    print_ln('Ticks: %s', tick)

def run_code_with_data(code, data, start=0, data_type=sint, oram_type=OptimalORAM):
    operations = []
    code_oram = oram_type(len(code) + 1, value_length=4, \
                                value_type=data_type.basic_type, \
                                init_rounds=0)
    code = list(list(inst) for inst in code)
    for i,inst in enumerate(code):
        try:
            op = operations.index(globals()[inst[0]])
        except ValueError:
            op = len(operations)
            if debug:
                print op, inst[0]
            operations.append(globals()[inst[0]])
        inst[0] = 1 << op
    code.append((0, 0, 0, 0))
    stop_timer(0)
    start_timer(1)
    @foreach_enumerate(code)
    def f(i, *inst):
        @if_(i % 1000 == 0)
        def f():
            print_ln('Loaded %s/%s instructions', i, len(code))
            time()
        code_oram[i] = [data_type(x) for x in inst]
    stop_timer(1)
    start_timer(0)
    run(code_oram, data, operations, start, data_type)

def run_code(code, data_length, start=0, n_global_vars=0, data_type=sint, \
             oram_type=OptimalORAM):
    data = oram_type(data_length, value_type=data_type.basic_type, \
                     init_rounds=0)
    stop_timer(0)
    start_timer(2)
    @for_range(n_global_vars)
    def f(i):
        data[i] = 32 + i % 96
    data[data_length-1] = 0
    stop_timer(2)
    start_timer(0)
    run_code_with_data(code, data, start, data_type)
    return data

def test_straight_machine():
    code = OptimalORAM(10, value_length=4)
    data = OptimalORAM(10)
    instructions = [add, mul]
    data[1] = 3
    data[2] = 5
    data[3] = 7
    code[0] = (2, 0, 1, 2)
    code[1] = (1, 0, 3, 0)
    code[2] = (0, 0, 0, 0)
    run(code, data, instructions)
    return data

def test_loop_machine():
    code = OptimalORAM(10, value_length=4)
    data = OptimalORAM(10)
    instructions = [add, lt, br]
    data[0] = 0
    data[1] = 1
    data[2] = 7
    code[0] = (1, 0, 0, 1)
    code[1] = (2, 3, 0, 2)
    code[2] = (4, 0, 3, 3)
    code[3] = (0, 0, 0, 0)
    run(code, data, instructions)
    return data

def test_mov():
    code = OptimalORAM(10, value_length=4)
    data = OptimalORAM(10)
    instructions = [mov]
    data[0] = 0
    data[1] = 1
    code[0] = (1, 0, 1, 0)
    code[1] = (0, 0, 0, 0)
    run(code, data, instructions)
    return data

def test_indirect():
    code = OptimalORAM(10, value_length=4)
    data = OptimalORAM(10)
    instructions = [load, store]
    data[0] = 0
    data[1] = 2
    data[2] = 1
    data[4] = 0
    code[0] = (1, 3, 0, 1)
    code[1] = (2, 0, 3, 4)
    code[2] = (0, 0, 0, 0)
    run(code, data, instructions)
    return data
