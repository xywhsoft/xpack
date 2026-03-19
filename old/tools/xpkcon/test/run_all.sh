#!/bin/bash
echo "================================================="
echo "  xpkcon Full Test Suite"
echo "================================================="
echo

OUTPUT_DIR="../../release/linux"
TEST_EXE="$OUTPUT_DIR/test_all"

if [ ! -f "$TEST_EXE" ]; then
    echo "Test executable not found, building..."
    ./build_test.sh
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
    echo
fi

echo "Running full test suite..."
echo "Options: --html  Generate HTML report"
echo "         --json  Generate JSON report"
echo
echo "Test executable: $TEST_EXE"
echo

"$TEST_EXE" "$@"

RESULT=$?

echo

if [ $RESULT -eq 0 ]; then
    echo "All tests passed!"
else
    echo "Some tests failed!"
fi

exit $RESULT
