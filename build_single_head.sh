#!/bin/sh

set -e

cd "$(dirname "$0")"

mkdir -p singlehead

gcc singlehead/single_head_maker.c -O2 -s -o singlehead/single_head_maker
./singlehead/single_head_maker

echo
echo "Single header generated: singlehead/xpack.h"
