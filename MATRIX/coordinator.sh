#!/bin/bash

num_parties=$1
test_suite=$2

# to be able to rsync between parties
./ssh-setup.sh
# generate FHE setup
./key-setup.sh $num_parties $test_suite
