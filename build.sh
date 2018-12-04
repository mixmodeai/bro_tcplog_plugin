#!/bin/bash

if [ -z "$ROOTDIR" ] ; then
	echo ROOTDIR must be defined prior to calling ${0}
	exit 1
fi

set -e
FULLPATH=$(dirname $(readlink -f $0))
cd $FULLPATH
rm -rf build/
./configure --bro-dist=$ROOTDIR/bro --install-root=$ROOTDIR/install/usr/local/bro/lib/bro/plugins
make
make install
