#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $0 {DEVNAME}"
	exit 1
fi

bossac --boot=1 -a -i -e -w --port=$1 subtree_constr_atk_flash.bin
