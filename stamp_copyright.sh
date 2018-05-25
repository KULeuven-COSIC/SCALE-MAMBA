#!/bin/bash

for i in `find . -type f -regex ".*\.[ch]\(pp\)?$"`
do
  echo $i
  if ! grep -q Copyright $i
  then
    cat Copyright.txt $i >$i.new && mv $i.new $i
  fi
  dos2unix $i
done
