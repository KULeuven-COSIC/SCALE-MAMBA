#!/bin/bash

set -ex

for d in scale/examples/*.rs ; do
    ./run_demo.sh $d
done
