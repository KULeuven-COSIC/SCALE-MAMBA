#!/usr/bin/env bash
source config_paths.sh

# read the arguments without party id
argc=$#
argv=($@)
values=""

for (( j=1; j<argc; j++ )); do
    values+=`echo ${argv[j]}`
    values+=' '
done

idx=${1}
source_file=${2}

# have first party run the key setup

cd ~/SCALE-MAMBA
# sleep enough so that files can sync;
# with more parties probably need to sync more
sleep 15

./compile.py ${source_file}
./Player.x ${idx} ${values}



