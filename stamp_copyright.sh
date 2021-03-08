#!/bin/bash

for i in `find . -type f -regex ".*\.[ch]\(pp\)?$"`
do
  echo $i
  if ! grep -q Copyright $i
  then
    cat Copyright-cpp.txt $i >$i.new && mv $i.new $i
  fi
  dos2unix $i
done

for i in `find . -type f -regex ".*\.[ch]\(pp\)?$"`
do
  echo $i
  sed -i 's/Copyright (c) 2020/Copyright (c) 2021/g' $i
  dos2unix $i
done

for i in `find . -type f -regex ".*\.\(py\)?$"`
do
  echo $i
  if ! grep -q Copyright $i
  then
    cat Copyright-python.txt $i >$i.new && mv $i.new $i
  fi
  dos2unix $i
done

for i in `find . -type f -regex ".*\.\(py\)?$"`
do
  echo $i
  sed -i 's/Copyright (c) 2020/Copyright (c) 2021/g' $i
  dos2unix $i
done

for i in `find . -type f -regex ".*\.\(rs\)?$"`
do
  echo $i
  if ! grep -q Copyright $i
  then
    cat Copyright-rust.txt $i >$i.new && mv $i.new $i
  fi
  dos2unix $i
done

for i in `find . -type f -regex ".*\.\(rs\)?$"`
do
  echo $i
  sed -i 's/Copyright (c) 2020/Copyright (c) 2021/g' $i
  dos2unix $i
done

