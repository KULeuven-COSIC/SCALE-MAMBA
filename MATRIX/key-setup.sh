#!/bin/bash

source config_paths.sh

num_parties=${1}
test_suite=${2}

# Usage: aws-key-setup.sh num_parties prime_no

# put default network data into place
cd ~/SCALE-MAMBA/MATRIX


network_data_file=~/SCALE-MAMBA/Data/NetworkData.txt
rm $network_data_file

echo "RootCA.crt" >> $network_data_file
echo "${num_parties}" >> $network_data_file

# Generate OpenSSL certificates
SUBJ="/CN=www.example.com"

# create directory for logs
mkdir -p logs

cd ~/SCALE-MAMBA/Cert-Store/
SUBJ="/CN=www.example.com"
openssl genrsa -out RootCA.key 4096
openssl req -new -x509 -days 1826 -key RootCA.key -subj $SUBJ -out RootCA.crt

mkdir csr
for (( ID=1; ID<=${num_parties}; ID++ )) do
	SUBJ="/CN=Player$ID@example.com"
	openssl genrsa -out Player${ID}.key 2048
	openssl req -new -key Player$ID.key -subj $SUBJ -out csr/Player$ID.csr
	openssl x509 -req -days 1000 -set_serial 0101 \
		-CA RootCA.crt -CAkey RootCA.key \
		-in csr/Player$ID.csr -out Player$ID.crt 
done


cd ~/SCALE-MAMBA
# now put the cert in place into Networkdata using Setup.x
cert_command="1 RootCA ${num_parties}"
for (( ID=1; ID<=${num_parties}; ID++ )) do
	private_ip=`sed -n ${ID}p MATRIX/HOSTS.private`
	cert_command="$cert_command ${private_ip} Player${ID}.crt"
done

# set fake offline and fake sacrifice
cert_command="$cert_command N N"
./Setup.x <<< ${cert_command}

if test $(wc -l MATRIX/HOSTS.public | cut -f 1 -d ' ') -lt $1; then
    echo not enough instances: $(wc -l MATRIX/HOSTS.public)
    exit 1
fi

#Now take test configuration and put it into Data/ main directory
cp "Auto-Test-Data/${test_suite}/SharingData.txt" Data/

scale_dir=$PWD
matrix_dir=${scale_dir}/MATRIX

{
    files="${scale_dir}/./Data/NetworkData.txt ${scale_dir}/./Data/SharingData.txt ${scale_dir}/./Cert-Store/RootCA.crt"
    k=0
    for i in $(cat ${matrix_dir}/HOSTS.public); do
	{
	    eval rsync -tR $files $i:${scale_dir}
	    eval rsync -RL ${scale_dir}/./Data/FHE-Key-$k.key $i:${scale_dir}
	    eval rsync -RL ${scale_dir}/./Data/MKey-$k.key $i:${scale_dir}
	    eval rsync -RL ${scale_dir}/./Cert-Store/Player$[k+1].key $i:${scale_dir}
	    eval rsync -RL ${scale_dir}/./Cert-Store/Player*.crt $i:${scale_dir}
	    echo -n .
	} &
	k=$[k+1]
    done
}
2>&1 | grep -v 'Permanently added' | grep -v 'nofile 10000' | grep -v 'no process found'
wait


