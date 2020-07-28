#!/usr/bin/python3

# Author: Mark Will 
# Copyright (c) 2020 Acronis Asia R&D Pte Ltd 
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse
import logging
import re
import subprocess
from functools import reduce

# Handle arguments
parser = argparse.ArgumentParser()
parser.add_argument(
    "input_file",
    help = "File to be converted", 
    type = str
)
parser.add_argument(
    "output_file", 
    help = "Bristol output file", 
    type = str
)
parser.add_argument(
    "-t", 
    "--top_module", 
    help = "Top module of the design", 
    type = str
)
parser.add_argument(
    "-sy", 
    "--skip_yosys", 
    help = "Input file is now assumed to have been synthesised. This allows you to run Yosys manually", 
    action = "store_true"
)
parser.add_argument(
    "-a", 
    "--add_file", 
    help = "Add more files to the design", 
    action = 'append', 
    type = str
)
parser.add_argument(
    "-l", 
    "--lang", 
    help = """ 
        Input langauge. Note vhdl requires Yosys to be built with Verific support, 
        probably better to convert to Verilog for this converter
    """, 
    type = str, 
    choices = ["verilog", "vhdl"], 
    default = "verilog"
)
parser.add_argument(
    "-v", 
    "--verbose", 
    action = "store_true"
) 
args = parser.parse_args()

if args.verbose:
    logging.basicConfig(level=logging.INFO)
else:
    logging.basicConfig(level=logging.ERROR)

if args.top_module == None:
    tm = re.findall(r"([^/.]+)\.[^/]*$", args.input_file)
    if len(tm) != 1:
        tm = ['']
    args.top_module = tm[0]
    logging.info("Missing top module, assuming input filename: %s" % args.top_module)


if not args.skip_yosys:
    # Make sure yosys is installed and in the path
    try:
        with subprocess.Popen(["yosys", "-V"], stdout=subprocess.PIPE) as proc:
            proc.wait()
            version = re.findall(r"Yosys (\d+\.\d+)", str(proc.stdout.read()))
            if len(version) == 1: 
                logging.info("Using Yosys version %s" % version[0])
            else:
                raise Exception("Yosys version not found.")
    except Exception as e:
        logging.error(e)
        logging.error("Yosys not found. Please install it (http://www.clifford.at/yosys/download.html)")
        exit()
    # Process the design
    try:
        pipe_stdout = subprocess.PIPE if not args.verbose else None
        with subprocess.Popen(["yosys"], stdin = subprocess.PIPE, stdout = pipe_stdout) as proc:
            # Please feel free to change the commands or use -sy to manually run Yosys
            if args.lang == 'verilog':
                read_option = "-sv"
            else:
                read_option = "-vhdl"
            input_files = [args.input_file]
            input_files += args.add_file if args.add_file else []
            for f in input_files: 
                proc.stdin.write(("read %s %s\n" % (read_option, f)).encode())
            proc.stdin.write(("synth -top %s\n" % args.top_module).encode())
            # Needed to reduce the gates to ones supported by the Bristol format
            proc.stdin.write(b"proc\n")
            proc.stdin.write(b"opt\n")
            proc.stdin.write(b"fsm\n")
            proc.stdin.write(b"flatten\n")
            proc.stdin.write(b"memory\n")
            proc.stdin.write(b"opt\n")
            proc.stdin.write(b"techmap\n")
            proc.stdin.write(b"opt\n")
            proc.stdin.write(b"wreduce\n")
            # This can be slow
            #proc.stdin.write(b"freduce -vv\n")
            proc.stdin.write(b"clean\n")
            proc.stdin.write(b"abc -g AND\n")
            proc.stdin.write(b"opt_reduce -fine\n")
            for i in range(5):
                proc.stdin.write(b"abc -g XOR,AND\n")
                proc.stdin.write(b"opt -fine\n")
                proc.stdin.write(b"clean\n")
                proc.stdin.write(b"flatten\n")
            proc.stdin.write(b"torder\n")
            # Note that -nohex and -noattr are required
            proc.stdin.write(("write_verilog -nohex -noattr %s.yosys.v\n" % args.input_file).encode())
            out = proc.communicate(input=b"exit")
            if not args.verbose:
                pstdout = out[0].decode("utf-8")
                logging.info(pstdout)
                errors = re.findall(r"\nERROR: ([^\n]*)\n", pstdout)
                for error in errors:
                    logging.error(error)
                    if args.lang == 'vhdl' and 'Verific' in error:
                        logging.error("Note that Yosys needs to be built with Verific support. You could also try https://github.com/pliu6/vhd2vl.git to convert your VHDL code to Verilog.")
    except Exception as e:
        logging.error(e)
        exit()

