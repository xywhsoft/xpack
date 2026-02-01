#!/bin/bash
echo "Building xpkcon test programs..."
echo

OUTPUT_DIR="../../release/linux"
OUTPUT1="$OUTPUT_DIR/test_xpkcon"
OUTPUT2="$OUTPUT_DIR/test_all"

mkdir -p "$OUTPUT_DIR"

gcc -o "$OUTPUT1" \
	test_xpkcon.c \
	test_framework.c \
	-I../.. \
	-O2 -Wall

if [ $? -ne 0 ]; then
    echo "Build of test_xpkcon failed!"
    exit 1
fi

gcc -o "$OUTPUT2" \
	test_all.c \
	test_xpkcon.c \
	test_framework.c \
	-I../.. \
	-O2 -Wall

if [ $? -eq 0 ]; then
    echo
    echo "Build successful!"
    echo "Output: $OUTPUT1"
    echo "        $OUTPUT2"
    chmod +x "$OUTPUT1" "$OUTPUT2"
else
    echo
    echo "Build of test_all failed!"
fi
