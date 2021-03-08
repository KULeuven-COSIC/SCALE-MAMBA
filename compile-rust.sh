#!/usr/bin/env bash

set -ex

# Portably find the directory containing this script.
ROOT=$(cd `dirname $0`; pwd)

shift $[OPTIND-1]

ARGS=("$@")
# Get the last argument
test=${ARGS[-1]}
# Drop it from the array
unset ARGS[${#ARGS[@]}-1]


# Generate the wasm for the given Rust program
# FIXME: set `-Cinline-threshold=9999`
env SHARING_DATA=$ROOT/Data/SharingData.txt RUSTC_BOOTSTRAP=1 RUSTFLAGS="-Ctarget-feature=+simd128,+unimplemented-simd128 -C inline-threshold=9999 --remap-path-prefix $ROOT=" cargo build --target wasm32-unknown-unknown --manifest-path=$ROOT/RustPrograms/Cargo.toml --release --example $test || exit 1
# The path where cargo build the file to
file=$ROOT/RustPrograms/target/wasm32-unknown-unknown/release/examples/$test.wasm
# Create the output directory
example_dir=$ROOT/RustPrograms/examples/$test
mkdir -p $example_dir
# Optimize the wasm
if [ "$DUMP_WASM" == "1" ]; then
    wasm2wat --enable-simd $file -o $example_dir/wasm.unoptimized.wat
fi
OPTS="${OPTS=--always-inline-max-function-size=9999 --flexible-inline-max-function-size=9999 --one-caller-inline-max-function-size=9999}"
wasm-opt -g --enable-simd -O4 --dce --inlining-optimizing -iit -lmu --optimize-added-constants-propagate --vacuum $OPTS $file -o $example_dir/wasca.wasm

if [ "$DUMP_WASM" == "1" ]; then
wasm2wat --enable-simd $file -o $file.optimized
wasm2wat --enable-simd $example_dir/wasca.wasm -o $example_dir/wasm.wat
fi

# code --diff $file.optimized $example_dir/wasm.wat
# Generate the scale bytecode and schedule from the wasm
cargo run --manifest-path $ROOT/WebAssembly/Cargo.toml -- $example_dir/wasca.wasm $example_dir/$test.bc $ARGS
#cargo run --quiet --manifest-path $ROOT/WebAssembly/Cargo.toml -- $example_dir/wasca.wasm $example_dir/$test.dot $ARGS
cargo run --quiet --manifest-path $ROOT/WebAssembly/Cargo.toml -- $example_dir/wasca.wasm $example_dir/$test.asm $ARGS