# Convert yosys verilog output to Bristol format
# Note only one module should be here.
wires = []
inputs = []
outputs = []
assigns = []
logging.info("Loading design from Yosys")
filename = "%s.yosys.v" % args.input_file if not args.skip_yosys else args.input_file
with open(filename, 'r') as f:
    modules = 0
    while True:
        line = f.readline()
        if not line:
            break
        line = line.strip()
        if line.startswith('wire'):
            wires.append(line[5:-1])
        elif line.startswith('input'):
            inputs.append(line[6:-1])
        elif line.startswith('output'):
            outputs.append(line[7:-1])
        elif line.startswith('assign'):
            assigns.append(line[7:-1])
        elif line.startswith('module'):
            modules += 1
    assert modules == 1



num_input_wires = 0
num_output_wires = 0
wire_mapping = {}
input_wire = 0
input_lens = []
output_lens = []
prop_lens = {}

# Inputs are first wires
logging.info("Processing input wires")

def getIOProp(line):
    line = line.strip()
    arr = re.findall(r"\[([0-9]+)\:([0-9]+)\]", line)
    c = 1
    if arr:
        c += (reduce(lambda a,b: a + int(b), [0] + list(arr[0])))
    name = re.findall(r"[^ ]*$", line)[0]
    return (name, c)

for ip in inputs:
    name, l = getIOProp(ip)
    num_input_wires += l
    input_lens.append(str(l))
    prop_lens[name] = l
    if l == 1:
        wire_mapping[name] = input_wire
        input_wire += 1
    else:
        for i in range(l):
            wire_mapping["%s[%s]" % (name, i)] = input_wire
            input_wire += 1

# Fix wires to offset with input_wires
# Sort, in string mode should be fine
logging.info("Processing internal wires")
wires.sort()
num_wires = 2
for i in range(len(wires)):
    name, l = getIOProp(wires[i])
    prop_lens[name] = l
    if l == 1:
        wire_mapping[name] = num_wires + num_input_wires
        num_wires += 1
    else:
        for j in range(l):
            wire_mapping["%s[%s]" % (name, j)] = num_wires + num_input_wires
            num_wires += 1

# Outputs are last wires, note gnd and high require a wire
logging.info("Processing output wires")
output_wire = num_input_wires + num_wires
output_wire_names = []
for op in outputs:
    name, l = getIOProp(op)
    num_output_wires += l
    output_lens.append(str(l))
    prop_lens[name] = l
    if l == 1:
        wire_mapping[name] = output_wire
        output_wire_names.append(name)
        output_wire += 1
    else:
        for i in range(l):
            wire_mapping["%s[%s]" % (name, i)] = output_wire
            output_wire_names.append("%s[%s]" % (name, i))
            output_wire += 1

total_num_wires = sum([num_input_wires, num_output_wires, num_wires])

# Set gnd and high wire.
STR_GND_WIRE = "YOSYS_STR_GND_WIRE"
STR_HIGH_WIRE = "YOSYS_STR_HIGH_WIRE"
wire_mapping[STR_GND_WIRE] = num_input_wires
wire_mapping[STR_HIGH_WIRE] = num_input_wires + 1


# Process assignments
logging.info("Processing assigments")
class Instruction(object):
    def __init__(self, inputs, output, cmd):
        self.inputs = list(map(lambda a: a.strip(), inputs))
        # Only a single output is supported
        self.output = output.strip()
        self.cmd = cmd
    def __str__(self):
        io = ' '.join(list(map(lambda a: str(wire_mapping[a]), self.inputs + [self.output])))
        return "%d 1 %s %s" % (len(self.inputs), io, self.cmd) 

