#!/bin/bash

# Test script for 32 bit inputs

# Portably find the directory containing this script.
HERE=$(cd `dirname $0`; pwd)
# Assuming it lives in Scripts/, set ROOT to the root directory of the
# project.
ROOT=$HERE/..
# Now set the PYTHONPATH so we don't have to worry about the CWD
# whenever we invoke python scripts.
export PYTHONPATH=$ROOT:$PYTHONPATH

run_test() {
    test=$1
    optimizations=$2
    shift
    printf "\n\n\n\n\n\n\n\n\n\n"
    echo "       $test    "
    $ROOT/compile.sh $optimizations Programs/$test || exit 1
    Scripts/run-online.sh Programs/$test || exit 1
    python2 Scripts/test-result.py $test_opts $test || exit 1
}

if test "$1"; then
    test=$1
    shift
    run_test $test $*
else
    # We do not run sfix, float, math, sqrt and custom_array tests for 32 bit inputs 
    for test in test_array test_stacks test_branch test_branching test_comparison test_count test_empty_tape test_flex test_for_range_multithread test_function test_idle_threads test_lib test_loop test_map_reduce test_mem_order test_new_threads test_threads test_vector test_all test_sbit; do
	run_test $test
    done
fi
