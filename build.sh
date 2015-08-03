#!/bin/bash

set -e
FULLPATH=$(dirname $(readlink -f $0))
cd $FULLPATH
rm -rf build/
./configure --bro-dist=$FULLPATH/../../bro
make
make install
