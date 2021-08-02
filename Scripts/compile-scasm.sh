#!/bin/bash

# This file creates the test assembly files for scasm

# Portably find the directory containing this script.
HERE=$(cd `dirname $0`; pwd)
# Assuming it lives in Scripts/, set ROOT to the root directory of the
# project.
ROOT=$HERE/..
# Now set the PYTHONPATH so we don't have to worry about the CWD
# whenever we invoke python scripts.
export PYTHONPATH=$ROOT:$PYTHONPATH

shift $[OPTIND-1]

run_test() {
    test=$1
    shift
    printf "\n\n\n\n\n\n\n\n\n\n"
    echo "        $test       "
    $ROOT/compile-mamba.py -a scasm -n -r -M -u -D -s Programs/$test || exit 1
    mv *.asm Assembler/tests/scasm-tests/
}

for test in test_all test_array test_branch test_branching test_comparison test_count test_empty_tape test_flex test_float test_floatingpoint test_float_sorting test_float_vector test_for_range_multithread test_function test_idle_threads test_lib test_loop test_map_reduce test_mem_order test_new_threads test_sregint test_threads test_vector test_sfix test_sqrt test_math test_custom_array test_fix_array do_nothing GC_test IO_demo Local_test mem_clear_demo mult_demo restart sregint_tests tutorial test_dabit test_stacks test_sbit; do
    run_test $test
done
