#!/bin/bash

set -ex

pushd scale
name=`basename $1`
name="${name%.*}"
cargo build --release --example $name --target wasm32-unknown-unknown
ls target/wasm32-unknown-unknown/release
file=target/wasm32-unknown-unknown/release/examples/$name.wasm
du -h $file
#wasm-snip --snip-rust-panicking-code $file > /tmp/wasca.wasm
wasm-opt --dce -O4 $file -o /tmp/wasca.wasm || (cp $file /tmp/wasca.wasm && echo "not using wasm-opt, because it is not installed, install via the binaryen package")
wasm2wat /tmp/wasca.wasm -o wasm.wat
du -h /tmp/wasca.wasm
env RUST_LOG=trace cargo run --manifest-path ../Cargo.toml -- /tmp/wasca.wasm examples/$name.asm
env RUST_LOG=trace cargo run --manifest-path ../Cargo.toml -- /tmp/wasca.wasm examples/$name.dot
popd
