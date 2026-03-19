#!/bin/bash
echo "================================================="
echo "  xpkcon Test Runner"
echo "================================================="
echo

OUTPUT_DIR="../../release/linux"
TEST_EXE="$OUTPUT_DIR/test_xpkcon"

if [ ! -f "$TEST_EXE" ]; then
    echo "Test executable not found, building..."
    ./build_test.sh
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
    echo
fi

echo "Running tests..."
echo
echo "Test executable: $TEST_EXE"
echo

"$TEST_EXE"
RESULT=$?

echo

if [ $RESULT -eq 0 ]; then
    echo "All tests passed!"
else
    echo "Some tests failed!"
fi

exit $RESULT
