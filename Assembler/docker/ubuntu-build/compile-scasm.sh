#!/bin/bash

# determine version to build
if [ "$1" = "" ]; then
    export CARGO_VERSION=debug
    echo "Building DEBUG version of scasm"
elif [ "$1" = "debug" ]; then
    export CARGO_VERSION=debug
    echo "Building DEBUG version of scasm"
elif [ "$1" = "release" ]; then
    export CARGO_VERSION=release
    echo "Building RELEASE version of scasm"    
else
    echo "usage: './compile-scasm VERSION', where VERSION is one of debug, release"
    exit 1
fi;

docker run --rm \
    -v /var/lib/jenkins/workspace/scasm:/root/projects/scasm \
    -v ubuntu_cargo_registry:/root/.cargo/registry \
    -v ubuntu_sccache:/root/sccache \
    --name ubuntu-compile-scasm \
    ubuntu-build-scasm /bin/bash -c /root/build-scasm-${CARGO_VERSION}.sh
