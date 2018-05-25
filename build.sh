#!/bin/bash

ROOTDIR=$(git rev-parse --show-toplevel)

set -e
FULLPATH=$(dirname $(readlink -f $0))
cd $FULLPATH
rm -rf build/
./configure --bro-dist=$ROOTDIR/bro --install-root=$ROOTDIR/install/usr/local/bro/lib/bro/plugins
make
make install
