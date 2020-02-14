#!/bin/bash

cd /root/projects/scasm

# directory used by sccache to cach artifacts mounted on an external volume
export SCCACHE_DIR=/root/sccache

/root/.cargo/bin/cargo build --release --target-dir target/ubuntu-18_04
