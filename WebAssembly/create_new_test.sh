#!/bin/bash

set -ex

cd scale/demos
cargo new --bin $1
cp new_demo.rs $1/src/main.rs
echo "scale = { path = \"../../scale\" }" >> $1/Cargo.toml
