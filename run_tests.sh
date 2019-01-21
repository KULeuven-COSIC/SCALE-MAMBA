#!/bin/bash

cp -R Auto-Test-Data/Cert-Store/* Cert-Store/

for i in $(seq 0 9); do
  \rm -f Scripts/logs/$i
done

for i in $(seq 1 19); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/test.sh
done

for i in $(seq 20 21); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/test_32.sh
done

for i in $(seq 22 24); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/test.sh
done

for i in $(seq 25 25); do
  cp -R Auto-Test-Data/$i/* Data/
  echo Running testscript on set $i
  ./Scripts/test-n-10.sh
done

