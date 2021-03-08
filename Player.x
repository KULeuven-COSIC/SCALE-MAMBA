#!/usr/bin/env bash

set -ex

if [ "$USE_RUST" == "1" ]
then
    cargo run --bin player "$@"
else
    ./PlayerBinary.x "$@"
fi
