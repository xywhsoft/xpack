#!/bin/sh

set -e

mkdir -p release/x64
gcc tools/make_header/make_header.c -O2 -s -o release/x64/make_header
./release/x64/make_header

echo
echo "Header generated: xpack.h"
