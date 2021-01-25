#!/bin/bash

cp -R Auto-Test-Data/Cert-Store/* Cert-Store/

if [[ "$1" =~ 'test_wasm' ]]
then
    TEST_SCRIPT="test_wasm"
    TEST_SCRIPT_32="test_wasm_32"
    TEST_SCRIPT_n10="test_wasm-n-10"
    shift
else
    TEST_SCRIPT="test"
    TEST_SCRIPT_32="test_32"
    TEST_SCRIPT_n10="test-n-10"
fi


for i in $(seq 0 27); do
  \rm -f Scripts/logs/$i
done

for i in $(seq 1 19); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT.sh
done

for i in $(seq 20 21); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT_32.sh
done

for i in $(seq 22 24); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT.sh
done

for i in $(seq 25 25); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT_n10.sh
done

for i in $(seq 26 27); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT_32.sh
done
