export mylocal="$HOME/local"
export PATH="${mylocal}/bin:${PATH}"

# export OpenSSL paths
export PATH="${mylocal}/openssl/bin/:${PATH}"
export C_INCLUDE_PATH="${mylocal}/openssl/include/:${C_INCLUDE_PATH}"
export CPLUS_INCLUDE_PATH="${mylocal}/openssl/include/:${CPLUS_INCLUDE_PATH}"
export LIBRARY_PATH="${mylocal}/openssl/lib/:${LIBRARY_PATH}"
export LD_LIBRARY_PATH="${mylocal}/openssl/lib/:${LD_LIBRARY_PATH}"

# export MPIR paths
export PATH="${mylocal}/mpir/bin/:${PATH}"
export C_INCLUDE_PATH="${mylocal}/mpir/include/:${C_INCLUDE_PATH}"
export CPLUS_INCLUDE_PATH="${mylocal}/mpir/include/:${CPLUS_INCLUDE_PATH}"
export LIBRARY_PATH="${mylocal}/mpir/lib/:${LIBRARY_PATH}"
export LD_LIBRARY_PATH="${mylocal}/mpir/lib/:${LD_LIBRARY_PATH}"

