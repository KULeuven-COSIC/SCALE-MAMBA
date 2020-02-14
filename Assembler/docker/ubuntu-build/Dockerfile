# Check http://releases.llvm.org/download.html#8.0.0 for the latest available binaries
FROM ubuntu:18.04

WORKDIR /root

RUN apt-get update --fix-missing;

RUN apt-get install -y libssl-dev curl;

# rust
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y

RUN apt-get install -y build-essential pkg-config;

# sccache
RUN /root/.cargo/bin/cargo install sccache

# rustfmt
RUN /root/.cargo/bin/rustup component add rustfmt

RUN mkdir -p /root/projects

# Env
COPY resources/add-bashrc /root/add-bashrc
RUN cat /root/add-bashrc >> /root/.bashrc; \
    rm /root/add-bashrc; \
    echo "\ncd /root/projects" >> /root/.bashrc

# build script for scasm
COPY resources/build-scasm-debug.sh /root/build-scasm-debug.sh
RUN chmod u+x /root/build-scasm-debug.sh
COPY resources/build-scasm-release.sh /root/build-scasm-release.sh
RUN chmod u+x /root/build-scasm-release.sh

# Start from a Bash prompt
CMD [ "/bin/bash" ]
