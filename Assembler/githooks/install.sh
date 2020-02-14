#!/bin/bash

set -e

wget https://github.com/git-hooks/git-hooks/releases/download/v1.2.0/git-hooks_linux_amd64.tar.gz
tar -xf git-hooks_linux_amd64.tar.gz
rm git-hooks_linux_amd64.tar.gz
mv build/git-hooks_linux_amd64 ~/.cargo/bin/git-hooks
rmdir build

git hooks install
