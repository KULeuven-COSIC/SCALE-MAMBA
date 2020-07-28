#!/bin/bash

# This is the main compilation command for the new compilation
# pipeline

# Portably find the directory containing this script.
HERE=$(cd `dirname $0`; pwd)
# Assuming it lives in root directory of the project,
# set ROOT to the root directory of the project.
ROOT=$HERE

# Now set the PYTHONPATH so we don't have to worry about the CWD
# whenever we invoke python scripts.
export PYTHONPATH=$ROOT:$PYTHONPATH

shift $[OPTIND-1]

ARGS=("$@")
# Get the last argument which is the program name
target=${ARGS[-1]}

# Extract mamba/scasm specific args
SCASM_ARGS=()
MAMBA_ARGS=()
for arg in "$@" 
do
  if [ $arg == "-D" ]; then
    MAMBA_ARGS+=($arg)
  fi
  if [ $arg == "-O0" ] || [ $arg == "-O1" ] || [ $arg == "-O2" ] || [ $arg == "-O3" ] || [ $arg == "-T" ]; then
    SCASM_ARGS+=($arg)
  fi
done

rm -f $target/*.asm
rm -f $target/*.bc
rm -f $target/*.sch
# Do not add in -D flag automatically as this is too aggressive a remover of instructions
printf "Running \n\t ./compile-mamba.py -A -n -r -u -s %s %s\n\n" $MAMBA_ARGS $target
./compile-mamba.py -A -n -r -u -s $MAMBA_ARGS $target || exit 1
printf "\nNow running \n\t./scasm %s %s\n\n" $SCASM_ARGS $target
#./scasm --verbose $1 -- --optimizations start_stop_open nop_removal || exit 1
#./scasm --verbose $1 -- --optimizations nop_removal || exit 1
#./scasm --verbose $1 -- --optimizations start_stop_open || exit 1
#./scasm --verbose $1 -- --optimizations register_coloring || exit 1
#./scasm --verbose $1 -- --optimizations nop_removal register_coloring || exit 1
#./scasm --verbose $1 -- --dump-optimizations=temp || exit 1
./scasm --verbose $target -- --hide-warnings "${SCASM_ARGS[@]}" || exit 1
printf "\n\n"
