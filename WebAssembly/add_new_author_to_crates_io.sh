#!/usr/bin/env bash

set -ex

# Portably find the directory containing this script.
HERE=$(cd `dirname $0`; pwd)
# Assuming it lives in Scripts/, set ROOT to the root directory of the
# project.
ROOT=$HERE/..

for crate in Documentation/documentation_parser Documentation WebAssembly/scale WebAssembly/scale_std WebAssembly/scale/impl_generator; do
    pushd $crate
    cargo owner --add $1
    popd
done
