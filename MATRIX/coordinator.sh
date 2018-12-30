#!/bin/bash

test_suite=$1

# to be able to rsync between parties
./ssh-setup.sh
# generate FHE setup
./key-setup.sh $test_suite
