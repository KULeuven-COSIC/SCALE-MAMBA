#!/bin/bash

cd ~/SCALE-MAMBA/MATRIX

# cut out the coordinator
grep 'ip=' parties.conf | cut -f2 -d"=" | tail -n +2 > HOSTS.private
tail -n +2 public_ips > HOSTS.public

cd ~/SCALE-MAMBA/
scale_dir=$PWD
matrix_dir=${scale_dir}/MATRIX

{

    sudo cp ${matrix_dir}/matrix.pem ~/.ssh/id_rsa
    echo StrictHostKeyChecking no >> ~/.ssh/config;
    echo UserKnownHostsFile /dev/null >> ~/.ssh/config
    files="${matrix_dir}/./HOSTS.private ${matrix_dir}/./HOSTS.public"
    k=0
    for i in $(cat ${matrix_dir}/HOSTS.public); do
        {
            eval rsync -tR $files $i:${matrix_dir}
        } & true
    done
    wait
    echo Global setup distribution done
}
2>&1 | grep -v 'Permanently added' | grep -v 'nofile 10000' | grep -v 'no process found'
