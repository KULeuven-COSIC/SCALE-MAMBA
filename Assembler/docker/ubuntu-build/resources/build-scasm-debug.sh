#!/bin/bash

cd /root/projects/scasm

# directory used by sccache to cach artifacts mounted on an external volume
export SCCACHE_DIR=/root/sccache

/root/.cargo/bin/cargo build --target-dir target/ubuntu-18_04

/root/.cargo/bin/cargo test --target-dir target/ubuntu-18_04

/root/.cargo/bin/cargo fmt --all -- --check