code = []
code.append(Instruction([STR_GND_WIRE], STR_HIGH_WIRE, "INV"))
try:
    for assign in assigns:
        result_wire = re.findall(r"([^=]+)=", assign)[0].strip()
        operation_side = re.findall(r"=(.*)$", assign)[0].strip()
        # AND
        is_and = re.findall(r"(.*)\&(.*)", operation_side)
        if len(is_and) > 0:
            assert(len(is_and[0]) == 2)
            code.append(Instruction([is_and[0][0], is_and[0][1]], result_wire, "AND"))
            continue
        # NOT (INV)
        is_inv = re.findall(r"~(.*)$", operation_side)
        if len(is_inv) > 0:
            assert(len(is_inv) == 1)
            code.append(Instruction([is_inv[0]], result_wire, "INV"))
            continue
        # XOR
        is_xor = re.findall(r"(.*)\^(.*)$", operation_side)
        if len(is_xor) > 0:
            assert(len(is_xor[0]) == 2)
            code.append(Instruction([is_xor[0][0], is_xor[0][1]], result_wire, "XOR"))
            continue

        # ADD CUSTOM GATES HERE




        # ASSIGNMENT
        # Note last part handles assignments to wires/vars, 
        operation_side = "{%s}" % operation_side
        result_var = re.findall(r"^([^[]+)", result_wire)[0]
        result_arr1 = re.findall(r"\[(.+)\]", result_wire)
        result_arr2 = re.findall(r"\[(.+):(.+)\]", result_wire)
        # Part of variable is being assigned e.g. assign a[1] =
        if len(result_arr2) > 0:
            tmp_wire_start = int(result_arr2[0][1])
            tmp_wire_end = int(result_arr2[0][0])
        elif len(result_arr1) > 0:
            tmp_wire_start = int(result_arr1[0][0])
            tmp_wire_end = tmp_wire_start
        # All the variable is assigned e.g. assign a =
        else:
            tmp_wire_start = 0
            tmp_wire_end = prop_lens[result_var] - 1
        tmp_wires = [None for i in range(prop_lens[result_var])]
        # Since re.findall is non-overlapping
        operation_side = operation_side.replace(",", ",,")
        parts = re.findall(r"[{,]([^{},]+)[},]", operation_side)
        pos_i = tmp_wire_end 
        for part in parts:
            part = part.strip()
            result_arr = re.findall(r"(\d{1,})'b(.*)", part)
            # Static assignment
            if len(result_arr) == 1:
                for i in range(int(result_arr[0][0])):
                    if result_arr[0][1][i] == '1':
                        tmp_wires[pos_i] = STR_HIGH_WIRE
                    elif result_arr[0][1][i] == '0':
                        tmp_wires[pos_i] = STR_GND_WIRE
                    # Note x is just skipped
                    pos_i -= 1
            else:
                if part in wire_mapping:
                    tmp_wires[pos_i] = part
                    pos_i -= 1
                else:
                    a_var = re.findall(r"^([^[]+)", part)[0]
                    a_arr = re.findall(r"\[(.+):(.+)\]", part)
                    if len(a_arr) > 0:
                        for i in range(int(a_arr[0][0]), int(a_arr[0][1]) - 1, -1):
                            tmp_wires[pos_i] = "%s[%s]" % (a_var, i)
                            pos_i -= 1
                    else:
                        for i in range(prop_lens[part] - 1, -1, -1):
                            tmp_wires[pos_i] = "%s[%s]" % (part, i)
                            pos_i -= 1
        assert pos_i + 1 == tmp_wire_start
        if result_var in wire_mapping and tmp_wire_start == tmp_wire_end and tmp_wire_start == 0:
            code.append(Instruction([tmp_wires[0]], "%s" % result_var, "EQW"))
        else:
            for i in range(tmp_wire_start, tmp_wire_end + 1):
                if tmp_wires[i] == None:
                    continue
                code.append(Instruction([tmp_wires[i]], "%s[%s]" % (result_var, i), "EQW"))

except Exception as e:
    logging.error(e)
    exit()

# Apply some quick optimisations
#- Remove output wires which are not used and not connected to an output port
while True:
    output_wires = set(map(lambda a: a.output, code))
    input_wires = []
    remove_wires = []
    remove_wires_mappings = []
    for instr in code:
        input_wires += instr.inputs
    input_wires = set(input_wires)
    for output_wire in output_wires:
        if output_wire not in input_wires and output_wire not in output_wire_names and wire_mapping[output_wire] >= num_input_wires:
            remove_wires.append(output_wire)
            remove_wires_mappings.append(wire_mapping[output_wire])
            del wire_mapping[output_wire]
    if len(remove_wires) == 0:
        break
    # Remove from code
    i = len(code) - 1
    while i >= 0:
        instr = code[i]
        if instr.output in remove_wires:
            del code[i]
        i -= 1
    total_num_wires -= len(remove_wires_mappings)
    # Adjust wire mappings
    for wire in wire_mapping:
        curr = wire_mapping[wire]
        if curr < num_input_wires:
            continue
        num = reduce(lambda c, a: c + int(a < curr), [0] + remove_wires_mappings)
        wire_mapping[wire] -= num

# Output the Bristol Format
logging.info("Outputting the Bristol formatted netlist to %s" % args.output_file)
with open(args.output_file, 'w') as f:
    f.write("%s %s\n" % (len(code), total_num_wires))
    f.write("%s %s\n" % (len(input_lens), ' '.join(input_lens)))
    f.write("%s %s\n" % (len(output_lens), ' '.join(output_lens)))
    f.write("\n")
    for instr in code:
        f.write("%s\n" % str(instr))
    f.write("\n\n")
