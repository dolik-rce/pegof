#!/usr/bin/env bats
load "$TESTDIR/utils.sh"

export BATS_NO_PARALLELIZE_WITHIN_FILE=true
PCC_TEST_DIR="$PWD/../packcc/tests"
mapfile -t TESTS < <(cd "$PCC_TEST_DIR"; ls -d -- *.d)

run_wrapped() {
    [ "$INCLUDE_SLOW_TESTS" ] || skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    (OPTIMIZE="$2" PACKCC="$PWD/packcc.d/wrapper.sh" "$PCC_TEST_DIR"/test.sh -f "$1")
}

if check_bats_version 1 11; then
    BTF="bats_test_function"
    for TEST in "${TESTS[@]}"; do
        if ! ls "$PCC_TEST_DIR/$TEST"/*.peg &> /dev/null; then
            # There!s no point in running tests that do not have any grammar (e.g. code style).
            continue
        fi
        if ls "$PCC_TEST_DIR/$TEST"/*.bats &> /dev/null && grep -qEe "--debug|--lines" "$PCC_TEST_DIR/$TEST"/*.bats; then
            # Tests with --lines and --debug can't be used, because the file content is different after pegof processes it
            continue
        fi
        $BTF --description "packcc.d - ${TEST/.d/} formatted" -- run_wrapped "$TEST" 0
        if [ "$TEST" = "debug_macro.d" ]; then
            # The debug macro output is different for optimized grammar, so it can't be tested
            continue
        fi
        $BTF --description "packcc.d - ${TEST/.d/} optimized" -- run_wrapped "$TEST" 1
    done
else
    @test "packcc.d - *" {
        skip "this testsuite requires bats version at least 1.11"
    }
fi
