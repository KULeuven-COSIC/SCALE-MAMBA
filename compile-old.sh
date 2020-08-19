#!/bin/bash

# This is the main compilation command for the old compilation
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

rm -f $1/*.asm
rm -f $1/*.bc
printf "Running \n\t ./compile.py -s %s\n\n" $1
./compile-mamba.py -s $1 || exit 1

