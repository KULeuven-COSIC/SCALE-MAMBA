#!/usr/bin/env bash

set -ex

HERE=$(cd `dirname $0`; pwd)

. $HERE/run-common.sh

run_player Player.x ${1:-test_all} || exit 1
