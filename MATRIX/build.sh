#!/usr/bin/env bash

sudo apt-get update
sudo apt-get install build-essential -y
sudo apt-get install yasm m4 python -y

mkdir -p ~/local
export mylocal="$HOME/local"
# install mpir
#
cd ${mylocal}
curl -O 'http://mpir.org/mpir-3.0.0.tar.bz2'
tar xf mpir-3.0.0.tar.bz2
cd mpir-3.0.0
./configure --enable-cxx --prefix="${mylocal}/mpir"
make
make check
make install

# install OpenSSL 1.1.0

cd $mylocal
curl -O https://www.openssl.org/source/openssl-1.1.0j.tar.gz
tar -xf openssl-1.1.0j.tar.gz
cd openssl-1.1.0j
./config --prefix="${mylocal}/openssl"
make -j 4
make install

cd ~/SCALE-MAMBA/
cp MATRIX/config_paths.sh ~/local/

echo "source ${mylocal}/config_paths.sh" >> ~/.bashrc
# Reload libraries
source MATRIX/config_paths.sh
sudo ldconfig

# install SCALE-MAMBA
cp CONFIG CONFIG.mine
echo "ROOT = $HOME/SCALE-MAMBA" >> CONFIG.mine
echo "OSSL = ${mylocal}/openssl" >> CONFIG.mine
make progs


