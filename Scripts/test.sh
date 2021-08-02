#!/bin/bash

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
    echo "    $test    "
    $ROOT/compile.sh $optimizations Programs/$test || exit 1
    Scripts/run-online.sh Programs/$test || exit 1
    python2 Scripts/test-result.py $test_opts $test || exit 1
}

if test "$1"; then
    test=$1
    shift
    run_test $test $*
else
    for test in test_array test_branch test_branching test_comparison test_count test_empty_tape test_flex test_float test_floatingpoint test_float_sorting test_float_vector test_for_range_multithread test_function test_idle_threads test_lib test_loop test_map_reduce test_mem_order test_new_threads test_threads test_vector test_sfix test_sqrt test_custom_array test_fix_array test_all test_sbit test_stacks test_sregint; do
	run_test $test
    done
    run_test test_math -O1
    printf "\n\n\n\n\n\n\n\n\n\n"
fi
