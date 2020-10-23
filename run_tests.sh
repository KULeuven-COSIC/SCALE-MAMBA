#!/bin/bash

cp -R Auto-Test-Data/Cert-Store/* Cert-Store/

if test "$1"; then
    TEST_SCRIPT=$1
    shift
else
    TEST_SCRIPT="test"
fi


for i in $(seq 0 25); do
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
  ./Scripts/test_32.sh
done

for i in $(seq 22 24); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/$TEST_SCRIPT.sh
done

for i in $(seq 25 25); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/test-n-10.sh
done
